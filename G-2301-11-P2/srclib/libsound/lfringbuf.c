#include "lfringbuf.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#define __sync_retrieve(val) __sync_add_and_fetch(&(val), 0)
#define __sync_are_equal(vsync, v2) __sync_bool_compare_and_swap(&(vsync), v2, v2);

lfringbuf* lfringbuf_new(unsigned int capacity, size_t item_size)
{
	lfringbuf* rb = malloc(sizeof(lfringbuf));

	if(!rb)
		return NULL;

	rb->end_ptr = 0;
	rb->start_ptr = 0;
	rb->capacity = capacity;
	rb->item_size = item_size;
	rb->next_push_is_overwrite = 0;
	rb->destroying = 0;
	rb->items = 1;

	pthread_mutex_init(&rb->waiting_mutex, NULL);
	pthread_cond_init(&rb->waiting_cond, NULL);

	rb->list = calloc(capacity, item_size);

	if(!rb->list)
	{
		free(rb);
		return NULL;
	}

	return rb;
}

int lfringbuf_push(lfringbuf* rb, void* item)
{
	unsigned int end_ptr;
	char* list_ptr = rb->list;
	int overwrite_pos;
	int overwrite;

	end_ptr = __sync_retrieve(rb->end_ptr);
	overwrite = __sync_are_equal(rb->start_ptr, end_ptr);

	if(__sync_retrieve(rb->next_push_is_overwrite) && overwrite)
		return ERR;

	overwrite_pos = (end_ptr + 1) % rb->capacity;
	rb->next_push_is_overwrite = __sync_are_equal(rb->start_ptr, overwrite_pos);

	list_ptr += end_ptr * rb->item_size;

	memcpy(list_ptr, item, rb->item_size);

	/* marcamos el avance la última posición, ya lista para leer */
	__sync_lock_test_and_set(&(rb->end_ptr), (end_ptr + 1) % rb->capacity);

	__sync_add_and_fetch(&(rb->items), 1);

	pthread_mutex_lock(&rb->waiting_mutex);
	pthread_cond_signal(&rb->waiting_cond);
	pthread_mutex_unlock(&rb->waiting_mutex);

	return OK;
}	

int lfringbuf_pop(lfringbuf* rb, void* dst)
{
	int empty;
	unsigned int start_ptr;
	char* list_ptr = rb->list;

	start_ptr = __sync_retrieve(rb->start_ptr);
	empty = __sync_are_equal(rb->end_ptr, start_ptr);

	if(empty && !__sync_retrieve(rb->next_push_is_overwrite))
		return ERR;


	list_ptr += start_ptr * rb->item_size;

	memcpy(dst, list_ptr, rb->item_size);
	__sync_lock_test_and_set(&(rb->start_ptr), (start_ptr + 1) % rb->capacity);
	__sync_lock_test_and_set(&(rb->next_push_is_overwrite), 0);
	__sync_sub_and_fetch(&(rb->items), 1);

	return 0;
}

int lfringbuf_wait_for_items(lfringbuf* rb, long ms_timeout)
{
	struct timeval tv;
    struct timespec ts;
	int empty;
	int retval = 0;
	short destroying;

    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + ms_timeout / 1000;
    ts.tv_nsec = tv.tv_usec / 1000 + (ms_timeout % 1000) * 1000;

	pthread_mutex_lock(&rb->waiting_mutex);
    {
    	empty = __sync_bool_compare_and_swap(&(rb->end_ptr), rb->start_ptr, rb->start_ptr);
        destroying = __sync_retrieve(rb->destroying);
        
        while (empty && !destroying)
    	{
    		if(ms_timeout < 0)
	    		pthread_cond_wait(&rb->waiting_cond, &rb->waiting_mutex);
    		else
    			retval = pthread_cond_timedwait(&rb->waiting_cond, &rb->waiting_mutex, &ts);

    		if(retval == ETIMEDOUT)
    			return ERR_TIMEOUT;

    		empty = __sync_bool_compare_and_swap(&(rb->end_ptr), rb->start_ptr, rb->start_ptr);
        	destroying = __sync_retrieve(rb->destroying);
        }
    }
    pthread_mutex_unlock(&rb->waiting_mutex);

 	if(destroying)
 		return ERR;
 	else
	    return OK;
}

void lfringbuf_signal_destroying(lfringbuf* rb)
{
	__sync_lock_test_and_set(&(rb->destroying), 0);
	pthread_mutex_lock(&rb->waiting_mutex);
	pthread_cond_signal(&rb->waiting_cond);
	pthread_mutex_unlock(&rb->waiting_mutex);
}

int lfringbuf_count(lfringbuf* rb)
{
	return __sync_retrieve(rb->items);
}

void lfringbuf_destroy(lfringbuf* rb)
{
	if(!rb)
		return;

	if(pthread_cond_destroy(&rb->waiting_cond) == EBUSY)
	{
		lfringbuf_signal_destroying(rb);		

		usleep(1000);
		pthread_cond_destroy(&rb->waiting_cond);	
	}

	pthread_mutex_destroy(&rb->waiting_mutex);

	free(rb->list);
	free(rb);
}
