#include "../includes/daemonize.h"
#include "../includes/errors.h"
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
	if (pid>0) exit(OK);

	setsid();

	syslog(LOG_INFO, "Hijo creado como lider de la sesión");


	if (signal(SIGHUP,SIG_IGN))
	{
		syslog(LOG_ERR, "Error en la captura de SIGTSTP");
		return ERR;
	}

	umask(0);




	closelog();
	return 0;
}