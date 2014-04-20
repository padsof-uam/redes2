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

int spawn_call_manager_thread(struct cm_info *cm, uint32_t ip, uint16_t port, int socket)
{
    struct cm_thdata *thdata = malloc(sizeof(struct cm_thdata));
    struct sockaddr_in dst_addr;
    int retval = ERR_SYS;

    if (socket == 0)
        socket = open_listen_socket();

    bzero(cm, sizeof(struct cm_info));

    thdata->dst_ip = ip;
    thdata->dst_port = port;
    thdata->socket = socket;
    thdata->ssrc = generate_ssrc();
    thdata->id = cm;
    thdata->ringbuf = lfringbuf_new(VC_RINGBUF_CAP, VC_PAYLOAD_SIZE);

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

    if (pthread_create(&(cm->receiver_pth), NULL, sound_sender_entrypoint, thdata) != 0)
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

void *sound_sender_entrypoint(void *data)
{
    struct rtp_header packet;
    struct cm_thdata *thdata = (struct cm_thdata *) data;
    int retval;

    bzero(&packet, sizeof packet);

    openRecord(VC_RECORD_ID);

    packet.version = 2;
    packet.padding = 0;
    packet.extension = 0;
    packet.contributor_count = 0;
    packet.marker = 0;
    packet.payload_type = sampleFormat(PA_SAMPLE_ALAW, 1);

    packet.seq = 0;
    packet.ssrc_id = thdata->ssrc;

    while (1)
    {
        thdata->sndr_status = VC_OK;

        packet.timestamp = get_timestamp();
        packet.seq++;

        if ((retval = recordSound(packet.payload, VC_PAYLOAD_SIZE)) != 0)
        {
            slog(LOG_ERR, "Error grabando sonido: %d. Terminando.", retval);
            call_stop(thdata->id);
            return NULL;
        }

        if (send_message(thdata->socket, &packet, sizeof packet) != OK)
        {
            slog(LOG_ERR, "Error enviando a través del socket: %s. Terminando.", strerror(errno));
            call_stop(thdata->id);
            return NULL;
        }
    }
}

void *sound_player_entrypoint(void *data)
{
    struct cm_thdata *thdata = (struct cm_thdata *) data;
    char buffer[VC_PAYLOAD_SIZE];

    openPlay(VC_STREAM_ID);

    while (1)
    {
        lfringbuf_wait_for_items(thdata->ringbuf, -1);

        if (lfringbuf_pop(thdata->ringbuf, buffer) == OK)
            playSound(buffer, VC_PAYLOAD_SIZE);
    }

    return NULL;
}

void *sound_receiver_entrypoint(void *data)
{
    struct cm_thdata *thdata = (struct cm_thdata *) data;
    struct rtp_header packet;
    struct pollfd pfd;
    int poll_retval;
    int psize;

    pfd.events = POLLIN;
    pfd.fd = thdata->socket;

    while ((poll_retval = poll(&pfd, 1, -1)) == 1)
    {
        psize = rcv_message_staticbuf(thdata->socket, &packet, sizeof packet);

        if (psize == 0)
        {
            slog(LOG_ERR, "Llamada cerrada en remoto. Sin avisar. Eso no se hace.");
            thdata->recv_status = VC_CALL_ENDED;
            call_stop(thdata->id);
            return NULL;
        }
        else if (psize != sizeof packet)
        {
            slog(LOG_WARNING, "Hemos recibido un paquete de longitud menor... ¿qué hacemos?");
            continue;
        }

        lfringbuf_push(thdata->ringbuf, packet.payload);
    }

    return NULL;
}

uint32_t get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}


int call_stop(struct cm_info *cm)
{
    pthread_cancel_join(&(cm->player_pth));
    pthread_cancel_join(&(cm->receiver_pth));
    pthread_cancel_join(&(cm->sender_pth));

    return OK;
}

uint32_t generate_ssrc()
{
    /* Podemos implementar lo que pone en el RFC, o hacer esta chapuza. */
    return rand();
}
