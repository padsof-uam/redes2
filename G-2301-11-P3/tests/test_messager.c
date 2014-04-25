#include "errors.h"
#include "test_messager.h"
#include "messager.h"
#include "testmacros.h"
#include "sockutils.h"
#include "ssltrans.h"

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/un.h>
#include <pthread.h>

struct th_data
{
    const void *data;
    int size;
    int socket;
};

static int _get_sockpair(short use_ssl, int sockets[2])
{
    int retval;

    if (!use_ssl)
        retval = socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets);
    else
        retval = ssl_socketpair(PF_INET, SOCK_STREAM, 0, sockets);

    return retval;
}

static char *_rand_data(size_t len)
{
    int i;
    char *buffer = malloc(len * sizeof(char));
    srand(clock());

    for (i = 0; i < len; i++)
        buffer[i] = rand();

    return buffer;
}

static int _test_msgsize_send(int size, short use_ssl)
{
    char *data = _rand_data(size);
    char *rcvbuf = malloc(size * sizeof(char));
    int sockets[2];
    int r_sck, s_sck;
    int rcv_data = 0;
    int i;
    int retval = MU_PASSED;

    if (_get_sockpair(use_ssl, sockets) == -1)
    {
        mu_cleanup_sysfail(nosock_cleanup, "socketpair");
    }

    r_sck = sockets[0];
    s_sck = sockets[1];

    if (send_message(s_sck, data, size) != OK)
    {
        ERR_print_errors_fp(stderr);
        mu_cleanup_sysfail(cleanup, "send_message");
    }

    if (!sock_data_available(r_sck))
    {
        ERR_print_errors_fp(stderr);
        mu_cleanup_fail(cleanup, "No data available.");
    }

    rcv_data = rcv_message_staticbuf(r_sck, rcvbuf, size);

    if(rcv_data <= 0)
    {
        perror("rcv");
        ERR_print_errors_fp(stderr);
    }

    mu_cleanup_assert_eq(cleanup, rcv_data, size, "didn't receive all data");

    for (i = 0; i < size; i++)
    {
        if (rcvbuf[i] != data[i])
        {
            mu_cleanup_fail(cleanup, "received data differs");
        }
    }

cleanup:
    close(r_sck);
    close(s_sck);
nosock_cleanup:
    free(rcvbuf);
    free(data);

    return retval;
}

static void *_send_msg(void *data)
{
    struct th_data *tdata = (struct th_data *)data;
    if (send_message(tdata->socket, tdata->data, tdata->size) < 0)
        return NULL;
    else
        return (void *) 1;
}

static int _test_msgsize_send_async(int size, short use_ssl)
{
    char *data = _rand_data(size);
    char *rcvbuf = malloc(size * sizeof(char));
    int sockets[2];
    int r_sck, s_sck;
    int rcv_data = 0;
    int i;
    int retval = MU_PASSED;
    pthread_t send_t;
    struct th_data tdata;

    if (_get_sockpair(use_ssl, sockets) == -1)
    {
        mu_cleanup_sysfail(nosock_cleanup, "socketpair");
    }

    r_sck = sockets[0];
    s_sck = sockets[1];

    tdata.data = data;
    tdata.size = size;
    tdata.socket = s_sck;

    if (pthread_create(&send_t, NULL, _send_msg, &tdata) < 0)
    {
        mu_cleanup_sysfail(cleanup, "pthread_create");
    }

    usleep(200); /* Chapuza. */

    if (!sock_data_available(r_sck))
    {
        ERR_print_errors_fp(stderr);
        mu_cleanup_fail(cleanup, "No data available.");
    }

    rcv_data = rcv_message(r_sck, (void **)(&rcvbuf));

    if(rcv_data <= 0)
    {
        perror("rcv");
        ERR_print_errors_fp(stderr);
    }

    mu_cleanup_assert_eq(cleanup, rcv_data, size, "didn't receive all data");

    for (i = 0; i < size; i++)
    {
        if (rcvbuf[i] != data[i])
        {
            mu_cleanup_fail(cleanup, "received data differs");
        }
    }

cleanup:
    close(r_sck);
    close(s_sck);
nosock_cleanup:
    free(rcvbuf);
    free(data);

    return retval;
}

/* BEGIN TESTS */
int t_rcv_message__bigmsg__received()
{
    int retval;
    retval = _test_msgsize_send_async(65000, 0);

    mu_assert_eq(retval, MU_PASSED, "operation failed");

    mu_end;
}
int t_rcv_message__normalmsg__received()
{
    int retval;
    retval = _test_msgsize_send_async(512, 0);

    mu_assert_eq(retval, MU_PASSED, "operation failed");

    mu_end;
}
int t_send_message__big_message__sent()
{
    /* No podemos hacerlo muy grande que si no hay que esperar
         a que el otro extremo lea y es un lío enorme para probarlo. */
    int retval = _test_msgsize_send(5000, 0);

    mu_assert_eq(retval, MU_PASSED, "operation failed");
    mu_end;
}

int t_send_message__normal_len__message_sent()
{
    int retval = _test_msgsize_send(512, 0);

    mu_assert_eq(retval, MU_PASSED, "operation failed");
    mu_end;
}

int t_rcv_message__bigmsg_ssl__received()
{
    int retval;
    init_all_ssl_default();
    retval = _test_msgsize_send_async(65000, 1);

    mu_assert_eq(retval, MU_PASSED, "operation failed");

    cleanup_all_ssl();
    mu_end;
}
int t_rcv_message__normalmsg_ssl__received()
{
    int retval;
    init_all_ssl_default();
    retval = _test_msgsize_send_async(512, 1);

    mu_assert_eq(retval, MU_PASSED, "operation failed");

    cleanup_all_ssl();
    mu_end;
}
int t_send_message__big_message_ssl__sent()
{
    /* No podemos hacerlo muy grande que si no hay que esperar
         a que el otro extremo lea y es un lío enorme para probarlo. */
    int retval;

    init_all_ssl_default();
    retval = _test_msgsize_send(5000, 1);
    cleanup_all_ssl();

    mu_end;
}

int t_send_message__normal_len_ssl__message_sent()
{
    int retval;

    init_all_ssl_default();
    retval = _test_msgsize_send(512, 1);
    cleanup_all_ssl();

    mu_end;
}

int t_send_message__negative_len__returns_err()
{
    char data[3];
    mu_assert_eq((int)send_message(0, data, -1), ERR, "didn't return err");
    mu_end;
}
int t_send_message__msg_is_null__returns_err()
{
    mu_assert_eq((int)sendmsg(0, NULL, 12), ERR, "didn't return err");
    mu_end;
}

/* END TESTS */

int test_messager_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_messager suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_rcv_message__bigmsg__received);
    mu_run_test(t_rcv_message__normalmsg__received);
    mu_run_test(t_send_message__big_message__sent);
    mu_run_test(t_send_message__normal_len__message_sent);
    mu_run_test(t_rcv_message__bigmsg_ssl__received);
    mu_run_test(t_rcv_message__normalmsg_ssl__received);
    mu_run_test(t_send_message__big_message_ssl__sent);
    mu_run_test(t_send_message__normal_len_ssl__message_sent);
    mu_run_test(t_send_message__msg_is_null__returns_err);

    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_messager suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_messager suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);

    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}
