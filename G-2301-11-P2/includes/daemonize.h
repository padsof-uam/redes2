#ifndef DAEMONIZE_H
#define DAEMONIZE_H 

/**
 * Función que daemoniza el proceso.
 * @param  log_id Nombre del log.
 * @return        OK/ERR
 */
int daemonize();

void open_syslog(const char* id);

/**
 * @internal
 * Cierra los ficheros que estén abiertos.
 * @return OK/ERR.
 */
int close_open_fds();
/**
 * @internal
 * Crea un nuevo proceso descolgado.
 * @return OK/ERR.
 */
int unlink_proc();

#endif
