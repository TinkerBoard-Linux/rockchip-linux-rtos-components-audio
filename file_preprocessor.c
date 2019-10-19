/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

/* os_log_create_module(file_preprocessor, OS_LOG_LEVEL_INFO); */
#define FILE_PREPROCESSOR_FRAME_SIZE 1024

extern int music_need_seek;
extern uint32_t music_seek;
int file_preprocessor_init_impl(struct play_preprocessor *self,
                                play_preprocessor_cfg_t *cfg)
{
    int fd = 0;
    fd = check_native_audio_type(cfg->target, &cfg->type);
    if (!fd)
    {
        RK_AUDIO_LOG_E("[%s] open native file error, file: %s",
                       cfg->tag,
                       cfg->target);
        return RK_AUDIO_FAILURE;
    }

    RK_AUDIO_LOG_D("[%s] open native file, file: %s", cfg->tag, cfg->target);
    cfg->frame_size = FILE_PREPROCESSOR_FRAME_SIZE;
    self->userdata = (void *)fd;
    RK_AUDIO_LOG_V("[%s] open native file ok, file: %s, audio type:%s",
                   cfg->tag,
                   cfg->target,
                   cfg->type);
    if (music_need_seek)
    {
        audio_fseek(fd, music_seek, SEEK_SET);
        music_need_seek = false;
    }
    return RK_AUDIO_SUCCESS;
}

int file_preprocessor_read_impl(struct play_preprocessor *self, char *data,
                                size_t data_len)
{
    int fd = (int)self->userdata;
    return audio_fread(data, 1, data_len, fd);

}
void file_preprocessor_destroy_impl(struct play_preprocessor *self)
{
    if (!self) return;
    int fd = (int)self->userdata;
    audio_fclose(fd);
}
