/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
#include "audio_pcm.h"

struct pcm
{
    void *device;
    int type;
    struct pcm_config config;
    void *user_data;
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

struct pcm *pcm_open(const char *dev_name, int flag)
{
    struct pcm *pcm_dev = audio_malloc(sizeof(struct pcm));
    if (pcm_dev == NULL)
    {
        RK_AUDIO_LOG_E("pcm malloc failed\n");
        return NULL;
    }
    void *audio_dev = audio_device_open(dev_name, flag);
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
    memset(&aparams, 0x0, sizeof(aparams));
    aparams.channels = config.channels;
    aparams.sampleRate = config.rate;
    aparams.sampleBits = config.bits;
    pcm_dev->config = config;
    int ret;
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
    ret = audio_device_control(pcm_dev->device, RK_AUDIO_CTL_PCM_PREPARE, abuf);


    ret = audio_device_control(pcm_dev->device, RK_AUDIO_CTL_HW_PARAMS, &aparams);


    pcm_dev->user_data = abuf;

    return ret;
}

int pcm_prepare(struct pcm *pcm_dev)
{
    return 0;
}

int pcm_start(struct pcm *pcm_dev)
{
#ifdef OS_IS_FREERTOS
    audio_device_control(pcm_dev->device, RK_AUDIO_CTL_START, NULL);
#endif
    return 0;
}

int pcm_stop(struct pcm *pcm_dev)
{
    int ret = audio_device_control(pcm_dev->device, RK_AUDIO_CTL_STOP, NULL);
    ret = audio_device_control(pcm_dev->device, RK_AUDIO_CTL_PCM_RELEASE, NULL);

    return ret;
}

int pcm_close(struct pcm *pcm_dev)
{
    if (pcm_dev->user_data)
    {
        if (((struct audio_buf *)pcm_dev->user_data)->buf)
            audio_free_uncache(((struct audio_buf *)pcm_dev->user_data)->buf);
        audio_free(pcm_dev->user_data);
    }
    audio_device_close(pcm_dev->device);
    audio_free(pcm_dev);

    return 0;
}

unsigned long pcm_write(struct pcm *pcm_dev, void *data, unsigned long bytes)
{
    if (pcm_dev->type & PCM_IN)
        return -1;
    if (bytes != pcm_dev->config.period_size)
        return -1;
    unsigned long frames = m_bytes_to_frames(pcm_dev, bytes);
    return audio_device_write(pcm_dev->device, data, frames);
}

unsigned long pcm_read(struct pcm *pcm_dev, void *data, unsigned long bytes)
{
    if (!(pcm_dev->type & PCM_IN))
        return -1;
    if (bytes != pcm_dev->config.period_size)
        return -1;
    unsigned long frames = m_bytes_to_frames(pcm_dev, bytes);
    return audio_device_read(pcm_dev->device, data, frames);
}
