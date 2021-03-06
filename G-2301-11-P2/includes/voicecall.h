#ifndef VOICECALL_H
#define VOICECALL_H

#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

#include "sound.h"

#define VC_OK 0
#define VC_CALL_ENDED 1
#define VC_ERR 2
#define VC_FUCKOFF 4
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

int open_listen_socket();
int spawn_call_manager_thread(struct cm_info *cm, uint32_t ip, uint16_t port, int socket, int format, int channels, int chunk_time_ms);
void* sound_sender_entrypoint(void* data);
void* sound_receiver_entrypoint(void* data);
void* sound_player_entrypoint(void* data);
int call_stop(struct cm_info* cm);
uint32_t generate_ssrc();
uint32_t get_timestamp();
int get_socket_port(int sock, int *port);

#endif
