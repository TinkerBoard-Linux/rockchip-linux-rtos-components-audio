/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */
#include <stdio.h>
#include "AudioConfig.h"
#include "audio_pcm.h"

static struct pcm *playback_handle = NULL;

#define NO_BUFFER_MODE 1
int playback_device_open_impl(struct playback_device *self, playback_device_cfg_t *cfg)
{
    struct pcm_config config;

    RK_AUDIO_LOG_D("cfg->frame_size = %d.", cfg->frame_size);

    if (playback_handle)
        goto OPEN_SUCCESS;

    if (NO_BUFFER_MODE)
        config.channels = 2;
    else
        config.channels = cfg->channels ? cfg->channels : 2;

    config.rate = cfg->samplerate ? cfg->samplerate : 16000;
    config.bits = cfg->bits ? cfg->bits : 16;
    config.period_size = cfg->frame_size;
    config.period_count = 3;

    RK_AUDIO_LOG_V("rate:%d bits:%d ch:%d", config.rate, config.bits, config.channels);
    playback_handle = pcm_open(6, AUDIO_FLAG_WRONLY);
    if (!playback_handle)
        return RK_AUDIO_FAILURE;
    if (pcm_set_config(playback_handle, config))
    {
        pcm_close(playback_handle);
        playback_handle = NULL;
        return RK_AUDIO_FAILURE;
    }
OPEN_SUCCESS:
    RK_AUDIO_LOG_D("Open Playback success.");

    return RK_AUDIO_SUCCESS;
}

int playback_device_start_impl(struct playback_device *self)
{
    int err = pcm_prepare(playback_handle);
    err = pcm_start(playback_handle);
    RK_AUDIO_LOG_D("playback_device_start_impl err= %d", err);

    return err;
}

int playback_device_write_impl(struct playback_device *self, const char *data, size_t data_len)
{
    int write_err ;
    write_err = pcm_write(playback_handle, (void *)data, data_len);
    if (write_err < 0)
    {
        RK_AUDIO_LOG_W("write_frames: %d", write_err);
    }
    if (write_err == -EINVAL)
    {
        pcm_prepare(playback_handle);
    }

    return data_len;
}

int playback_device_stop_impl(struct playback_device *self)
{
    int stop_err = 0;

    RK_AUDIO_LOG_V("\n");

    stop_err = pcm_stop(playback_handle);
    RK_AUDIO_LOG_D("stop_err = %d", stop_err);
    if (stop_err != 0)
        return RK_AUDIO_FAILURE;

    return RK_AUDIO_SUCCESS;
}

int playback_device_abort_impl(struct playback_device *self)
{
    RK_AUDIO_LOG_D("\n");

    return RK_AUDIO_SUCCESS;
}

void playback_device_close_impl(struct playback_device *self)
{
    RK_AUDIO_LOG_D("\n");
    if (playback_handle)
    {
        pcm_close(playback_handle);
    }
    playback_handle = NULL;
}
