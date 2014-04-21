#include "test_voicecall.h"
#include "testmacros.h"

#include "sound.h"
#include "voicecall.h"
#include "errors.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <arpa/inet.h>

#define CALL_BUFF_NUMSAMPLES 100
#define CALL_DURATION_SEC 5

#define IP(a,b,c,d) (htonl(((a) << 24) + ((b) << 16) + ((c) << 8) + (d)))
 
char *_callbuf;
int _callbuf_index = 0;
int _callbuf_numsamples;
pthread_mutex_t _call_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _call_cond = PTHREAD_COND_INITIALIZER;
int _offending_index = -1;
int _received_packets = 0;
int _sent_packets = 0;
pthread_t _receiver_verifying;
pthread_t _sender_verifying;

static int init_callbuf(int num_samples, int sample_size)
{
    size_t buflen = num_samples * sample_size * sizeof(char);
    int i, j;

    _callbuf = malloc(buflen);

    if (!_callbuf)
        return ERR_MEM;

    _callbuf_numsamples = num_samples;

    srand(clock());

    for (i = 0; i < num_samples; i++)
    {
        _callbuf[i] = i;

        for (j = 1; j < sample_size; j++)
            _callbuf[i + j] = (char) rand();
    }

    return OK;
}

static int _mock_send_buffer(char *buf, int size)
{
    memcpy(buf, _callbuf + _callbuf_index * size, size);
    _callbuf_index = (_callbuf_index + 1) % _callbuf_numsamples;

    if(pthread_equal(pthread_self(), _sender_verifying))   
        _sent_packets++;

    if(_callbuf_index == _callbuf_numsamples)
        sleep(CALL_DURATION_SEC);

    return OK;
}

static int _mock_receive_buffer(char *buf, int size)
{
    if(!pthread_equal(pthread_self(), _receiver_verifying))
        _received_packets++;

    return OK;
}

/* BEGIN TESTS */
int t_voicecall__20_sec_transmission__works()
{
    struct cm_info info_a, info_b;
    int socket_a = 0, socket_b = 0;
    int port_a, port_b;
    int format = PA_SAMPLE_ALAW, channels = 1, sample_time = 20;
    int retval;
    struct timeval tv;
    struct timespec ts;

    // Tarda mucho, no se controla bien la pérdida de paquetes UDP.
    mu_ignore;

    _set_use_mocks(1);

    _set_play_cb(_mock_receive_buffer);
    _set_record_cb(_mock_send_buffer);

    retval = init_callbuf(CALL_BUFF_NUMSAMPLES, getBytesPerSample(format, channels, sample_time));

    if (retval != OK)
        mu_sysfail("init_callbuf");

    socket_a = open_listen_socket();
    if (socket_a <= 0)
        mu_cleanup_sysfail(cleanup, "open_listen_socket a");

    socket_b = open_listen_socket();
    if (socket_b <= 0)
        mu_cleanup_sysfail(cleanup, "open_listen_socket b");

    retval = get_socket_port(socket_a, &port_a);
    if (retval != OK)
        mu_cleanup_sysfail(cleanup, "get_socket_port a");

    retval = get_socket_port(socket_b, &port_b);
    if (retval != OK)
        mu_cleanup_sysfail(cleanup, "get_socket_port b");

    retval = spawn_call_manager_thread(&info_a, IP(127, 0, 0, 1), port_b, socket_a, format, channels, sample_time);
    _sender_verifying = info_a.sender_pth;
    mu_assert_eq(retval, OK, "spawn call a");


    retval = spawn_call_manager_thread(&info_b, IP(127, 0, 0, 1), port_a, socket_b, format, channels, sample_time);
    _receiver_verifying = info_b.player_pth;
    mu_assert_eq(retval, OK, "spawn call b");


    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec + CALL_DURATION_SEC;
    ts.tv_nsec = tv.tv_usec / 1000;

    retval = OK;

    pthread_mutex_lock(&_call_mutex);
    {
        while (_offending_index == -1 && retval != ETIMEDOUT)
            retval = pthread_cond_timedwait(&_call_cond, &_call_mutex, &ts);
    }
    pthread_mutex_unlock(&_call_mutex);

    call_stop(&info_a);
    call_stop(&info_b);

    mu_assert_eq(_offending_index, -1, "wrong packet received");
    mu_assert("no packets sent", _sent_packets > 0);
    mu_assert("no packets received", _received_packets > 0);
    mu_assert_eq(_sent_packets, _received_packets, "different number of received and sent packets");

    retval = MU_PASSED;

cleanup:

    _set_play_cb(NULL);
    _set_record_cb(NULL);

    if (socket_a > 0)
        close(socket_a);

    if (socket_b > 0)
        close(socket_b);

    if (_callbuf)
        free(_callbuf);

    mu_cleanup_end;
}
int t_getBytesPerSample__one_sample__size_is_correct()
{
    mu_assert_eq((int) getBytesPerSample(PA_SAMPLE_ALAW, 1, 20), 160, "PA_SAMPLE_ALAW");
    mu_assert_eq((int) getBytesPerSample(PA_SAMPLE_ULAW, 1, 20), 160, "PA_SAMPLE_ULAW");
    mu_assert_eq((int) getBytesPerSample(PA_SAMPLE_S16BE, 1, 20), 1764, "PA_SAMPLE_S16BE 1CH");
    mu_assert_eq((int) getBytesPerSample(PA_SAMPLE_S16BE, 2, 20), 3528, "PA_SAMPLE_S16BE 2CH");
    mu_end;
}
int t_getFormatBps__all_formats__size_is_correct()
{
    mu_assert_eq((int) getFormatBps(PA_SAMPLE_ALAW, 1), 64000, "PA_SAMPLE_ALAW");
    mu_assert_eq((int) getFormatBps(PA_SAMPLE_ULAW, 1), 64000, "PA_SAMPLE_ULAW");
    mu_assert_eq((int) getFormatBps(PA_SAMPLE_S16BE, 1), 705600, "PA_SAMPLE_S16BE 1CH");
    mu_assert_eq((int) getFormatBps(PA_SAMPLE_S16BE, 2), 1411200, "PA_SAMPLE_S16BE 2CH");

    mu_end;
}
/* END TESTS */

int test_voicecall_suite(int *errors, int *success)
{
    int tests_run = 0;
    int tests_passed = 0;

    printf("Begin test_voicecall suite.\n");
    /* BEGIN TEST EXEC */
    mu_run_test(t_voicecall__20_sec_transmission__works);
    mu_run_test(t_getBytesPerSample__one_sample__size_is_correct);
    mu_run_test(t_getFormatBps__all_formats__size_is_correct);
    /* END TEST EXEC */
    if (tests_passed == tests_run)
        printf("End test_voicecall suite. " TGREEN "%d/%d\n\n" TRESET, tests_passed, tests_run);
    else
        printf("End test_voicecall suite. " TRED "%d/%d\n\n" TRESET, tests_passed, tests_run);


    *errors += (tests_run - tests_passed);
    *success += tests_passed;
    return tests_run;
}
