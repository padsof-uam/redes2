#include <stdlib.h>
#include "errors.h"
#include "list_helpers.h"

#ifndef LIST_H
#define LIST_H

typedef void* (*duplicator)(const void*);
typedef void (*destructor)(void*);
typedef int (*comparator)(const void*, const void *);

typedef struct {
	void** array;
	int count;
	int capacity;
	duplicator duplicate;
	destructor destruct;
} list;

list* list_new(duplicator dp, destructor ds);
void list_destroy(list* ls);
int list_clear(list* ls);
int list_add(list* ls, const void* element);
int list_insert(list* ls, const void* element, int index);
int list_remove(list* ls, int element);
int list_find(list* ls, comparator comp, void * element);
int list_count(list* ls);
void* list_at(list* ls, int index);

#endif
