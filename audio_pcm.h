/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef __PCM_H__
#define __PCM_H__

#include "AudioConfig.h"

struct pcm;

typedef unsigned long snd_pcm_uframes_t;
typedef signed long snd_pcm_sframes_t;
typedef unsigned int __u32;

#define __force
#define __bitwise
#define __user

enum
{
    SNDRV_PCM_STREAM_PLAYBACK = 0,
    SNDRV_PCM_STREAM_CAPTURE,
    SNDRV_PCM_STREAM_LAST = SNDRV_PCM_STREAM_CAPTURE,
};

typedef int __bitwise snd_pcm_access_t;
#define SNDRV_PCM_ACCESS_MMAP_INTERLEAVED ((__force snd_pcm_access_t) 0)
#define SNDRV_PCM_ACCESS_MMAP_NONINTERLEAVED ((__force snd_pcm_access_t) 1)
#define SNDRV_PCM_ACCESS_MMAP_COMPLEX ((__force snd_pcm_access_t) 2)
#define SNDRV_PCM_ACCESS_RW_INTERLEAVED ((__force snd_pcm_access_t) 3)
#define SNDRV_PCM_ACCESS_RW_NONINTERLEAVED ((__force snd_pcm_access_t) 4)
#define SNDRV_PCM_ACCESS_LAST SNDRV_PCM_ACCESS_RW_NONINTERLEAVED

typedef int __bitwise snd_pcm_format_t;
#define SNDRV_PCM_FORMAT_S8 ((__force snd_pcm_format_t) 0)
#define SNDRV_PCM_FORMAT_U8 ((__force snd_pcm_format_t) 1)
#define SNDRV_PCM_FORMAT_S16_LE ((__force snd_pcm_format_t) 2)
#define SNDRV_PCM_FORMAT_S16_BE ((__force snd_pcm_format_t) 3)
#define SNDRV_PCM_FORMAT_U16_LE ((__force snd_pcm_format_t) 4)
#define SNDRV_PCM_FORMAT_U16_BE ((__force snd_pcm_format_t) 5)
#define SNDRV_PCM_FORMAT_S24_LE ((__force snd_pcm_format_t) 6)
#define SNDRV_PCM_FORMAT_S24_BE ((__force snd_pcm_format_t) 7)
#define SNDRV_PCM_FORMAT_U24_LE ((__force snd_pcm_format_t) 8)
#define SNDRV_PCM_FORMAT_U24_BE ((__force snd_pcm_format_t) 9)
#define SNDRV_PCM_FORMAT_S32_LE ((__force snd_pcm_format_t) 10)
#define SNDRV_PCM_FORMAT_S32_BE ((__force snd_pcm_format_t) 11)
#define SNDRV_PCM_FORMAT_U32_LE ((__force snd_pcm_format_t) 12)
#define SNDRV_PCM_FORMAT_U32_BE ((__force snd_pcm_format_t) 13)
#define SNDRV_PCM_FORMAT_FLOAT_LE ((__force snd_pcm_format_t) 14)
#define SNDRV_PCM_FORMAT_FLOAT_BE ((__force snd_pcm_format_t) 15)
#define SNDRV_PCM_FORMAT_FLOAT64_LE ((__force snd_pcm_format_t) 16)
#define SNDRV_PCM_FORMAT_FLOAT64_BE ((__force snd_pcm_format_t) 17)
#define SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE ((__force snd_pcm_format_t) 18)
#define SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE ((__force snd_pcm_format_t) 19)
#define SNDRV_PCM_FORMAT_MU_LAW ((__force snd_pcm_format_t) 20)
#define SNDRV_PCM_FORMAT_A_LAW ((__force snd_pcm_format_t) 21)
#define SNDRV_PCM_FORMAT_IMA_ADPCM ((__force snd_pcm_format_t) 22)
#define SNDRV_PCM_FORMAT_MPEG ((__force snd_pcm_format_t) 23)
#define SNDRV_PCM_FORMAT_GSM ((__force snd_pcm_format_t) 24)
#define SNDRV_PCM_FORMAT_SPECIAL ((__force snd_pcm_format_t) 31)
#define SNDRV_PCM_FORMAT_S24_3LE ((__force snd_pcm_format_t) 32)
#define SNDRV_PCM_FORMAT_S24_3BE ((__force snd_pcm_format_t) 33)
#define SNDRV_PCM_FORMAT_U24_3LE ((__force snd_pcm_format_t) 34)
#define SNDRV_PCM_FORMAT_U24_3BE ((__force snd_pcm_format_t) 35)
#define SNDRV_PCM_FORMAT_S20_3LE ((__force snd_pcm_format_t) 36)
#define SNDRV_PCM_FORMAT_S20_3BE ((__force snd_pcm_format_t) 37)
#define SNDRV_PCM_FORMAT_U20_3LE ((__force snd_pcm_format_t) 38)
#define SNDRV_PCM_FORMAT_U20_3BE ((__force snd_pcm_format_t) 39)
#define SNDRV_PCM_FORMAT_S18_3LE ((__force snd_pcm_format_t) 40)
#define SNDRV_PCM_FORMAT_S18_3BE ((__force snd_pcm_format_t) 41)
#define SNDRV_PCM_FORMAT_U18_3LE ((__force snd_pcm_format_t) 42)
#define SNDRV_PCM_FORMAT_U18_3BE ((__force snd_pcm_format_t) 43)
#define SNDRV_PCM_FORMAT_LAST SNDRV_PCM_FORMAT_U18_3BE

