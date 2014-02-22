#include "dictionary.h"
#include "list_helpers.h"
#include "errors.h"
#include <string.h>
#include <stdlib.h> 

static void _destroy_kvpair(dictionary* dic, kv_pair* pair)
{
	dic->key_des(pair->key);
	free(pair);
}

dictionary* dic_new(duplicator key_dp, destructor key_dc, comparator comp, hash_function hash)
{
	dictionary* dic = NULL;

	dic = (dictionary*) malloc(sizeof(dictionary));

	if(dic == NULL)
		return NULL;

	dic->symtab = (kv_pair**) calloc(NHASH, sizeof(kv_pair*));

	if(dic->symtab == NULL)
	{
		free(dic);
		return NULL;
	}

	dic->n_hash = NHASH;
	dic->items = 0;
	dic->key_comp = comp;
	dic->key_des = key_dc;
	dic->key_dup = key_dp;
	dic->hash = hash;

	return dic;
}


int dic_add(dictionary* dic, const void* key, void* value)
{
	kv_pair* pair = _dic_lookup(dic, key, value, 1);

	if(pair == NULL)
		return ERR_MEM;
	else
	{
		if(dic->key_comp(pair->value, value) == 0)
			return OK;
		else
			return ERR_REPEAT;
	}
}
int dic_remove(dictionary* dic, const void* key)
{
	int h = (*dic->hash)(dic, key);
	kv_pair* pair;
	kv_pair* previous;

	pair = dic->symtab[h];
	previous = NULL;

	while(pair != NULL)
	{
		if(strcmp(pair->key, key) == 0)
		{
			if(previous == NULL)
				dic->symtab[h] = pair->next;
			else
				previous->next = pair->next;

			_destroy_kvpair(dic, pair);
			dic->items--;
			return OK;
		}

		previous = pair;
		pair = pair->next;
	}

	return ERR_NOTFOUND;
}

void* dic_lookup(dictionary* dic, const void* key)
{
	kv_pair* pair = _dic_lookup(dic, key, NULL, 0);

	if(pair == NULL)
		return NULL;
	else
		return pair->value;
}


void* dic_lookup_create(dictionary* dic, const void* key, void* value)
{
	kv_pair* pair = _dic_lookup(dic, key, value, 1);

	if(pair == NULL)
		return NULL;
	else
		return pair->value;
}

kv_pair* _dic_lookup(dictionary* dic, const void* key, void* value, int create)
{
	int h = dic->hash(dic, key);
	kv_pair* pair;

	for(pair = dic->symtab[h]; pair != NULL; pair = pair->next)
	{
		if(dic->key_comp(pair->key, key) == 0)
			return pair;
	}

	if(create)
	{
		pair = (kv_pair*) malloc(sizeof(kv_pair));

		if(pair == NULL)
			return NULL;

		pair->key = dic->key_dup(key);
		pair->value = value;
		pair->next = dic->symtab[h];
		dic->symtab[h] = pair;

		dic->items++;
	}

	return pair;
}

void dic_destroy(dictionary* dic, destructor dc)
{
	int i;
	kv_pair* to_destroy; 
	kv_pair* next;

	for(i = 0; i < dic->n_hash; i++)
	{
		to_destroy = dic->symtab[i];

		while(to_destroy != NULL)
		{
			next = to_destroy->next;

			if(dc != NULL)
				dc(to_destroy->value);

			_destroy_kvpair(dic, to_destroy);
			dic->items--;
			to_destroy = next;
		}
	}

	free(dic->symtab);
	free(dic);
}

int dic_count(dictionary* dic)
{
	return dic->items;
}

int dic_iterate(dictionary* dic, iterator_action action, void* pass_through)
{
	int i;
	kv_pair* pair;
	int retval;

	for(i = 0; i < dic->n_hash; i++)
	{
		pair = dic->symtab[i];
		while(pair != NULL)
		{
			retval = action(pair->key, pair->value, pass_through);

			if(retval != OK)
				return retval;

			pair = pair->next;
		}
	}

	return OK;
}

int dic_update(dictionary* dic, const void* key, void* value)
{
	kv_pair* pair = _dic_lookup(dic, key, value, 1);
	
	pair->value = value;
	
	return OK;
}

/* Funciones para crear diccionarios predefinidos */
static unsigned int str_hash(const dictionary* dic, const void* key)
{
	unsigned int h;
	unsigned char* p;

	h = 0;
	for(p = (unsigned char*) key; *p != '\0'; p++)
		h = MULTIPLIER * h + *p;
	return h % dic->n_hash;
}

static unsigned int int_hash(const dictionary* dic, const void* key)
{
	const int* n = (const int*) key;

	return *n % dic->n_hash;	
}

dictionary* dic_new_withstr()
{
	return dic_new(str_duplicator, free, str_comparator, str_hash);
}

dictionary* dic_new_withint()
{
	return dic_new(int_duplicator, free, int_comparator, int_hash);
}

