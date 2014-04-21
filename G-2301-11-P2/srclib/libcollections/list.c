#include "list.h"
#include "errors.h"
#include "strings.h"
#include "list_helpers.h"

list* list_new()
{
	list* l = NULL;

	l = (list*) malloc(sizeof(list));

	if(l == NULL)
		return l;

	l->array = (void**) calloc(4, sizeof(void*));

	if(l->array != NULL)
		l->capacity = 4;

	l->count = 0;

	return l;
}

void list_destroy(list* ls, destructor dc)
{
	int i;
	if(ls != NULL)
	{
		if(ls->array != NULL)
		{
			for(i = 0; i < ls->capacity; i++)
			{
				if(ls->array[i] != NULL && dc != NULL)
					dc(ls->array[i]);
			}

			free(ls->array);
		}

		free(ls);
	}
}

int list_clear(list* ls)
{
	/* Just say "there's nothing there" and let it write from the start */
	ls->count = 0;

	return OK;
}

static int _set_capacity(list* ls, int new_cap)
{
	int i;	
	void** new_array = NULL;

	new_array = (void**) realloc(ls->array, new_cap * sizeof(void*));

	if(new_array == NULL)
		return ERR;

	for(i = ls->capacity; i < new_cap; i++)
		new_array[i] = NULL; /* Explicitly set all pointers to NULL */

	ls->capacity = new_cap;
	ls->array = new_array;

	return OK;
}

static int _increase_capacity(list* ls)
{
	return _set_capacity(ls, ls->capacity * 2);
}

int list_add(list* ls, void* element)
{
	return list_insert(ls, element, ls->count);
}

int list_insert(list* ls, void* element, int index)
{
	if(index > ls->capacity)
	{
		return ERR;
	}
	else if (index == ls->capacity)
	{
		/* This is just a normal "add" operation, increase capacity as usual */
		if(_increase_capacity(ls) == ERR)
			return ERR;
	}

	ls->array[index] = element;

	ls->count++;

	return OK;
}

int list_count(list* ls)
{
	return ls->count;
}

void* list_at(list* ls, int index)
{
	if(index >= ls->count)
		return NULL;

	return ls->array[index];
}


int list_find(list* ls, comparator comp, const void * element)
{
	int i;
	int size = list_count(ls);
	void* current = NULL;

	for(i = 0; i < size; i++)
	{
		current = list_at(ls, i);
		if((*comp)(current, element) == 0)
			return i;
	}

	return -1;
}

int list_remove_last(list *ls)
{
	return list_remove(ls, ls->count-1);
}

int list_remove(list* ls, int element)
{
	if(element < 0 || element >= ls->count)
		return ERR_RANGE;

	/* Ponemos el último elemento en la posición que nos piden */
	ls->count--;
	ls->array[element] = ls->array[ls->count];

	return OK;
}

int list_remove_element(list* ls, comparator comp, void* element)
{
	int index;

	index = list_find(ls, comp, element);

	if(index != -1)
	{
		list_remove(ls, index);
		return OK;
	}

	return ERR_NOTFOUND;
}
