#include "test_irc_ftp.h"
#include "testmacros.h"
#include "irc_ftp.h"
#include "errors.h"
#include "messager.h"

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

static char *_rand_data(size_t len)
{
	int i;
	char *buffer = malloc(len * sizeof(char));
	srand(clock());

	for(i = 0; i < len; i++)
		buffer[i] = rand();

	return buffer;
}

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
int t_irc_ftp__transfer_big_file__ok() {
	glob_status_snd = ftp_started;
	glob_status_rcv = ftp_started;

	int len = 3*MAX_LEN_FTP+6,retval;
	char * payload_snd = _rand_data(len);	
	FILE * tosend, *torcv;
	pthread_t  snd_ftp_th=0,recv_ftp_th=0;
	uint32_t ip = inet_addr("127.0.0.1");
	int * port = malloc(sizeof(int));
	char ch1,ch2;


	tosend = fopen(TOSEND_FILE,"w");
	if (tosend == NULL)
		mu_fail("No se puede abrir el fichero de envío");
	fwrite(payload_snd, sizeof(char), len, tosend);
	fclose(tosend);


	retval = ftp_wait_file(TORCV_FILE, port, ftp_glob_change_rcv, &recv_ftp_th);
	mu_assert_eq(retval, OK, "Fallando la preparación para recepción ftp");

	retval = ftp_send_file(TOSEND_FILE, ip, *port, ftp_glob_change_snd, &snd_ftp_th);
	mu_assert_eq(retval, OK, "Fallando la preparación para envío ftp");    	

	/* Espera a que finalice la transmisión. */
	pthread_mutex_lock(&stop_mutex);
	{
		while (glob_status_rcv == ftp_started || glob_status_snd == ftp_started)
			pthread_cond_wait(&stop_cond, &stop_mutex);
	}
	pthread_mutex_unlock(&stop_mutex);

	mu_assert("Condición de recepción ha fallado", glob_status_rcv == ftp_finished);
	mu_assert("Condición de envío ha fallado", glob_status_snd == ftp_finished);


	torcv = fopen(TORCV_FILE,"r");
	if (torcv == NULL)
		mu_fail("No se puede abrir el fichero de recepción.");

	tosend = fopen(TOSEND_FILE,"r");
	if (tosend == NULL)
		mu_fail("No se puede abrir el fichero de envío");


	do{
		ch1 = getc(torcv);
		ch2 = getc(tosend) ;
	}while( (ch1!=EOF) && (ch2!=EOF) && (ch1 == ch2));

	mu_assert_eq(ch1, ch2, "Los contenidos de los ficheros no son iguales");

	mu_end;
}

int t_irc_ftp__transfer__timeout() {
	
	glob_status_rcv = ftp_started;
	char payload_snd[] = "01234";	
	pthread_t recv_ftp_th=0;
	int * port = malloc(sizeof(int));
	struct sockaddr_in dst_addr;
	int sock,retval;

	retval = ftp_wait_file(TORCV_FILE, port, ftp_glob_change_rcv, &recv_ftp_th);
	mu_assert_eq(retval, OK, "Fallando la preparación para recepción ftp");

	/* Nos preparamos para enviar con timeout. */
	bzero(&dst_addr, sizeof dst_addr);
	dst_addr.sin_addr.s_addr =  inet_addr("127.0.0.1");;
	dst_addr.sin_port = *port;
	dst_addr.sin_family = PF_INET;

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connect(sock, (struct sockaddr *)&dst_addr, sizeof dst_addr) == -1)
	{
		mu_fail("Error conectando");
		return ERR_SOCK;
	}

	send_message(sock,payload_snd,sizeof(long));

	/* Espera del timeout */
	sleep(2*FTP_TIMEOUT);

	/* Espera a que finalice la transmisión. */
	pthread_mutex_lock(&stop_mutex);
	{
		while (glob_status_rcv == ftp_started)
			pthread_cond_wait(&stop_cond, &stop_mutex);
	}
	pthread_mutex_unlock(&stop_mutex);

	mu_assert_eq(glob_status_rcv, ftp_timeout, "Condición de recepción ha fallado");

	mu_end;
}
/* END TESTS */

int test_irc_ftp_suite(int *errors, int *success)
{
	int tests_run = 0;
	int tests_passed = 0;

	printf("Begin test_irc_ftp suite.\n");
	/* BEGIN TEST EXEC */
	mu_run_test(t_irc_ftp__transfer__timeout);
	mu_run_test(t_irc_ftp__transfer_big_file__ok);
	/* END TEST EXEC */
	if (tests_passed == tests_run)
		printf("End test_irc_ftp suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
	else
		printf("End test_irc_ftp suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


	*errors += (tests_run - tests_passed);
	*success += tests_passed;
	return tests_run;
}
