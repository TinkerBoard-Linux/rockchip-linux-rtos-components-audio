/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
#include "audio_pcm.h"

struct
{
    int vol;
    int dB;
    bool changed;
} play_vol, rec_vol;

struct pcm
{
    void *device;
    int type;
    bool prepared;
    struct pcm_config config;
    void *user_data;
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

static int pcm_gain_inited = 0;
struct AUDIO_GAIN_INFO pcm_gain_info;
/* Generaye by EXP((32-A1)*-1*2.302585093/20) fit to human hearing */
static char pcm_vol_table[MAX_VOLUME_TABLE + 1] =
{
    255, 227, 201, 179, 159, 141, 125, 111,
    98, 87, 76, 67, 59, 52, 46, 40,
    35, 31, 27, 23, 20, 17, 15, 12,
    10, 9, 7, 6, 4, 3, 2, 1, 0
};

void pcm_gain_init(void *dev)
{
    rkdev_control(dev, RK_AUDIO_CTL_GET_GAIN_INFO, &pcm_gain_info);
    pcm_gain_inited = 1;
}

struct pcm *pcm_open(const int dev_id, int flag)
{
    struct pcm *pcm_dev = audio_malloc(sizeof(struct pcm));
    if (pcm_dev == NULL)
    {
        RK_AUDIO_LOG_E("pcm malloc failed\n");
        return NULL;
    }
    void *audio_dev = audio_device_open(dev_id, flag);
    if (audio_dev == NULL)
    {
        audio_free(pcm_dev);
        return NULL;
    }

    switch (flag)
    {
    case AUDIO_FLAG_RDONLY:
        pcm_dev->type = PCM_IN;
        break;

    case AUDIO_FLAG_WRONLY:
        pcm_dev->type = PCM_OUT;
        break;

    default:
        RK_AUDIO_LOG_E("unsupport flag:%d\n", flag);
        pcm_dev->type = PCM_OUT;
        break;
    }
    pcm_dev->device = audio_dev;
    pcm_dev->prepared = RK_AUDIO_FAIL;
    RK_AUDIO_LOG_V("%s:%p", __func__, audio_dev);

    return pcm_dev;
}

static unsigned long m_bytes_to_frames(struct pcm *pcm_dev, unsigned long bytes)
{
    return bytes / (pcm_dev->config.channels * (pcm_dev->config.bits >> 3));
}

int pcm_set_config(struct pcm *pcm_dev, struct pcm_config config)
{
    struct AUDIO_PARAMS aparams;
    int dB;
    int ret;

    if (!pcm_dev)
        return RK_AUDIO_FAILURE;

    pcm_dev->config = config;
    struct audio_buf *abuf = audio_malloc(sizeof(struct audio_buf));
    if (!abuf)
    {
        RK_AUDIO_LOG_E("malloc failed %d", sizeof(struct audio_buf));
        return RK_AUDIO_FAILURE;
    }
    pcm_dev->user_data = abuf;

    abuf->buf = (uint8_t *)audio_malloc_uncache(config.period_size * config.period_count);
    if (!abuf->buf)
    {
        RK_AUDIO_LOG_E("unchache malloc failed %d", config.period_size * config.period_count);
        return RK_AUDIO_FAILURE;
    }

    memset(abuf->buf, 0x00, config.period_size * config.period_count);
    abuf->buf_size = m_bytes_to_frames(pcm_dev, config.period_size * config.period_count);
    abuf->period_size = m_bytes_to_frames(pcm_dev, config.period_size);
    aparams.channels = config.channels;
    aparams.sampleRate = config.rate;
    aparams.sampleBits = config.bits;
    ret = audio_device_control(pcm_dev->device, RK_AUDIO_CTL_PCM_PREPARE, abuf);
    if (ret)
    {
        audio_free_uncache(abuf->buf);
        audio_free(abuf);
        return ret;
    }
    ret = audio_device_control(pcm_dev->device, RK_AUDIO_CTL_HW_PARAMS, &aparams);
    if (ret)
    {
        audio_free_uncache(abuf->buf);
        audio_free(abuf);
        return ret;
    }
    pcm_dev->prepared = RK_AUDIO_TRUE;
    if (!pcm_gain_inited)
        pcm_gain_init(pcm_dev->device);
    if (rec_vol.changed && pcm_dev->type == PCM_IN)
    {
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_SET_GAIN, &rec_vol.dB);
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_GET_GAIN, &rec_vol.dB);
        rec_vol.changed = RK_AUDIO_FAIL;
    }
    if (play_vol.changed && pcm_dev->type == PCM_OUT)
    {
        int dB = play_vol.vol ? (0 - pcm_vol_table[play_vol.vol] * pcm_gain_info.step) : pcm_gain_info.mindB;
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_SET_GAIN, &dB);
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_GET_GAIN, &play_vol.dB);
        play_vol.changed = RK_AUDIO_FAIL;
    }

    return 0;
}

