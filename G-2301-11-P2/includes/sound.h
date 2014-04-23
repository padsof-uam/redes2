#ifndef _SOUND_H
#define _SOUND_H

#include <pulse/simple.h>
#include <pulse/error.h>

typedef int (*sound_cb)(char*, int);

int sampleFormat(enum pa_sample_format format, int channels);
int _openRecord	(char *identificacion);
int _openPlay	(char *identificacion);
int _recordSound	(char * buf, int size);
int _playSound	(char * buf, int size);
void _closeRecord(void);
void _closePlay	(void);

int openRecord	(char *identificacion);
int openPlay	(char *identificacion);
int recordSound	(char * buf, int size);
int playSound	(char * buf, int size);
void closeRecord(void);
void closePlay	(void);

long getFormatBps(enum pa_sample_format format, int channels);
long getBytesPerSample(enum pa_sample_format format, int channels, int ms_samplelen);

int _openMock(char* identificacion);
int _closeMock();
int _set_record_cb(sound_cb cb);
int _set_play_cb(sound_cb cb);
int _playMock(char* buf, int size);
int _recordMock(char* buf, int size);

void _set_use_mocks(int use_mocks);

#endif
