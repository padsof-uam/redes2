#include "lfringbuf.h"

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>

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
	rb->waiting_signal = NULL;
	rb->item_size = item_size;
	rb->next_push_is_overwrite = 0;

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
	rb->next_push_is_overwrite = __sync_are_equal(rb->start_ptr, overwrite_pos);;

	list_ptr += end_ptr * rb->item_size;

	memcpy(list_ptr, item, rb->item_size);

	/* marcamos el avance la última posición, ya lista para leer */
	__sync_lock_test_and_set(&(rb->end_ptr), (end_ptr + 1) % rb->capacity);

	if(rb->waiting_signal)
		pthread_cond_signal(rb->waiting_signal);

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

	return 0;
}

int lfringbuf_wait_for_items(lfringbuf* rb, long ms_timeout)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	struct timeval tv;
    struct timespec ts;
	int empty;
	int retval = 0;

    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + ms_timeout / 1000;
    ts.tv_nsec = tv.tv_usec / 1000 + (ms_timeout % 1000) * 1000;

    rb->waiting_signal = &cond;

	pthread_mutex_lock(&mtx);
    {
    	empty = __sync_bool_compare_and_swap(&(rb->end_ptr), rb->start_ptr, rb->start_ptr);
        
        while (empty)
    	{
    		if(ms_timeout < 0)
	    		pthread_cond_wait(&cond, &mtx);
    		else
    			retval = pthread_cond_timedwait(&cond, &mtx, &ts);

    		if(retval == ETIMEDOUT)
    			return ERR_TIMEOUT;

    		empty = __sync_bool_compare_and_swap(&(rb->end_ptr), rb->start_ptr, rb->start_ptr);
        }
    }
    pthread_mutex_unlock(&mtx);

    return OK;
}

void lfringbuf_destroy(lfringbuf* rb)
{
	if(!rb)
		return;

	free(rb->list);
	free(rb);
}
