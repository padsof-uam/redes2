#include "G-2301-11-P3-list_helpers.h"
 
#ifndef DICTIONARY_H
#define DICTIONARY_H

#define NHASH 103
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
	duplicator key_dup;
	destructor key_des;
	comparator key_comp;
	hash_function hash;
	int items;
};

/**
 * Create a new dictionary.
 * @param  key_dp Duplicator for the keys.
 * @param  key_dc Destructor for the keys.
 * @param  comp   Key comparator.
 * @param  hash   Hash function.
 * @return        New dictionary.
 */
dictionary* dic_new(duplicator key_dp, destructor key_dc, comparator comp, hash_function hash);

/**
 * Add a new value to the dictionary. The key is 
 * 	duplicated and stored.
 * @param  dic   Dictionary
 * @param  key   Key
 * @param  value value
 * @return       OK/ERR
 */
int dic_add(dictionary* dic, const void* key, void* value);

/**
 * Return number of items in the dictionary.
 * @param  dic Dictionary
 * @return     Count.
 */
int dic_count(dictionary* dic);

/**
 * Remove an element from the dictionary.
 * @param  dic Dictionary
 * @param  key Key for the element.
 * @return     OK or ERR_NOTFOUND.
 */
int dic_remove(dictionary* dic, const void* key);

/**
 * Try to find an element in the dictionary.
 * @param dic Dictionary
 * @param key Key
 * @return 	  element or NULL if it wasn't found	
 */
void* dic_lookup(dictionary* dic, const void* key);

/**
 * Try to find an element in the dictionary, create 
 * 	it if it wasn't found.
 * @param dic   Dictionary
 * @param key   Key
 * @param value Value to insert in the new element.
 * @return 		New value.
 */
void* dic_lookup_create(dictionary* dic, const void* key, void* value);

/**
 * Update an element in the dictionary.
 * 	It creates it if it wasn't found.
 * @param  dic   Dictionary
 * @param  key   Key
 * @param  value Value
 * @return       OK.
 */
int dic_update(dictionary* dic, const void* key, void* value);

/**
 * -- INTERNAL --
 * Return the key/val pair for the given key.
 * 	If it wasn't found and value is not equal to 0,
 * 	the function creates the value.
 * @param  dic    Dictionary
 * @param  key    Key
 * @param  value  Value
 * @param  create Create flag.
 * @return        key/value pair.
 */
kv_pair* _dic_lookup(dictionary* dic, const void* key, void* value, int create);

/**
 * Destroy the dictionary, optionally destroying 
 * 	each element with the given destructor.
 * @param dic Dictionary
 * @param dc  Optional. Destructor function.
 */
void dic_destroy(dictionary* dic, destructor dc);

/**
 * Iterate through each element of the dictionary.
 * @param  dic          Dictionary
 * @param  action       Action to execute.
 * @param  pass_through Pass through for the action.
 * @return              OK/ERR
 */
int dic_iterate(dictionary* dic, iterator_action action, void* pass_through);

/* Funciones para crear diccionarios sin tener que pasar todos los argumentos */
dictionary* dic_new_withint();
dictionary* dic_new_withstr();

#endif
