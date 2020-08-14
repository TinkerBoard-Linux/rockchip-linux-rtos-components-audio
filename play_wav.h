/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef PLAY_WAV_H
#define PLAY_WAV_H
#ifdef __cplusplus
extern "C" {
#endif

#include "AudioConfig.h"

__attribute((visibility("default"))) int play_wav_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg);
__attribute((visibility("default"))) play_decoder_error_t play_wav_process_impl(struct play_decoder *self);
__attribute((visibility("default"))) bool play_wav_get_post_state_impl(struct play_decoder *self);
__attribute((visibility("default"))) void play_wav_destroy_impl(struct play_decoder *self);

#ifndef DEFAULT_WAV_DECODER
#ifdef  CONFIG_FWANALYSIS_SEGMENT
#define DEFAULT_WAV_DECODER { \
        .type = "wav", \
        .segment = SEGMENT_ID_PLAY_WAV, \
        .init = play_wav_init_impl, \
        .process = play_wav_process_impl, \
        .get_post_state = play_wav_get_post_state_impl, \
        .destroy = play_wav_destroy_impl, \
    }
#else
#define DEFAULT_WAV_DECODER { \
        .type = "wav", \
        .init = play_wav_init_impl, \
        .process = play_wav_process_impl, \
        .get_post_state = play_wav_get_post_state_impl, \
        .destroy = play_wav_destroy_impl, \
    }
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif
