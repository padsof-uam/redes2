#include "voicecall.h"
#include "types.h"
#include "log.h"
#include "sound.h"
#include "messager.h"
#include "lfringbuf.h"
#include "sysutils.h"
#include "irc_processor.h"
#include "gui_client.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/time.h>

#define ERR_THRESHOLD 5
#define RUNNING_AVG_FACTOR 0.2
#define PACKET_MIN_DROP_MS_SEP 200
#define LATENCY_THRESHOLD_MS 150

#define __sync_retrieve(val) __sync_add_and_fetch(&(val), 0)

#define rtp_payload(buffer) (buffer + sizeof(struct rtp_header))

#define mark_delta(msg) do { \
        tsa = tsb; \
        tsb = get_timestamp(); \
        slog(LOG_DEBUG, "Time delta %d ns: %s", tsb - tsa, msg); \
    } while(0)

#define delta_reset() tsb = get_timestamp()

struct cm_thdata
{
    uint32_t dst_ip;
    uint16_t dst_port;
    int socket;
    struct cm_info *id;
    short recv_status;
    short sndr_status;
    short player_status;
    uint32_t ssrc;
    lfringbuf *ringbuf;
    int payload_size;
    int sample_duration_ms;
    int payload_format;
    int play_latency;
    volatile sig_atomic_t stop;
};

int open_listen_socket()
{
    int sock;
    struct sockaddr_in addr;

    bzero(&addr, sizeof(struct sockaddr_in));

    addr.sin_family = PF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = INADDR_ANY;

    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock == -1)
        return ERR_SOCK;

    if (bind(sock, (const struct sockaddr *) &addr, sizeof(struct sockaddr_in) ) == -1)
    {
        close(sock);
        return ERR_SOCK;
    }

    return sock;
}

int get_socket_port(int sock, int *port)
{
    struct sockaddr_in addr;
    socklen_t sa_size = sizeof(struct sockaddr_in);

    if (getsockname(sock, (struct sockaddr *)&addr, &sa_size) == -1)
        return ERR_SOCK;

    *port = addr.sin_port;

    return OK;
}

int spawn_call_manager_thread(struct cm_info *cm, uint32_t ip, uint16_t port, int socket, int format, int channels, int chunk_time_ms)
{
    struct cm_thdata *thdata = malloc(sizeof(struct cm_thdata));
    struct sockaddr_in dst_addr;
    int retval = ERR_SYS;

    if (socket == 0)
        socket = open_listen_socket();

    bzero(cm, sizeof(struct cm_info));

    thdata->payload_format = sampleFormat(format, channels);
    thdata->payload_size = getBytesPerSample(format, channels, chunk_time_ms);
    thdata->sample_duration_ms = chunk_time_ms;
    thdata->dst_ip = ip;
    thdata->dst_port = port;
    thdata->socket = socket;
    thdata->ssrc = generate_ssrc();
    thdata->id = cm;
    thdata->stop = 0;
    thdata->ringbuf = lfringbuf_new(VC_RINGBUF_CAP, thdata->payload_size);

    if (!thdata->ringbuf)
    {
        slog(LOG_CRIT, "Error creando un buffer circular de tamaño %ld", VC_RINGBUF_CAP * thdata->payload_size);
        return ERR_MEM;
    }

    cm->thdata = thdata;

    bzero(&dst_addr, sizeof dst_addr);
    dst_addr.sin_addr.s_addr = ip;
    dst_addr.sin_port = port;
    dst_addr.sin_family = PF_INET;

    if (connect(socket, (struct sockaddr *)&dst_addr, sizeof dst_addr) == -1)
    {
        slog(LOG_CRIT, "Error asociando socket UDP al otro extremo de la conexión: %s", strerror(errno));
        retval = ERR_SOCK;
        goto error;
    }

    fcntl(socket, F_SETFL, O_NONBLOCK);

    if (pthread_create(&(cm->receiver_pth), NULL, sound_receiver_entrypoint, thdata) != 0)
    {
        slog(LOG_CRIT, "Error creando hilo de recepción de sonido: %s", strerror(errno));
        goto error;
    }

    if (pthread_create(&(cm->sender_pth), NULL, sound_sender_entrypoint, thdata) != 0)
    {
        slog(LOG_CRIT, "Error creando hilo de envío de sonido: %s", strerror(errno));
        goto error;
    }

    if (pthread_create(&(cm->player_pth), NULL, sound_player_entrypoint, thdata) != 0)
    {
        slog(LOG_CRIT, "Error creando hilo de reproducción de sonido: %s", strerror(errno));
        goto error;
    }

    return OK;

error:
    call_stop(cm);
    return retval;
}

static void _cancel_record(void *data)
{
    closeRecord();
}