int pcm_set_volume(struct pcm *pcm_dev, int vol, int flag)
{
    int dB;

    if (!pcm_dev || !pcm_dev->prepared)
    {
        switch (flag)
        {
        case AUDIO_FLAG_RDONLY:
            rec_vol.dB = vol;
            rec_vol.changed = RK_AUDIO_TRUE;
            return RK_AUDIO_SUCCESS;

        case AUDIO_FLAG_WRONLY:
            play_vol.vol = vol;
            play_vol.changed = RK_AUDIO_TRUE;
            return RK_AUDIO_SUCCESS;

        default:
            RK_AUDIO_LOG_E("unsupport flag:%d\n", flag);
            return RK_AUDIO_FAILURE;
        }
    }
    if (!pcm_gain_inited)
        pcm_gain_init(pcm_dev->device);
    switch (flag)
    {
    case AUDIO_FLAG_RDONLY:
        rec_vol.dB = vol;
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_SET_GAIN, &rec_vol.dB);
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_GET_GAIN, &rec_vol.dB);
        return RK_AUDIO_SUCCESS;

    case AUDIO_FLAG_WRONLY:
        play_vol.vol = vol;
        dB = vol ? (0 - pcm_vol_table[vol] * pcm_gain_info.step) : pcm_gain_info.mindB;
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_SET_GAIN, &dB);
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_GET_GAIN, &play_vol.dB);
        return RK_AUDIO_SUCCESS;

    default:
        RK_AUDIO_LOG_E("unsupport flag:%d\n", flag);
        return RK_AUDIO_FAILURE;
    }
}

int pcm_get_volume(struct pcm *pcm_dev, int flag)
{
    int vol, i;

    if (!pcm_dev || !pcm_dev->prepared)
    {
        switch (flag)
        {
        case AUDIO_FLAG_RDONLY:
            return rec_vol.dB;

        case AUDIO_FLAG_WRONLY:
            return play_vol.vol;

        default:
            RK_AUDIO_LOG_E("unsupport flag:%d\n", flag);
            return RK_AUDIO_FAILURE;
        }
    }
    if (!pcm_gain_inited)
        pcm_gain_init(pcm_dev->device);
    switch (flag)
    {
    case AUDIO_FLAG_RDONLY:
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_GET_GAIN, &rec_vol.dB);
        return rec_vol.dB;

    case AUDIO_FLAG_WRONLY:
        audio_device_control(pcm_dev->device, RK_AUDIO_CTL_GET_GAIN, &play_vol.dB);
        vol = -play_vol.dB / pcm_gain_info.step;
        for (i = 0; i <= MAX_VOLUME; i++)
        {
            if (pcm_vol_table[i] == vol)
                return i;
        }
        return play_vol.dB;

    default:
        RK_AUDIO_LOG_E("unsupport flag:%d\n", flag);
        return RK_AUDIO_FAILURE;
    }
}

int pcm_prepare(struct pcm *pcm_dev)
{
    return 0;
}

int pcm_start(struct pcm *pcm_dev)
{
    return 0;//audio_device_control(pcm_dev->device, RK_AUDIO_CTL_START, NULL);
}

int pcm_stop(struct pcm *pcm_dev)
{
    if (!pcm_dev)
        return RK_AUDIO_FAILURE;

    audio_device_control(pcm_dev->device, RK_AUDIO_CTL_STOP, NULL);
    audio_device_control(pcm_dev->device, RK_AUDIO_CTL_PCM_RELEASE, NULL);
    if (pcm_dev->user_data)
    {
        if (((struct audio_buf *)pcm_dev->user_data)->buf)
            audio_free_uncache(((struct audio_buf *)pcm_dev->user_data)->buf);
        audio_free(pcm_dev->user_data);
    }
    pcm_dev->prepared = RK_AUDIO_FAIL;

    return 0;
}

int pcm_close(struct pcm *pcm_dev)
{
    if (!pcm_dev)
        return RK_AUDIO_FAILURE;
    audio_device_close(pcm_dev->device);
    audio_free(pcm_dev);

    return 0;
}

unsigned long pcm_write(struct pcm *pcm_dev, void *data, unsigned long bytes)
{
    if (!pcm_dev)
        return RK_AUDIO_FAILURE;
    if (pcm_dev->type & PCM_IN)
        return PCM_WRONG_TYPE;
    if (bytes != pcm_dev->config.period_size)
        return PCM_WRONG_LENGTH;
    unsigned long frames = m_bytes_to_frames(pcm_dev, bytes);
    return audio_device_write(pcm_dev->device, data, frames);
}

unsigned long pcm_read(struct pcm *pcm_dev, void *data, unsigned long bytes)
{
    if (!pcm_dev)
        return RK_AUDIO_FAILURE;
    if (!(pcm_dev->type & PCM_IN))
        return PCM_WRONG_TYPE;
    if (bytes != pcm_dev->config.period_size)
        return PCM_WRONG_LENGTH;
    unsigned long frames = m_bytes_to_frames(pcm_dev, bytes);
    return audio_device_read(pcm_dev->device, data, frames);
}
