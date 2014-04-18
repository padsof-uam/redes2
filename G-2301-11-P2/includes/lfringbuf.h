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
	pthread_cond_t* waiting_signal;
};

typedef struct lfringbuf lfringbuf;

lfringbuf* lfringbuf_new(unsigned int capacity, size_t item_size);
int lfringbuf_push(lfringbuf* rb, void* item);
int lfringbuf_wait_for_items(lfringbuf* rb);
int lfringbuf_pop(lfringbuf* rb, void* dst);

#endif