void *sound_sender_entrypoint(void *data)
{
    char *packet;
    struct rtp_header *header;
    size_t buf_size;
    struct cm_thdata *thdata = (struct cm_thdata *) data;
    int retval;
    int send_errors = 0;
    int max_send_error_threshold = ERR_THRESHOLD;

    buf_size = sizeof(struct rtp_header) + thdata->payload_size;

    packet = calloc(buf_size, sizeof(char));

    if (!packet)
    {
        slog(LOG_CRIT, "No se ha podido reservar memoria (%d bytes) para el buffer de envío: %s", buf_size, strerror(errno));
        call_stop(thdata->id);
        return NULL;
    }

    slog(LOG_DEBUG, "Memoria reservada, tamaño %d", buf_size);

    pthread_cleanup_push(_cancel_record, NULL);
    pthread_cleanup_push(free, packet);

    openRecord(VC_RECORD_ID);

    header = (struct rtp_header *) packet;

    header->version = 2;
    header->padding = 0;
    header->extension = 0;
    header->contributor_count = 0;
    header->marker = 0;
    header->payload_type = thdata->payload_format;

    header->seq = 0;
    header->ssrc_id = thdata->ssrc;

    while (!thdata->stop)
    {
        thdata->sndr_status = VC_OK;

        header->timestamp = get_timestamp();
        header->seq++;

        retval = recordSound(rtp_payload(packet), thdata->payload_size);

        if (retval != 0)
        {
            slog(LOG_ERR, "Error grabando sonido: %d (%s). Terminando.", retval, pa_strerror(retval));
            call_stop(thdata->id);
            return NULL;
        }

        if (send_message(thdata->socket, packet, buf_size) != OK)
        {
            slog(LOG_ERR, "Error (%d) enviando datos de sonido a través del socket: %s.", send_errors, strerror(errno));
            send_errors++;

            if (send_errors >= max_send_error_threshold)
            {
                slog(LOG_ERR, "Superado margen de errores (%d) en sender. Terminando llamada.", max_send_error_threshold);
                call_stop(thdata->id);
                return NULL;
            }

        }
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return NULL;
}

static void _cancel_play(void *data)
{
    closePlay();
}

void *sound_player_entrypoint(void *data)
{
    struct cm_thdata *thdata = (struct cm_thdata *) data;
    char *buffer;
    size_t buf_size;
    int retval;
    long latency;

    buf_size = thdata->payload_size;

    buffer = calloc(buf_size, sizeof(char));

    if (!buffer)
    {
        slog(LOG_CRIT, "No se ha podido reservar memoria (%d bytes) para el buffer de reproducción: %s", buf_size, strerror(errno));
        call_stop(thdata->id);
        return NULL;
    }

    pthread_cleanup_push(free, buffer);
    pthread_cleanup_push(_cancel_play, NULL);

    openPlay(VC_STREAM_ID);

    while (!thdata->stop)
    {
        latency = getMsLatencyPlay();
        __sync_lock_test_and_set(&(thdata->play_latency), latency);

        if (thdata->ringbuf)
            retval = lfringbuf_wait_for_items(thdata->ringbuf, -1);
        else
            retval = ERR;

        if (retval != OK)
        {
            slog(LOG_ERR, "Error al esperar nuevos elementos en lfringbuf.");
            return NULL;
        }

        if (lfringbuf_pop(thdata->ringbuf, buffer) == OK)
        {
            retval = playSound(buffer, thdata->payload_size);
            usleep((thdata->sample_duration_ms - 5) * 1000);

            if (retval != 0)
                slog(LOG_ERR, "Error reproduciendo sonido: %s", pa_strerror(retval));
        }
        else
        {
            slog(LOG_ERR, "lfringbuf pop error");
        }
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return NULL;
}

static void _list_cleanup(void *ls)
{
    list_destroy((list *) ls, free);
}

static char *memdup(char *buffer, size_t size)
{
    char *new = malloc(size);

    if (!new) return NULL;

    memcpy(new, buffer, size);

    return new;
}

static char *_get_packet_with_seq(list *ls, int seq)
{
    int i;
    struct rtp_header *p;

    for (i = 0; i < list_count(ls); i++)
    {
        p = list_at(ls, i);
        if (p->seq == seq)
        {
            list_remove(ls, i);
            return (char *) p;
        }
    }

    return NULL;
}

static void _update_running_avg(double* avg, int newval)
{
    if(*avg < 0)
        *avg = newval;
    else
        *avg = (1 - RUNNING_AVG_FACTOR) * (*avg) + RUNNING_AVG_FACTOR * newval;
}

void *sound_receiver_entrypoint(void *data)
{
    struct cm_thdata *thdata = (struct cm_thdata *) data;
    struct rtp_header *packet;
    char *buffer;
    size_t buf_size;
    struct pollfd pfd;
    int poll_retval;
    int psize;
    list *pending_packets = list_new();
    int last_seq = 0;
    char *pending_packet_buf;
    char *blank_buf;
    int recv_errors = 0, max_recv_err_threshold = ERR_THRESHOLD;
    int last_force_drop_ts = -1;
    int current_ts;
    int empty_buffer = 0;
    int queue_delay_ms, tx_delay_ms;
    double tx_delay_avg = -1;
    int packets_per_sec = 1000 / ((double) thdata->sample_duration_ms);

    pfd.events = POLLIN;
    pfd.fd = thdata->socket;

    buf_size = sizeof(struct rtp_header) + thdata->payload_size;

    buffer = calloc(buf_size, sizeof(char));
    blank_buf = calloc(buf_size, sizeof(char));


    if (!buffer || !blank_buf)
    {
        if (blank_buf) free(blank_buf);
        if (buffer) free(buffer);

        slog(LOG_CRIT, "No se ha podido reservar memoria (%d bytes) para el buffer de recepción: %s", buf_size, strerror(errno));
        call_stop(thdata->id);
        return NULL;
    }

    pthread_cleanup_push(free, buffer);
    pthread_cleanup_push(free, blank_buf);
    pthread_cleanup_push(_list_cleanup, pending_packets);

    packet = (struct rtp_header *) buffer;

    while (!thdata->stop && (poll_retval = poll(&pfd, 1, -1)) == 1)
    {
        psize = rcv_message_staticbuf(thdata->socket, buffer, buf_size);

        if (psize <= 0)
        {
            slog(LOG_ERR, "Error de recepción (%d): %s", recv_errors, strerror(errno));
            recv_errors++;

            if (recv_errors >= max_recv_err_threshold)
            {
                slog(LOG_ERR, "Margen de errores de recepción (%d) alcanzado. Saliendo.", max_recv_err_threshold);
                thdata->recv_status = VC_CALL_ENDED;
                call_stop(thdata->id);
                return NULL;
            }
            else
            {
                continue;
            }
        }
        else if (psize != buf_size)
        {
            slog(LOG_WARNING, "Hemos recibido un paquete de longitud menor (%d). Ignorando.", psize);
            continue;
        }

        if (thdata->stop)
            break;

        if (packet->seq != last_seq + 1)
        {
            list_add(pending_packets, memdup(buffer, buf_size));
            
            /* Más de 10 paquetes pendientes, damos el que nos falta por perdido */
            if(list_count(pending_packets) >= 10)
            {
                slog(LOG_DEBUG, "Paquete con secuencia %d dado por perdido.", last_seq + 1);
                empty_buffer = 1;
                last_seq++;
            }
            else
            {
                continue;
            }
        }

        last_seq++;

        current_ts = get_timestamp();
        tx_delay_ms = current_ts - packet->timestamp;
        queue_delay_ms = lfringbuf_count(thdata->ringbuf) * thdata->sample_duration_ms + __sync_retrieve(thdata->play_latency);

        _update_running_avg(&tx_delay_avg, tx_delay_ms);

        if(last_seq % packets_per_sec == 0)
            slog(LOG_DEBUG, "Current tx calculated delay: %f ms. Queue delay: %d ms", tx_delay_avg, queue_delay_ms);

        if(abs(queue_delay_ms - tx_delay_avg) > LATENCY_THRESHOLD_MS && 
            (current_ts - last_force_drop_ts > PACKET_MIN_DROP_MS_SEP))
        {
            last_force_drop_ts = current_ts;
            slog(LOG_DEBUG, "Dropping packet, trying to reduce latency.");
            continue;
        }
        
        if (thdata->ringbuf)
        {
            lfringbuf_push(thdata->ringbuf, rtp_payload(buffer));

            while (empty_buffer || (pending_packet_buf = _get_packet_with_seq(pending_packets, last_seq + 1)) != NULL)
            {
                lfringbuf_push(thdata->ringbuf, pending_packet_buf);
                last_seq++;
                free(pending_packet_buf);
            }

            empty_buffer = 0;
        }
    }

    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);

    return NULL;
}

uint32_t get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (tv.tv_sec % 10000) * 1000 + tv.tv_usec / 1000;
}


int call_stop(struct cm_info *cm)
{
    lfringbuf *rb;

    if (!cm)
        return ERR;

    slog(LOG_DEBUG, "Cerrando llamada.");

    cm->thdata->stop = 1;

    if (cm->thdata->ringbuf)
        lfringbuf_signal_destroying(cm->thdata->ringbuf);

    usleep(100 * 1000);


    slog(LOG_DEBUG, "Cerrando sender...");
    pthread_cancel_join(&(cm->sender_pth));
    slog(LOG_DEBUG, "Cerrando receiver...");
    pthread_cancel_join(&(cm->receiver_pth));
    slog(LOG_DEBUG, "Cerrando player...");
    pthread_cancel_join(&(cm->player_pth));

    rb = cm->thdata->ringbuf;
    cm->thdata->ringbuf = NULL;

    if (rb)
        lfringbuf_destroy(rb);

    close(cm->thdata->socket);

    slog(LOG_INFO, "Llamada cerrada, recursos liberados");

    return OK;
}

uint32_t generate_ssrc()
{
    /* Podemos implementar lo que pone en el RFC, o hacer esta chapuza. */
    return rand();
}
