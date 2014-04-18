#include "lfringbuf.h"

#include <stdlib.h>
#include <string.h>

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
	int overwrite;
	unsigned int end_ptr;

 	end_ptr = rb->end_ptr;
	end_ptr++;
	overwrite = __sync_bool_compare_and_swap(&(rb->start_ptr), end_ptr, end_ptr);

	if(overwrite)
		return ERR;

	memcpy(rb->list + (end_ptr % rb->capacity), item, rb->item_size);

	/* marcamos el avance la última posición, ya lista para leer */
	__sync_add_and_fetch(&(rb->end_ptr), 1);

	if(rb->waiting_signal)
		pthread_cond_signal(rb->waiting_signal);

	return OK;
}	

int lfringbuf_pop(lfringbuf* rb, void* dst)
{
	int empty;
	unsigned int start_ptr;

	start_ptr = rb->start_ptr;

	empty = __sync_bool_compare_and_swap(&(rb->end_ptr), start_ptr, start_ptr);

	if(empty)
		return -1;

	memcpy(dst, rb->list + (start_ptr % rb->capacity), rb->item_size);

	__sync_add_and_fetch(&(rb->start_ptr), 1);

	return 0;
}

int lfringbuf_wait_for_items(lfringbuf* rb)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	int empty;

	pthread_mutex_lock(&mtx);
    {
    	empty = __sync_bool_compare_and_swap(&(rb->end_ptr), rb->start_ptr, rb->start_ptr);
        while (!empty)
    	{
    		pthread_cond_wait(&cond, &mtx);
    		empty = __sync_bool_compare_and_swap(&(rb->end_ptr), rb->start_ptr, rb->start_ptr);
        }
    }
    pthread_mutex_unlock(&mtx);

    return OK;
}
