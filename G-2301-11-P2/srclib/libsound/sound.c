#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pulse/simple.h>
#include <pulse/error.h>

#include "sound.h"

int _use_mocks = 0;
sound_cb _play_cb = NULL;
sound_cb _record_cb = NULL;

pa_sample_spec ss =
{
    .format = PA_SAMPLE_S16BE,
    .rate = 44100,
    .channels = 2
};

pa_simple *sr = NULL, *sp = NULL;

long getFormatBps(enum pa_sample_format format, int channels)
{
    int rate, bits_per_sample;

    if (format == PA_SAMPLE_ALAW || format == PA_SAMPLE_ULAW)
    {
        rate = 8000;
        bits_per_sample = 8;
    }
    else
    {
        rate = 44100;
        bits_per_sample = 16;
    }

    return rate * bits_per_sample * channels;
}


long getBytesPerSample(enum pa_sample_format format, int channels, int ms_samplelen)
{
    long bps = getFormatBps(format, channels);

    return (bps / 8) * ((double) ms_samplelen / 1000);
}

int sampleFormat(enum pa_sample_format format, int channels)
{
    switch (format)
    {
    case PA_SAMPLE_ULAW:
        case PA_SAMPLE_ALAW:
                if (channels != 1) return -1;
        ss.rate = 8000;
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

int _openRecord(char *identificacion)
{
    int error;

    if (!(sr = pa_simple_new(NULL, identificacion, PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error)))
        return error;
    return 0;
}

int _openPlay(char *identificacion)
{
    int error;

    if (!(sp = pa_simple_new(NULL, identificacion, PA_STREAM_PLAYBACK, NULL, "record", &ss, NULL, NULL, &error)))
        return error;
    return 0;
}

void _closeRecord()
{
    pa_simple_free(sr);
}
void _closePlay()
{
    pa_simple_free(sp);
}

int _recordSound(char *buf, int size)
{
    int error;

    if (pa_simple_read(sr, buf, (size_t) size, &error) < 0) return error;
    return 0;
}

int _playSound(char *buf, int size)
{
    int error;

    if (pa_simple_write(sp, buf, (size_t) size, &error) < 0) return error;
    return 0;
}

int _openMock(char *identificacion)
{
    return 0;
}
int _closeMock()
{
    return 0;
}

int _set_record_cb(sound_cb cb)
{
    _record_cb = cb;
    return 0;
}

int _set_play_cb(sound_cb cb)
{
    _play_cb = cb;
    return 0;
}


int _playMock(char *buf, int size)
{
    if (_play_cb)
        return _play_cb(buf, size);
    else
        return -1;
}

int _recordMock(char *buf, int size)
{
    if (_record_cb)
        return _record_cb(buf, size);
    else
        return -1;
}

int openRecord(char *id)
{
    if (!_use_mocks)
        return _openRecord(id);
    else
        return _openMock(id);
}

int openPlay(char *id)
{
    if (!_use_mocks)
        return _openPlay(id);
    else
        return _openMock(id);
}

int recordSound(char *buf, int size)
{
    if (!_use_mocks)
        return _recordSound(buf, size);
    else
        return _recordMock(buf, size);
}

int playSound(char *buf, int size)
{
    if (!_use_mocks)
        return _playSound(buf, size);
    else
        return _playMock(buf, size);
}

void closeRecord()
{
    if (!_use_mocks)
        _closeRecord();
    else
        _closeMock();
}

void closePlay()
{
    if (!_use_mocks)
        _closePlay();
    else
        _closeMock();
}

void _set_use_mocks(int use_mocks)
{
	_use_mocks = use_mocks;
}