#ifdef SNDRV_LITTLE_ENDIAN
#define SNDRV_PCM_FORMAT_S16 SNDRV_PCM_FORMAT_S16_LE
#define SNDRV_PCM_FORMAT_U16 SNDRV_PCM_FORMAT_U16_LE
#define SNDRV_PCM_FORMAT_S24 SNDRV_PCM_FORMAT_S24_LE
#define SNDRV_PCM_FORMAT_U24 SNDRV_PCM_FORMAT_U24_LE
#define SNDRV_PCM_FORMAT_S32 SNDRV_PCM_FORMAT_S32_LE
#define SNDRV_PCM_FORMAT_U32 SNDRV_PCM_FORMAT_U32_LE
#define SNDRV_PCM_FORMAT_FLOAT SNDRV_PCM_FORMAT_FLOAT_LE
#define SNDRV_PCM_FORMAT_FLOAT64 SNDRV_PCM_FORMAT_FLOAT64_LE
#define SNDRV_PCM_FORMAT_IEC958_SUBFRAME SNDRV_PCM_FORMAT_IEC958_SUBFRAME_LE
#endif
#ifdef SNDRV_BIG_ENDIAN
#define SNDRV_PCM_FORMAT_S16 SNDRV_PCM_FORMAT_S16_BE
#define SNDRV_PCM_FORMAT_U16 SNDRV_PCM_FORMAT_U16_BE
#define SNDRV_PCM_FORMAT_S24 SNDRV_PCM_FORMAT_S24_BE
#define SNDRV_PCM_FORMAT_U24 SNDRV_PCM_FORMAT_U24_BE
#define SNDRV_PCM_FORMAT_S32 SNDRV_PCM_FORMAT_S32_BE
#define SNDRV_PCM_FORMAT_U32 SNDRV_PCM_FORMAT_U32_BE
#define SNDRV_PCM_FORMAT_FLOAT SNDRV_PCM_FORMAT_FLOAT_BE
#define SNDRV_PCM_FORMAT_FLOAT64 SNDRV_PCM_FORMAT_FLOAT64_BE
#define SNDRV_PCM_FORMAT_IEC958_SUBFRAME SNDRV_PCM_FORMAT_IEC958_SUBFRAME_BE
#endif

typedef int __bitwise snd_pcm_subformat_t;
#define SNDRV_PCM_SUBFORMAT_STD ((__force snd_pcm_subformat_t) 0)
#define SNDRV_PCM_SUBFORMAT_LAST SNDRV_PCM_SUBFORMAT_STD

#define SNDRV_PCM_INFO_MMAP 0x00000001
#define SNDRV_PCM_INFO_MMAP_VALID 0x00000002
#define SNDRV_PCM_INFO_DOUBLE 0x00000004
#define SNDRV_PCM_INFO_BATCH 0x00000010
#define SNDRV_PCM_INFO_INTERLEAVED 0x00000100
#define SNDRV_PCM_INFO_NONINTERLEAVED 0x00000200
#define SNDRV_PCM_INFO_COMPLEX 0x00000400
#define SNDRV_PCM_INFO_BLOCK_TRANSFER 0x00010000
#define SNDRV_PCM_INFO_OVERRANGE 0x00020000
#define SNDRV_PCM_INFO_RESUME 0x00040000
#define SNDRV_PCM_INFO_PAUSE 0x00080000
#define SNDRV_PCM_INFO_HALF_DUPLEX 0x00100000
#define SNDRV_PCM_INFO_JOINT_DUPLEX 0x00200000
#define SNDRV_PCM_INFO_SYNC_START 0x00400000
#define SNDRV_PCM_INFO_FIFO_IN_FRAMES 0x80000000

typedef int __bitwise snd_pcm_state_t;
#define SNDRV_PCM_STATE_OPEN ((__force snd_pcm_state_t) 0)
#define SNDRV_PCM_STATE_SETUP ((__force snd_pcm_state_t) 1)
#define SNDRV_PCM_STATE_PREPARED ((__force snd_pcm_state_t) 2)
#define SNDRV_PCM_STATE_RUNNING ((__force snd_pcm_state_t) 3)
#define SNDRV_PCM_STATE_XRUN ((__force snd_pcm_state_t) 4)
#define SNDRV_PCM_STATE_DRAINING ((__force snd_pcm_state_t) 5)
#define SNDRV_PCM_STATE_PAUSED ((__force snd_pcm_state_t) 6)
#define SNDRV_PCM_STATE_SUSPENDED ((__force snd_pcm_state_t) 7)
#define SNDRV_PCM_STATE_DISCONNECTED ((__force snd_pcm_state_t) 8)
#define SNDRV_PCM_STATE_LAST SNDRV_PCM_STATE_DISCONNECTED

