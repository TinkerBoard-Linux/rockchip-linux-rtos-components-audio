/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

extern struct wav_header m_wav_header;
extern int g_wav_encode;
int file_writer_init_impl(struct record_writer *self,
                          record_writer_cfg_t *cfg)
{
    int fd = check_native_audio_type(cfg->target, &cfg->type);
    if (!fd)
    {
        RK_AUDIO_LOG_E("[%s]open native file error, file: %s", cfg->tag, cfg->target);
        return RK_AUDIO_FAILURE;
    }

    cfg->frame_size = 4096;
    self->userdata = (void *)fd;
    RK_AUDIO_LOG_D("[%s]open native file ok, file: %s, audio type:%s\n",
                   cfg->tag, cfg->target, cfg->type);

    return RK_AUDIO_SUCCESS;
}

int file_writer_write_impl(struct record_writer *self, char *data,
                           size_t data_len)
{
    int fd = (int)self->userdata;

    return audio_fwrite(data, 1, data_len, fd);
}

void file_writer_destroy_impl(struct record_writer *self)
{
    if (!self)
        return;

    int fd = (int)self->userdata;

    if (g_wav_encode)
    {
        audio_fseek(fd, 0, SEEK_SET);
        audio_fwrite((char *)&m_wav_header, 1, sizeof(m_wav_header), fd);
    }
    audio_fclose(fd);
}
