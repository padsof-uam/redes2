#include "list.h"
#include "errors.h"
#include "strings.h"
#include "list_helpers.h"

list* list_new(duplicator dp, destructor ds)
{
	list* l = NULL;

	l = (list*) malloc(sizeof(list));

	if(l == NULL)
		return l;

	l->array = (void**) calloc(4, sizeof(void*));

	if(l->array != NULL)
		l->capacity = 4;

	l->count = 0;
	l->destruct = ds;
	l->duplicate = dp;

	return l;
}

void list_destroy(list* ls)
{
	int i;
	if(ls != NULL)
	{
		if(ls->array != NULL)
		{
			for(i = 0; i < ls->capacity; i++)
			{
				if(ls->array[i] != NULL)
					ls->destruct(ls->array[i]);
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

int list_add(list* ls, const void* element)
{
	return list_insert(ls, element, ls->count);
}

int list_insert(list* ls, const void* element, int index)
{
	void* copy;

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

	copy = ls->duplicate(element);

	if(ls->array[index] != NULL)
		ls->destruct(ls->array[index]);

	ls->array[index] = copy;

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


int list_find(list* ls, comparator comp, void * element)
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

int list_remove(list* ls, int element)
{
	if(element < 0 || element >= ls->count)
		return ERR_RANGE;

	/* Ponemos el último elemento en la posición que nos piden */
	ls->count--;
	ls->array[element] = ls->array[ls->count - 1];

	return OK;
}
