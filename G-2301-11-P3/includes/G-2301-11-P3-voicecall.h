#ifndef VOICECALL_H
#define VOICECALL_H

#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#include "G-2301-11-P3-sound.h"

#define VC_OK 0
#define VC_CALL_ENDED 1
#define VC_ERR 2
#define VC_TIMEOUT 8

#define VC_CHUNK_TIME_MS 20
#define VC_FORMAT PA_SAMPLE_ULAW
#define VC_CHANNELS 1
#define VC_PAYLOAD_SIZE 400


#define VC_RINGBUF_CAP 1000
#define VC_RECORD_ID "redirc-rec"
#define VC_STREAM_ID "redirc-str"

struct cm_info {
	pthread_t sender_pth;
	pthread_t receiver_pth;
	pthread_t player_pth;
	struct cm_thdata* thdata;
};

struct rtp_header {
	unsigned int version : 2;
	unsigned int padding : 1;
	unsigned int extension : 1;
	unsigned int contributor_count : 4;
	unsigned int marker : 1;
	unsigned int payload_type : 7;
	uint16_t seq;
	uint32_t timestamp;
	uint32_t ssrc_id;
};

/**
 * Crea los hilos encargados de gestionar la llamada.
 * @param  cm            Información de la llamada.
 * @param  ip            IP de destino.
 * @param  port          Puerto de destino.
 * @param  socket        Socket a usar (se creará si no es válido)
 * @param  format        Formato de llamada.
 * @param  channels      Canales a usar.
 * @param  chunk_time_ms Tiempo de cada segmento a enviear.
 * @return               OK/ERR
 */
int spawn_call_manager_thread(struct cm_info *cm, uint32_t ip, uint16_t port, int socket, int format, int channels, int chunk_time_ms);

/**
 * Punto de entrada del hilo de grabación y envío.
 * @param data Datos del hilo
 */
void* sound_sender_entrypoint(void* data);

/**
 * Punto de entrada del hilo de recepción.
 * @param data Datos del hilo
 */
void* sound_receiver_entrypoint(void* data);

/**
 * Punto de entrada del hilo de reproducción.
 * @param data Datos del hilo
 */
void* sound_player_entrypoint(void* data);

/**
 * Detiene una llamada
 * @param  cm Información de la llamada
 * @return    OK/ERR
 */
int call_stop(struct cm_info* cm);

/**
 * Genera un código ssrc.
 * @return Código SSRC.
 */
uint32_t generate_ssrc();

/**
 * Obtiene el timestamp actual
 * @return Timestamp.
 */
uint32_t get_timestamp();

#endif
