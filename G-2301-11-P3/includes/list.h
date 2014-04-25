#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include "errors.h"
#include "list_helpers.h"

/**
 * Lista enlazada de memoria dinámica. 
 */
typedef struct {
	void** array;
	int count;
	int capacity;
} list;


/**
 * Crea una lista nueva de punteros a void.
 * @return Un puntero a la lista.
 */
list* list_new();

/**
 * Libera la memoria utilizada por una lista.
 * @param ls lista
 * @param dc función para liberar la memoria de los elemenos.
 */
void list_destroy(list* ls, destructor dc);

/**
 * Vacía la lista sin liberar la memoria. Se marca como que no tiene elementos.
 * @param  ls La lista a liberar.
 * @return    OK/ERR
 */
int list_clear(list* ls);

/**
 * Añade a la lista un elemento en la última posición.
 * @param  ls      La lista en la que insertar el elemento.
 * @param  element Elemento a insertar en la lista.
 * @return         OK/ERR.
 */
int list_add(list* ls, void* element);

/**
 * Inserta en la lista un elemento en la posición indicada.
 * @param  ls      La lista en la que insertar.
 * @param  element Elemento a insertar (como void *)
 * @param  index   Posición en la que insertar el elemento. 
 *                 No se genera una nueva copia del elemento, sino que se guarda el puntero. 
 *                 Si ya había un elemento en esa posición se sobreescribe la información.
 * @return         Ok/ERR
 */
int list_insert(list* ls, void* element, int index);

/**
 * Elimina un elemento de la lista, poniendo el último elemento de la lista en la posición indicada.
 * @param  ls      La lista en la que eliminar el elemento.
 * @param  element Posición de la que eliminar el elemento.
 * @return         OK si se ha podido eliminar, ERR_RANGE si la posición no es válida.
 */
int list_remove(list* ls, int position);

/**
 * Elimina el último elemento de la lista.
 * @param  ls Lista de la que eliminar el elemento.
 * @see		list_temove
 * @return    OK/ERR.
 */		
int list_remove_last(list *ls);

/**
 * Busca y elimina un elemento de la lista.
 * @param  ls      Lista de la que eliminar el elemento.
 * @param  comp    Función comparadora de elementos.
 * @param  element Elemento a eliminar de la lista (si se encuentra)
 * @return         OK si se ha podido eliminar, ERR_NOTFOUND si no se ha encontrado el elemento.
 */
int list_remove_element(list* ls, comparator comp, void* element);

/**
 * Busca en la lista un elemento.
 * @param  ls      Lista en la que buscar.
 * @param  comp    Función comparadora de elementos.
 * @param  element Elemento a buscar.
 * @return         La posición en la que se encuentra el elemento o -1 si no se ha encontrado.
 */
int list_find(list* ls, comparator comp, const void * element);


/**
 * Devuelve el elemento que ocupa una posición determinada.
 * @param ls    Lista de la que extraer el elemento.
 * @param index Posición de la que obtener el elemento.
 * @return		El elemento (como void *) que ocupa la posición dada o NULL si no se ha encontrado.
 */
void* list_at(list* ls, int index);

/**
 * Devuelve la longitud de la lista.
 * @param  ls Lista de la que devolver la longitud.
 * @return    La longitud de la lista.
 */
int list_count(list* ls);

#endif
