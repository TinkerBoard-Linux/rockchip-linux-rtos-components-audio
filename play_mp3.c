/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "play_mp3.h"

#ifdef AUDIO_DECODER_MP3
typedef struct play_mp3 *play_mp3_handle_t;
static short *pi_pcmbuf = NULL;
MP3PLAYERINFO *g_pMPI = NULL;
extern uint32_t g_BcoreRetval;
extern struct audio_server_data_share g_ABCoreDat;

int play_mp3_init_impl(struct play_decoder *self, play_decoder_cfg_t *cfg)
{
    int ret = 0;

    RK_AUDIO_LOG_D("play_mp3_init_impl in\n");
    play_mp3_handle_t mp3 = (play_mp3_handle_t) audio_calloc(1, sizeof(*mp3));
    if (!mp3)
        return RK_AUDIO_FAILURE;
    mp3->has_post = false;
    mp3->input = cfg->input;
    mp3->output = cfg->output;
    mp3->post = cfg->post;
    mp3->userdata = cfg->userdata;
    self->userdata = (void *) mp3;

    RK_AUDIO_LOG_D("play_decoder type [%s]\n", self->type);
    pi_pcmbuf = (short *)audio_malloc(sizeof(short) * MAX_NCHAN * MAX_NGRAN * MAX_NSAMP);
    if (!pi_pcmbuf)
    {
        RK_AUDIO_LOG_E("pi_pcmbuf malloc fail.\n");
        play_mp3_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }
    g_ABCoreDat.dat[0] = (uint32_t)pi_pcmbuf;
    g_ABCoreDat.type = TYPE_AUDIO_MP3_DEC;

    ret = AudioSendMsg(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN);
    if (ret < 0)
    {
        play_mp3_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }

    g_pMPI = (MP3PLAYERINFO *)g_ABCoreDat.dat[1];
    if (((int32_t)g_pMPI) == MP3_AB_CORE_SHARE_ADDR_INVALID)
    {
        RK_AUDIO_LOG_E("mp3 init Decoder error\n");
        play_mp3_destroy_impl(self);
        return RK_AUDIO_FAILURE;
    }
    else
    {
        RK_AUDIO_LOG_V("mp3 init SUCCESS out\n");
        return RK_AUDIO_SUCCESS;
    }
}

