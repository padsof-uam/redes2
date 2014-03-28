#ifndef SYSUTILS_H
#define SYSUTILS_H

#include <sys/resource.h>

#define MAX_KILL_WAIT_MS 5000

/**
 * Escribe el PID del proceso actual en un archivo.
 * @param  file Ruta del archivo.
 * @return      OK/ERR.
 */
int write_pid(const char* file);

/**
 * Lee el PID de un proceso del archivo y lo termina.
 * El proceso de finalización se realiza enviando una señal
 * 	SIGTERM. Si después de MAX_KILL_WAIT_MS el proceso no ha 
 * 	terminado, se eliminará con SIGKILL
 * @see kill(2)
 * @param  file Nombre del archivo con el PID.
 * @return      0 si el proceso ha salido normalmente, 1 si ha habido que forzar salida
 *                o -1 si ocurrió un error.
 */
int read_pid_and_kill(const char* file);

/**
 * Trata de obtener bloqueo exclusivo sobre un archivo.
 * @see flock(2)
 * @return 0 si no se pudo obtener bloqueo, 1 si se pudo obtener y -1 si hubo error.
 */
int try_getlock(const char* file);

/**
 * Obtiene el límite para un recurso dato.
 * @see getrlimit(2)
 * @param  resource Recurso (RLIMIT_*)
 * @return          Límite.
 */
rlim_t get_soft_limit(int resource);

/**
 * Instala el gestor de backtrace.
 * @return OK/ERR.
 */
int install_stop_handlers();
#endif

