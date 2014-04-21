#ifndef LFRINGBUF_H
#define LFRINGBUF_H

#include <pthread.h>
#include "types.h"

struct lfringbuf {
	unsigned int capacity;
	unsigned int start_ptr;
	unsigned int end_ptr;
	size_t item_size;
	void* list;
	short next_push_is_overwrite;
	pthread_cond_t waiting_cond;
	pthread_mutex_t waiting_mutex;
	short destroying;
};

typedef struct lfringbuf lfringbuf;

lfringbuf* lfringbuf_new(unsigned int capacity, size_t item_size);
int lfringbuf_push(lfringbuf* rb, void* item);
int lfringbuf_wait_for_items(lfringbuf* rb, long ms_timeout);
int lfringbuf_pop(lfringbuf* rb, void* dst);
void lfringbuf_destroy(lfringbuf* rb);
void lfringbuf_signal_destroying(lfringbuf* rb);

#endif
