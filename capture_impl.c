/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include <stdio.h>
#include "AudioConfig.h"
#include "audio_pcm.h"

struct pcm *capture_handle;

int capture_device_open_impl(struct capture_device *self, capture_device_cfg_t *cfg)
{
    struct pcm_config config;

    RK_AUDIO_LOG_D("cfg->frame_size = %d.\n", cfg->frame_size);

    config.channels = cfg->channels ? cfg->channels : 2;
    config.rate = cfg->samplerate ? cfg->samplerate : 16000;
    config.bits = cfg->bits ? cfg->bits : 16;
    config.period_size = cfg->frame_size;
    config.period_count = 4;

    capture_handle = pcm_open(cfg->device_name, AUDIO_FLAG_RDONLY);
    pcm_set_config(capture_handle, config);

    RK_AUDIO_LOG_D("Open Playback success.\n");

    return RK_AUDIO_SUCCESS;
}

int capture_device_start_impl(struct capture_device *self)
{
    int cap_ret = 0;

    RK_AUDIO_LOG_D("\n");
    cap_ret = pcm_start(capture_handle);
    RK_AUDIO_LOG_D("pcm start return %d.\n", cap_ret);
    if (cap_ret != 0)
        return RK_AUDIO_FAILURE;

    return RK_AUDIO_SUCCESS;
}

int capture_device_read_impl(struct capture_device *self, const char *data, size_t data_len)
{
    int read_err = 0;

    read_err = pcm_read(capture_handle, (void *)data, data_len);
    if (read_err < 0)
    {
        RK_AUDIO_LOG_E("read_frames: %d\n", read_err);
    }
    if (read_err == -EINVAL)
    {

        pcm_prepare(capture_handle);
    }

    return data_len;

#ifdef USE_ALSA
    int write_frames = snd_pcm_writei(capture_handle, data, period_size);
    if (write_frames < 0)
    {
        RK_AUDIO_LOG_E("write_frames: %d, %s", write_frames, snd_strerror(write_frames));
    }
    if (write_frames == -EPIPE)
    {
        snd_pcm_prepare(capture_handle);
    }
    return data_len;
#endif
}

int capture_device_stop_impl(struct capture_device *self)
{
    int stop_err = 0;

    RK_AUDIO_LOG_D("\n");

    stop_err = pcm_stop(capture_handle);
    RK_AUDIO_LOG_D("stop_err = %d\n", stop_err);
    if (stop_err != 0)
        return RK_AUDIO_FAILURE;

    return RK_AUDIO_SUCCESS;
}

int capture_device_abort_impl(struct capture_device *self)
{
    RK_AUDIO_LOG_D("\n");
    return RK_AUDIO_SUCCESS;
}

int capture_device_close_impl(struct capture_device *self)
{
    RK_AUDIO_LOG_D("\n");
    if (capture_handle)
    {
        pcm_close(capture_handle);
    }
    capture_handle = NULL;

    return RK_AUDIO_SUCCESS;
}