typedef int snd_pcm_hw_param_t;
#define SNDRV_PCM_HW_PARAM_ACCESS 0
#define SNDRV_PCM_HW_PARAM_FORMAT 1
#define SNDRV_PCM_HW_PARAM_SUBFORMAT 2
#define SNDRV_PCM_HW_PARAM_FIRST_MASK SNDRV_PCM_HW_PARAM_ACCESS
#define SNDRV_PCM_HW_PARAM_LAST_MASK SNDRV_PCM_HW_PARAM_SUBFORMAT

#define SNDRV_PCM_HW_PARAM_SAMPLE_BITS 8
#define SNDRV_PCM_HW_PARAM_FRAME_BITS 9
#define SNDRV_PCM_HW_PARAM_CHANNELS 10
#define SNDRV_PCM_HW_PARAM_RATE 11
#define SNDRV_PCM_HW_PARAM_PERIOD_TIME 12
#define SNDRV_PCM_HW_PARAM_PERIOD_SIZE 13
#define SNDRV_PCM_HW_PARAM_PERIOD_BYTES 14
#define SNDRV_PCM_HW_PARAM_PERIODS 15
#define SNDRV_PCM_HW_PARAM_BUFFER_TIME 16
#define SNDRV_PCM_HW_PARAM_BUFFER_SIZE 17
#define SNDRV_PCM_HW_PARAM_BUFFER_BYTES 18
#define SNDRV_PCM_HW_PARAM_TICK_TIME 19
#define SNDRV_PCM_HW_PARAM_FIRST_INTERVAL SNDRV_PCM_HW_PARAM_SAMPLE_BITS
#define SNDRV_PCM_HW_PARAM_LAST_INTERVAL SNDRV_PCM_HW_PARAM_TICK_TIME

#define SNDRV_PCM_HW_PARAMS_NORESAMPLE (1<<0)

#define SNDRV_MASK_MAX 256

#define SNDRV_PCM_SYNC_PTR_HWSYNC (1<<0)
#define SNDRV_PCM_SYNC_PTR_APPL (1<<1)
#define SNDRV_PCM_SYNC_PTR_AVAIL_MIN (1<<2)
/*
 * PCM API
 */

#define PCM_OUT        0x00000000
#define PCM_IN         0x10000000
#define PCM_MMAP       0x00000001
#define PCM_NOIRQ      0x00000002
#define PCM_NORESTART  0x00000004 /* PCM_NORESTART - when set, calls to
                                   * pcm_write for a playback stream will not
                                   * attempt to restart the stream in the case
                                   * of an underflow, but will return -EPIPE
                                   * instead.  After the first -EPIPE error, the
                                   * stream is considered to be stopped, and a
                                   * second call to pcm_write will attempt to
                                   * restart the stream.
                                   */
#define PCM_MONOTONIC  0x00000008 /* see pcm_get_htimestamp */
#define PCM_NOBUFFER   0x00000010

/* PCM runtime states */
#define PCM_STATE_OPEN      0
#define PCM_STATE_SETUP     1
#define PCM_STATE_PREPARED  2
#define PCM_STATE_RUNNING   3
#define PCM_STATE_XRUN      4
#define PCM_STATE_DRAINING  5
#define PCM_STATE_PAUSED    6
#define PCM_STATE_SUSPENDED 7
#define PCM_STATE_DISCONNECTED  8

/* Configuration for a stream */
struct pcm_config
{
    unsigned int channels;
    unsigned int rate;
    unsigned int bits;
    unsigned int period_size;
    unsigned int period_count;
};

/* PCM parameters */
enum pcm_param
{
    /* mask parameters */
    PCM_PARAM_ACCESS,
    PCM_PARAM_FORMAT,
    PCM_PARAM_SUBFORMAT,
    /* interval parameters */
    PCM_PARAM_SAMPLE_BITS,
    PCM_PARAM_FRAME_BITS,
    PCM_PARAM_CHANNELS,
    PCM_PARAM_RATE,
    PCM_PARAM_PERIOD_TIME,
    PCM_PARAM_PERIOD_SIZE,
    PCM_PARAM_PERIOD_BYTES,
    PCM_PARAM_PERIODS,
    PCM_PARAM_BUFFER_TIME,
    PCM_PARAM_BUFFER_SIZE,
    PCM_PARAM_BUFFER_BYTES,
    PCM_PARAM_TICK_TIME,
};

struct pcm *pcm_open(const char *dev_name, int flag);
int pcm_set_config(struct pcm *pcm_dev, struct pcm_config config);
int pcm_start(struct pcm *pcm_dev);
int pcm_prepare(struct pcm *pcm_dev);
int pcm_start(struct pcm *pcm_dev);
int pcm_stop(struct pcm *pcm_dev);
int pcm_close(struct pcm *pcm_dev);
unsigned long pcm_write(struct pcm *pcm_dev, void *data, unsigned long bytes);
unsigned long pcm_read(struct pcm *pcm_dev, void *data, unsigned long bytes);

#endif
