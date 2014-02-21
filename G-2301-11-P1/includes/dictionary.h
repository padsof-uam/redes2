#include "list_helpers.h"
 
#ifndef DICTIONARY_H
#define DICTIONARY_H

#define NHASH 757
#define MULTIPLIER 31

typedef struct dictionary dictionary;

typedef unsigned int (*hash_function)(const dictionary* dic, const void* key);

typedef struct kv_pair kv_pair;

struct kv_pair {
	char* key;
	void* value;
	struct kv_pair* next;
};

struct dictionary {
	kv_pair** symtab;
	int n_hash;
	duplicator val_dup;
	destructor val_des;
	duplicator key_dup;
	destructor key_des;
	comparator key_comp;
	hash_function hash;
	int items;
};

dictionary* dic_new(duplicator key_dp, destructor key_dc, duplicator val_dup, destructor val_dc, comparator comp, hash_function hash);
int dic_add(dictionary* dic, const void* key, void* value);
int dic_count(dictionary* dic);
int dic_remove(dictionary* dic, const void* key);
void* dic_lookup(dictionary* dic, const void* key);
void* dic_lookup_create(dictionary* dic, const void* key, void* value);
int dic_update(dictionary* dic, const void* key, void* value);
kv_pair* _dic_lookup(dictionary* dic, const void* key, void* value, int create);
void dic_destroy(dictionary* dic);
int dic_iterate(dictionary* dic, iterator_action action, void* pass_through);

/* Funciones para crear diccionarios sin tener que pasar todos los argumentos */
dictionary* dic_new_withint(duplicator dp, destructor dc);
dictionary* dic_new_withstr(duplicator dp, destructor dc);

#endif
