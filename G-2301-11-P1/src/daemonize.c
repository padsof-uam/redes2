#include "daemonize.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int daemonize(int argc, char const *argv[])
{
	int pid;

	umask(0);

	setlogmask (LOG_UPTO (LOG_INFO));
	openlog(NULL, 0, LOG_DAEMON);

	if (signal(SIGTTOU,SIG_IGN))
	{
		syslog(LOG_ERR, "Error en la captura de SIGTTOU");	
		return ERR;
	}
	if (signal(SIGTTIN,SIG_IGN))
	{
		syslog(LOG_ERR, "Error en la captura de SIGTTIN");	
		return ERR;
	}
	if (signal(SIGTSTP,SIG_IGN))
	{
		syslog(LOG_ERR, "Error en la captura de SIGTSTP");	
		return ERR;
	}

	pid = fork();
	if (pid>0) exit(EXIT_SUCCESS);
	if (pid<0) exit(EXIT_FAILURE);
	
	if(setsid() < 0){
		syslog(ERR, "Error creando un nuevo SID");
	}

	syslog(LOG_INFO, "Hijo creado como lider de la sesión");

	if ((chdir("/"))<0)
	{
		syslog(LOG_ERR, "Error cambiando el directorio de trabajo");
	}

	if (signal(SIGHUP,SIG_IGN))
	{
		syslog(LOG_ERR, "Error en la captura de SIGTSTP");
		exit(EXIT_FAILURE);
	}


	syslog(LOG_INFO, "Cerrando los descriptores standard");
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	return OK;
}
