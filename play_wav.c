/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

struct play_wav
{
    bool has_post;
    char read_buf[1024];
    decode_input_t input;
    decode_output_t output;
    decode_post_t post;
    void *userdata;
};
typedef struct play_wav *play_wav_handle_t;

int play_wav_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg)
{
    play_wav_handle_t wav = (play_wav_handle_t)audio_calloc(1, sizeof(*wav));
    if (!wav)
        return RK_AUDIO_FAILURE;
    wav->input = cfg->input;
    wav->output = cfg->output;
    wav->post = cfg->post;
    wav->userdata = cfg->userdata;
    self->userdata = (void *)wav;
    return RK_AUDIO_SUCCESS;
}
extern int music_saved_argu;
extern uint32_t music_samplerate, seek_samplerate;
extern uint16_t music_bits_per_sample, music_channels, seek_bits_per_sample, seek_channels;
play_decoder_error_t play_wav_process_impl(struct play_decoder *self)
{
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    int read_bytes;
    int write_bytes;
    struct wav_header header;
    if (!music_saved_argu)
    {
        read_bytes = wav->input(wav->userdata, (char *)&header,
                                sizeof(header));
        if (read_bytes != sizeof(header))
            return PLAY_DECODER_DECODE_ERROR;
        if (memcmp(header.chunk_id, "RIFF", 4) != 0 ||
            memcmp(header.format, "WAVE", 4) != 0 ||
            memcmp(header.subchunk1_id, "fmt ", 4) != 0 ||
            memcmp(header.subchunk2_id, "data", 4) != 0)
        {
            return PLAY_DECODER_DECODE_ERROR;
        }
        RK_AUDIO_LOG_V("wav decoder sample rate: %d, channels: %d, bits: %d",
                       header.samplerate, header.num_channels, header.bits_per_sample);
        music_samplerate = header.samplerate;
        music_bits_per_sample = header.bits_per_sample;
        music_channels = header.num_channels;
        wav->post(wav->userdata, header.samplerate, header.bits_per_sample,
                  header.num_channels);
    }
    else
    {
        wav->post(wav->userdata, seek_samplerate, seek_bits_per_sample, seek_channels);
        music_saved_argu = false;
    }
    wav->has_post = true;
    while (1)
    {
        read_bytes = wav->input(wav->userdata, wav->read_buf,
                                sizeof(wav->read_buf) / 2);
        if (read_bytes == 0)
        {
            return PLAY_DECODER_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            return PLAY_DECODER_SUCCESS;
        }
        write_bytes = wav->output(wav->userdata, wav->read_buf,
                                  read_bytes);
        if (write_bytes == -1)
            return PLAY_DECODER_OUTPUT_ERROR;
    }
    return PLAY_DECODER_SUCCESS;
}
bool play_wav_get_post_state_impl(struct play_decoder *self)
{
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    return wav->has_post;
}
void play_wav_destroy_impl(struct play_decoder *self)
{
    if (!self)
        return;
    play_wav_handle_t wav = (play_wav_handle_t)self->userdata;
    audio_free(wav);
}
