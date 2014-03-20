#ifndef _SOUND_H
#define _SOUND_H

#include <pulse/simple.h>
#include <pulse/error.h>

int sampleFormat(enum pa_sample_format format, int channels);
int openRecord	(char *identificacion);
int openPlay	(char *identificacion);
int recordSound	(char * buf, int size);
int playSound	(char * buf, int size);
void closeRecord(void);
void closePlay	(void);

#endif
