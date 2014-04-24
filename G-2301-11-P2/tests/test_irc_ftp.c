#include "test_irc_ftp.h"
#include "testmacros.h"
#include "irc_ftp.h"
#include "errors.h"

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TOSEND_FILE "/tmp/tosend"
#define TORCV_FILE "/tmp/torcv"

ftp_status glob_status_snd = ftp_started;
ftp_status glob_status_rcv = ftp_started;
pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stop_cond = PTHREAD_COND_INITIALIZER;

void ftp_glob_change_snd(ftp_status status){  

	pthread_mutex_lock(&stop_mutex);
	{
		glob_status_snd = status;
		pthread_cond_signal(&stop_cond);
	}
	pthread_mutex_unlock(&stop_mutex);

}
void ftp_glob_change_rcv(ftp_status status){

	pthread_mutex_lock(&stop_mutex);
	{
		glob_status_rcv = status;
		pthread_cond_signal(&stop_cond);
	}
	pthread_mutex_unlock(&stop_mutex);
}


/* BEGIN TESTS */
int t_irc_ftp__transfer_complete__ok() {
	/* Preparación del fichero a enviar. */
	char payload_snd[] = "01234";	
	int len = 6,retval;
	char * payload_read = calloc(sizeof(char),len+1);
	FILE * tosend;
	pthread_t  snd_ftp_th=0,recv_ftp_th=0;
	uint32_t ip = inet_addr("127.0.0.1");
	int * port = malloc(sizeof(int));


	tosend = fopen(TOSEND_FILE,"w");
	if (tosend == NULL)
		mu_fail("No se puede abrir el fichero de envío");
	fwrite(payload_snd, sizeof(char), len, tosend);
	fclose(tosend);

	/**
	 * Llamadas a funciones.
	 */

	retval = ftp_wait_file(TORCV_FILE, port, ftp_glob_change_rcv, &recv_ftp_th);
	mu_assert_eq(retval, OK, "Fallando la preparación para recepción ftp");

	retval = ftp_send_file(TOSEND_FILE, ip, *port, ftp_glob_change_snd, &snd_ftp_th);
	mu_assert_eq(retval, OK, "Fallando la preparación para envío ftp");    	
	/**
	 * Fin de la ejecución.
	 */

	/*
	Esperamos a que acaben. ARREGLAR
	 */
	pthread_mutex_lock(&stop_mutex);
	{
	while (glob_status_rcv == ftp_started || glob_status_snd == ftp_started)
		pthread_cond_wait(&stop_cond, &stop_mutex);
	}
	pthread_mutex_unlock(&stop_mutex);

	mu_assert("Condición de recepción ha fallado", glob_status_rcv == ftp_finished);
	mu_assert("Condición de envío ha fallado", glob_status_snd == ftp_finished);


	FILE * torcv = fopen(TORCV_FILE,"r");
	if (torcv == NULL)
		mu_fail("No se puede abrir el fichero de recepción.");
	fread(payload_read, sizeof(char), len, torcv);
	fclose(torcv);


	mu_assert_streq(payload_read, payload_snd, "No coinciden");

	mu_end;
}
/* END TESTS */

int test_irc_ftp_suite(int *errors, int *success)
{
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_ftp suite.\n");
	/* BEGIN TEST EXEC */
	mu_run_test(t_irc_ftp__transfer_complete__ok);
	/* END TEST EXEC */
	if (tests_passed == tests_run)
		printf("End test_irc_ftp suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_ftp suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}