extern uint32_t music_pos, music_frame_offset;
extern int music_saved_argu;
extern uint32_t music_samplerate, seek_samplerate;
extern uint16_t music_bits_per_sample, music_channels, seek_bits_per_sample, seek_channels;
play_decoder_error_t play_mp3_process_impl(struct play_decoder *self)
{
    play_mp3_handle_t mp3 = (play_mp3_handle_t) self->userdata;
    size_t n;
    size_t skip;
    int read_bytes;
    size_t id3_length = 0;
    size_t bytes_left = 0;
    size_t want_read_bytes = 0;
    int sample_rate, channels, bits;
    int ret = 0;
    int vol_cnt = 1;
    bool is_first_frame = true, can_decode = false;
    char *decode_ptr = NULL;
    RK_AUDIO_LOG_D("play_mp3_process\n");

    read_bytes = mp3->input(mp3->userdata, mp3->read_buf, MP3_ID3V2_HEADER_LENGHT);
    music_pos += read_bytes;
    if (read_bytes != MP3_ID3V2_HEADER_LENGHT)
    {
        RK_AUDIO_LOG_E("read buffer failed\n");
        return PLAY_DECODER_INPUT_ERROR;
    }

    n = MP3_ID3V2_HEADER_LENGHT;
    if (!memcmp(mp3->read_buf, "ID3", 3))
    {
        RK_AUDIO_LOG_D("find id3\n");
        id3_length = (mp3->read_buf[6] & 0x7f) << 21 |
                     (mp3->read_buf[7] & 0x7f) << 14 |
                     (mp3->read_buf[8] & 0x7f) << 7 |
                     (mp3->read_buf[9] & 0x7f) ;
        RK_AUDIO_LOG_D("id3_length:%d\n", id3_length);
        if (id3_length > 0)
        {
            while (1)
            {
                n = 0;
                read_bytes = mp3->input(mp3->userdata, mp3->read_buf, sizeof(mp3->read_buf));
                music_pos += read_bytes;
                RK_AUDIO_LOG_D("id3_length:%d,read_bytes:%d\n", id3_length, read_bytes);
                if (read_bytes <= 0)
                {
                    RK_AUDIO_LOG_E("read mp3 input failed\n");
                    return PLAY_DECODER_INPUT_ERROR;
                }
                if (read_bytes >= id3_length)
                {
                    bytes_left = read_bytes - id3_length;
                    decode_ptr = (char *)&mp3->read_buf[id3_length];
                    // memmove(mp3->read_buf,&mp3->read_buf[id3_length],bytes_left);
                    RK_AUDIO_LOG_D("%s,skip id3_length,:%d\n", __func__, id3_length);
                    break;
                }
                else
                {
                    id3_length -= read_bytes;
                }
            }
        }
    }

    want_read_bytes = sizeof(mp3->read_buf) - n - bytes_left;
    while (1)
    {
        if (bytes_left > 0)
        {
            memmove(mp3->read_buf, decode_ptr, bytes_left);
        }
        if (want_read_bytes)
            want_read_bytes = sizeof(mp3->read_buf) - n - bytes_left;
        read_bytes = mp3->input(mp3->userdata, mp3->read_buf + n + bytes_left, want_read_bytes);
        /*
         * RK_AUDIO_LOG_D("mp3 header = %02x, %02x, %02x, %02x, %02x, %02x",
         *         mp3->read_buf[0],
         *         mp3->read_buf[1],
         *         mp3->read_buf[2],
         *         mp3->read_buf[3],
         *         mp3->read_buf[4],
         *         mp3->read_buf[5]);
         */
        music_pos += read_bytes;
        if (read_bytes < want_read_bytes)
        {
            want_read_bytes = 0;
        }

        /* RK_AUDIO_LOG_D("mp3 process read:%x\n",read_bytes); */
        if ((read_bytes == 0) && (bytes_left == 0))
        {
            RK_AUDIO_LOG_V("mp3 read over");
            return RK_AUDIO_SUCCESS;
        }
        else if (read_bytes == -1)
        {
            RK_AUDIO_LOG_E("mp3 read fail");
            //return PLAY_DECODER_INPUT_ERROR;
            return RK_AUDIO_SUCCESS;
        }
        else
        {
            decode_ptr = mp3->read_buf;
            bytes_left = bytes_left + read_bytes + n;
            can_decode = true;
            n = 0;
        }

        g_ABCoreDat.dat[0] = (uint32_t)decode_ptr;
        g_ABCoreDat.dat[1] = bytes_left;
        if (can_decode)
        {
            ret = AudioSendMsg(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE);
            if (ret < 0)
            {
                RK_AUDIO_LOG_E("Decoder fail:%d\n", ret);
                return RK_AUDIO_FAILURE;
            }

            decode_ptr = (char *)g_ABCoreDat.dat[0];
            bytes_left = g_ABCoreDat.dat[1];
            music_frame_offset = music_pos - bytes_left;

            //RK_AUDIO_LOG_D("%d, %x, %x", music_pos - bytes_left, decode_ptr[0], decode_ptr[1]);

            /* fix bug system crash start */
            if ((decode_ptr < mp3->read_buf)
                || (decode_ptr > (mp3->read_buf + MAINBUF_SIZE * MP3_DECODE_FRAME_COUNT))
                || (bytes_left > MAINBUF_SIZE * MP3_DECODE_FRAME_COUNT))
            {
                RK_AUDIO_LOG_E("system_crash_start %x, %x", decode_ptr, bytes_left);
                decode_ptr = mp3->read_buf;
                bytes_left = 0;
            }
            /* fix bug system crash end */

            ret = g_BcoreRetval;
            if (is_first_frame)
            {
                bits        = g_pMPI->mpi_frameinfo.bitsPerSample;
                channels    = g_pMPI->mpi_frameinfo.nChans;
                sample_rate = g_pMPI->mpi_frameinfo.samprate;
                RK_AUDIO_LOG_V("is first frame mp3 samprate = %d Channel = %d bits = %d outlen = %d\n",
                               g_pMPI->mpi_frameinfo.samprate,
                               g_pMPI->mpi_frameinfo.nChans,
                               g_pMPI->mpi_frameinfo.bitsPerSample,
                               g_pMPI->mpi_frameinfo.outputSamps);

                if (ret != 0)
                {
                    RK_AUDIO_LOG_E("decode error, no frame header.\n");
                    continue;
                }

                if ((channels != 1) && (channels != 2))
                {
                    RK_AUDIO_LOG_W("mp3 decode channels don't support, continue read file...\n");
                    continue;
                }

                if (sample_rate < 8000 || sample_rate > 48000)
                {
                    RK_AUDIO_LOG_W("mp3 decode sample rate don't support, continue read file...\n");
                    continue;
                }
                if (bits != 16)
                {
                    RK_AUDIO_LOG_W("mp3 decode sample bit don't support, continue read file...\n");
                    continue;
                }
                mp3->post(mp3->userdata, sample_rate, bits, channels);
                mp3->has_post = true;
                is_first_frame = false;
            }

            if (ret != 0)
                RK_AUDIO_LOG_D("ret = %d,outlen = %d\n", ret, g_pMPI->mpi_frameinfo.outputSamps);
            if ((g_pMPI->mpi_frameinfo.outputSamps > 0) && (ret == 0))
            {
                if (g_pMPI->mpi_frameinfo.samprate != sample_rate)
                {
                    RK_AUDIO_LOG_W("WARNING: Changing Frame Header is Happenned.\n");
                }
                size_t output_bytes = g_pMPI->mpi_frameinfo.outputSamps * 2;
                int write_bytes = mp3->output(mp3->userdata, (char *)pi_pcmbuf, output_bytes);
                if (write_bytes == -1)
                {
                    RK_AUDIO_LOG_E("mp3_decode pcm write failed\n");
                    break;
                }
                g_pMPI->mpi_frameinfo.outputSamps = 0;
                // RK_AUDIO_LOG_D("mp3 output write_bytes:%d\n",write_bytes);
            }
        }
    }
    return RK_AUDIO_SUCCESS;
}

bool play_mp3_get_post_state_impl(struct play_decoder *self)
{
    play_mp3_handle_t mp3 = (play_mp3_handle_t) self->userdata;
    return mp3->has_post;
}

void play_mp3_destroy_impl(struct play_decoder *self)
{
    RK_AUDIO_LOG_D("in\n");
    play_mp3_handle_t mp3 = (play_mp3_handle_t) self->userdata;
    if (mp3)
    {
        RK_AUDIO_LOG_D("free mp3dec buffer.\n");
        audio_free(mp3);
    }
    if (pi_pcmbuf)
    {
        audio_free(pi_pcmbuf);
    }

    AudioSendMsg(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE);
    g_ABCoreDat.type = TYPE_DATA_MAX;

    RK_AUDIO_LOG_D("out\n");
}
#endif
