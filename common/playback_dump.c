/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */
#include <stdio.h>
#include "AudioConfig.h"
#include "audio_pcm.h"

static int playback_handle = 0;
static int fakesink = 0;
int playback_dump_open_impl(struct playback_device *self, playback_device_cfg_t *cfg)
{
    if (strncmp((char *)cfg->card_name, "fakesink", 8) == 0)
    {
        fakesink = 1;
        return RK_AUDIO_SUCCESS;
    }
    fakesink = 0;
    if (!playback_handle)
        playback_handle = audio_fopen((char *)cfg->card_name, "wb+");
    if (!playback_handle)
        return RK_AUDIO_FAILURE;

    RK_AUDIO_LOG_V("Open %s success.", cfg->card_name);

    return RK_AUDIO_SUCCESS;
}

int playback_dump_start_impl(struct playback_device *self)
{
    if (!playback_handle && !fakesink)
    {
        RK_AUDIO_LOG_E("Should call open first");
        return RK_AUDIO_FAILURE;
    }

    return RK_AUDIO_SUCCESS;
}

int playback_dump_write_impl(struct playback_device *self, const char *data, size_t data_len)
{
    int write_err;

    if (fakesink)
        return data_len;

    if (!playback_handle)
    {
        RK_AUDIO_LOG_E("Should call open first");
        return RK_AUDIO_FAILURE;
    }

    write_err = audio_fwrite((void *)data, 1, data_len, playback_handle);
    if (write_err < 0)
    {
        RK_AUDIO_LOG_W("write_frames: %d", write_err);
    }

    return data_len;
}

int playback_dump_stop_impl(struct playback_device *self)
{
    return RK_AUDIO_SUCCESS;
}

int playback_dump_abort_impl(struct playback_device *self)
{
    return RK_AUDIO_SUCCESS;
}

void playback_dump_close_impl(struct playback_device *self)
{
    RK_AUDIO_LOG_V("close");

    if (playback_handle && !fakesink)
    {
        audio_fclose(playback_handle);
    }
    playback_handle = 0;
}

