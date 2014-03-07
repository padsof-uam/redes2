#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include "errors.h"
#include "list_helpers.h"

typedef struct {
	void** array;
	int count;
	int capacity;
} list;

list* list_new();
void list_destroy(list* ls, destructor dc);
int list_clear(list* ls);
int list_add(list* ls, void* element);
int list_insert(list* ls, void* element, int index);
int list_remove(list* ls, int element);
int list_remove_last(list *ls);
int list_find(list* ls, comparator comp, void * element);
int list_count(list* ls);
void* list_at(list* ls, int index);


#endif
