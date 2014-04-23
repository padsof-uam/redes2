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

/* BEGIN TESTS */
ftp_status glob_status_snd = 0;
ftp_status glob_status_rcv = 0;

pthread_mutex_t stop_mutex_snd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stop_mutex_rcv = PTHREAD_MUTEX_INITIALIZER;

void ftp_glob_change_snd(ftp_status status){
    pthread_mutex_lock(&stop_mutex_snd);
    {
		glob_status_snd = status;
	}
    pthread_mutex_unlock(&stop_mutex_snd);

}
void ftp_glob_change_rcv(ftp_status status){
    pthread_mutex_lock(&stop_mutex_rcv);
    {
		glob_status_rcv = status;
	}
    pthread_mutex_unlock(&stop_mutex_rcv);
}


int t_irc_ftp__transfer_complete__ok() {
	/* Preparación del fichero a enviar. */
	char payload_snd[] = "01234";	
	int len = 6,retval;
	char * payload_read = calloc(sizeof(char),len+1);
	FILE * tosend;
	pthread_t  snd_ftp_th=0,recv_ftp_th=0;
	uint32_t ip = inet_addr("192.168.1.5");
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
	while((glob_status_rcv != ftp_finished || glob_status_rcv != ftp_aborted) && (glob_status_snd != ftp_finished || glob_status_snd != ftp_aborted));



	FILE * torcv = fopen(TORCV_FILE,"r");
	if (torcv == NULL)
		mu_fail("No se puede abrir el fichero de recepción.");
	fread(payload_read, sizeof(char), len, torcv);
	fclose(torcv);


	mu_assert_streq(payload_snd, payload_read, "No coinciden");

	mu_fail("Not implemented");
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
