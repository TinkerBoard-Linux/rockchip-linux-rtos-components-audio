/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct record_wav
{
    bool has_post;
    char read_buf[1024];
    encode_input_t input;
    encode_output_t output;
    encode_post_t post;
    void *userdata;
};
typedef struct record_wav *record_wav_handle_t;

static long total_byte;
struct wav_header m_wav_header;

int record_wav_init(struct record_encoder *self, record_encoder_cfg_t *cfg)
{
    record_wav_handle_t wav = (record_wav_handle_t) audio_calloc(1, sizeof(*wav));
    struct audio_config wav_cfg = *(struct audio_config *) self->userdata;
    if (!wav)
        return -1;
    wav->has_post = false;
    wav->input = cfg->input;
    wav->output = cfg->output;
    wav->post = cfg->post;
    wav->userdata = cfg->userdata;
    wav_header_init(&m_wav_header, wav_cfg.sample_rate, wav_cfg.bits, wav_cfg.channels);
    RK_AUDIO_LOG_D("cfg:r %d b %d c %d\n", wav_cfg.sample_rate, wav_cfg.bits, wav_cfg.channels);
    self->userdata = (void *)wav;
    total_byte = 0;

    return 0;
}

record_encoder_error_t record_wav_process(struct record_encoder *self)
{
    record_wav_handle_t wav = (record_wav_handle_t) self->userdata;
    RK_AUDIO_LOG_D("\n");
    wav->output(wav->userdata, (char *)&m_wav_header, sizeof(m_wav_header));
    while (1)
    {
        int read_bytes = wav->input(wav->userdata, wav->read_buf, sizeof(wav->read_buf));
        //RK_AUDIO_LOG_D("record_wav_process:%d\n",read_bytes);
        if (read_bytes == 0)
        {
            RK_AUDIO_LOG_W("wav->input finish \n");
            wav_header_complete(&m_wav_header, total_byte);
            wav->output(wav->userdata, (char *)&m_wav_header, sizeof(m_wav_header));
            return RECORD_ENCODER_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            RK_AUDIO_LOG_E("wav->input failed \n");
            wav_header_complete(&m_wav_header, total_byte);
            wav->output(wav->userdata, (char *)&m_wav_header, sizeof(m_wav_header));
            return RECORD_ENCODER_INPUT_ERROR;
        }
        int write_bytes = wav->output(wav->userdata, wav->read_buf, read_bytes);
        total_byte += write_bytes;
        if (write_bytes == -1)
        {
            RK_AUDIO_LOG_E("wav->output failed \n");
            return RECORD_ENCODER_OUTPUT_ERROR;
        }
    }

    return RECORD_ENCODER_SUCCESS;
}

bool record_wav_get_post_state(struct record_encoder *self)
{
    record_wav_handle_t wav = (record_wav_handle_t) self->userdata;

    return wav->has_post;
}

void record_wav_destroy(struct record_encoder *self)
{
    record_wav_handle_t wav = (record_wav_handle_t) self->userdata;
    if (wav)
    {
        audio_free(wav);
    }
}
