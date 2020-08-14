/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef PLAY_AMR_H
#define PLAY_AMR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AudioConfig.h"

#undef DEFAULT_AMR_DECODER

#ifdef AUDIO_DECODER_AMR

int play_amr_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg);
play_decoder_error_t play_amr_process_impl(struct play_decoder *self);
bool play_amr_get_post_state_impl(struct play_decoder *self);
void play_amr_destroy_impl(struct play_decoder *self);

#ifdef  CONFIG_FWANALYSIS_SEGMENT
#define DEFAULT_AMR_DECODER { \
    .type = AMR_DECODER_TYPE, \
    .segment = SEGMENT_ID_PLAY_AMR, \
    .init = play_amr_init_impl, \
    .process = play_amr_process_impl, \
    .get_post_state = play_amr_get_post_state_impl, \
    .destroy = play_amr_destroy_impl, \
}
#else
#define DEFAULT_AMR_DECODER { \
    .type = AMR_DECODER_TYPE, \
    .init = play_amr_init_impl, \
    .process = play_amr_process_impl, \
    .get_post_state = play_amr_get_post_state_impl, \
    .destroy = play_amr_destroy_impl, \
}
#endif

#else

#define DEFAULT_AMR_DECODER { \
        .type = "null", \
        .init = NULL, \
        .process = NULL, \
        .get_post_state = NULL, \
        .destroy = NULL, \
    }
#endif

#ifdef __cplusplus
}
#endif

#endif
