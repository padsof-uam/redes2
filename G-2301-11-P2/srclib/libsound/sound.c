#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pulse/simple.h>
#include <pulse/error.h>

pa_sample_spec ss = {
.format = PA_SAMPLE_S16BE,
.rate = 44100,
.channels = 2
};

pa_simple *sr = NULL, *sp = NULL;

int sampleFormat(enum pa_sample_format format, int channels)
{
  switch(format)
  {
    case PA_SAMPLE_ULAW:
    case PA_SAMPLE_ALAW:
      if (channels != 1) return -1;
      break;
    case PA_SAMPLE_S16BE:
      if (channels != 1 && channels != 2) return -1;
      break;
    default: return -1;
  }

  ss.format = format;
  ss.channels = channels;

  if (ss.format == PA_SAMPLE_ULAW) return 0;
  if (ss.format == PA_SAMPLE_ALAW) return 8;
  if (ss.format == PA_SAMPLE_S16BE) return 10;
  if (ss.format == PA_SAMPLE_S16BE) return 11;

  return -1;
}

int openRecord(char *identificacion)
{
  int error;

  if (!(sr = pa_simple_new(NULL, identificacion, PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error)))
    return error;
  return 0;
}

int openPlay(char *identificacion)
{
  int error;

  if (!(sp = pa_simple_new(NULL, identificacion, PA_STREAM_PLAYBACK, NULL, "record", &ss, NULL, NULL, &error)))
    return error;
  return 0;
}

void closeRecord(){pa_simple_free(sr);}
void closePlay(){pa_simple_free(sp);}

int recordSound(char * buf, int size)
{
  int error;

  if (pa_simple_read(sr, buf, (size_t) size, &error) < 0) return error;
  return 0;
}

int playSound(char * buf, int size)
{
  int error;

  if (pa_simple_write(sp, buf, (size_t) size, &error) < 0) return error;
  return 0;
}



