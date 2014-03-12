#ifndef STRINGS_H
#define STRINGS_H

#include <sys/types.h>

/**
 * Elimina caracteres de espacio del principio y final de la cadena.
 * Además de espacios, se eliminan tabuladores y demás caracteres que no
 * 	representan caracteres visibles.
 * @see isspace(3)
 * @param str Cadena.
 */
void strip(char** str);

/**
 * Encuentra la primera ocurrencia de la cadena _find_ en _s_.
 * @param  s    Cadena donde buscar.
 * @param  find Cadena a buscar.
 * @param  slen Longitud máxima que se buscará.
 * @return      Puntero a la posición de inicio de _find_ en _s_ o NULL 
 *                 si no se ha encontrado.
 */
char * strnstr(const char* s, const char* find, size_t slen);

/**
 * Separa una cadena en una matriz pasada como argumento
 * @param  str    Cadena
 * @param  sep    Separadores
 * @param  array  Matriz donde se guardan los valores
 * @param  arrlen Longitud del array para guardar valores
 * @return        Número de valores separados.
 */
int str_arrsep(const char* str, const char* sep, char** array, size_t arrlen);
#endif

