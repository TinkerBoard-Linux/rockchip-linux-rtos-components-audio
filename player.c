/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

#include "dsp_include/Src.h"

//****************************************************************************
//
// The actual polyphase filters.  These filters can have any number of
// polyphases but must have NUMTAPS taps.
//
//****************************************************************************
#include "dsp_include/filt2000.h"
#include "dsp_include/filt_44100to16000.h"
#include "dsp_include/filt_32k_16k.h"
#include "dsp_include/filt_22k_to_16k.h"
#include "dsp_include/filt_24k_to_16k.h"
#include "dsp_include/filt_48kto16k.h"
#include "dsp_include/filt_11k_to_16k.h"

#define MAX_DECODER_NUM (4)
#define MP3_MONO_MAX_LEN    (2304)

/*--define for seek--*/
uint32_t music_pos = 0, music_seek = 0, music_frame_offset = 0;
char *music_name = NULL;
int music_need_seek = 0, music_saved_argu = 0;
uint32_t music_samplerate, seek_samplerate;
uint16_t music_bits_per_sample, music_channels, seek_bits_per_sample, seek_channels;
uint32_t total_pcm_cnt;
enum
{
    FILE_TYPE_PCM = 0,
    FILE_TYPE_WAV,
    FILE_TYPE_MP3,
    FILE_TYPE_AMR
} music_file_type;

int g_player_freq = 0;
play_decoder_t *g_default_decoder;
play_decoder_cfg_t *decoder_cfg = NULL;
play_preprocessor_cfg_t *processor_cfg = NULL;

struct player
{
    struct audio_player_queue *preprocess_queue;
    struct audio_player_queue *decode_queue;
    struct audio_player_queue *play_queue;

    struct audio_player_stream *preprocess_stream;
    struct audio_player_stream *decode_stream;

    audio_player_mutex_handle state_lock;
    audio_player_semaphore_handle pause_sem;

    audio_player_thread_handle preprocess_task;
    audio_player_thread_handle decode_task;
    audio_player_thread_handle play_task;

    //os_event_group_handle_t event;

    player_state_t state;

    const char *tag;

    player_listen_cb listen;
    void *userdata;

    const char *name;
    playback_device_t device;

    int samplerate;
    int bits;
    int channels;

    int resample_rate;
};

void player_freq_init(void)
{
    switch (g_player_freq)
    {
    case PLAY_FREQ_LOCALPLAY:
        // rkpm_add_freq_scene(FREQ_LOCALPLAY);
        break;
    case PLAY_FREQ_NETPLAY:
        // rkpm_add_freq_scene(FREQ_NETPLAY);
        break;
    default:
        RK_AUDIO_LOG_E("\n==player_freq_init: error!!!==\n");
        while (1);
        break;
    }
}

void player_freq_deinit(void)
{
    switch (g_player_freq)
    {
    case PLAY_FREQ_LOCALPLAY:
        // rkpm_remove_freq_scene(FREQ_LOCALPLAY);
        break;
    case PLAY_FREQ_NETPLAY:
        // rkpm_remove_freq_scene(FREQ_NETPLAY);
        break;
    default:
        break;
    }
}

int player_init()
{
    if (!g_default_decoder)
        g_default_decoder = (play_decoder_t *)audio_calloc(MAX_DECODER_NUM, sizeof(*g_default_decoder));
    play_decoder_t pcm_decoder = DEFAULT_PCM_DECODER;
    play_decoder_t wav_decoder = DEFAULT_WAV_DECODER;
    play_decoder_t mp3_decoder = DEFAULT_MP3_DECODER;
    play_decoder_t amr_decoder = DEFAULT_AMR_DECODER;
    g_default_decoder[0] = pcm_decoder;
    g_default_decoder[1] = wav_decoder;
    g_default_decoder[2] = mp3_decoder;
    g_default_decoder[3] = amr_decoder;
    return 0;
}

int player_register_decoder(const char *type, play_decoder_t *decoder)
{
    return 0;
}

void *preprocess_run(void *data)
{
    player_handle_t player = (player_handle_t) data;
    media_sdk_msg_t msg;
    media_sdk_msg_t decode_msg;
    play_preprocessor_t preprocessor;
    int res;
    char *read_buf;
    size_t read_size = 0;
    size_t frame_size = 0;
    if (!processor_cfg)
    {
        processor_cfg = (play_preprocessor_cfg_t *)audio_malloc(sizeof(*processor_cfg));
        RK_AUDIO_LOG_D("malloc processor_cfg...");
    }

    //processor_cfg->type = (char *)audio_malloc(sizeof(char) * 10);
    //memset(processor_cfg->type,0,10);
__PREPROCESS_ERROR:
    while (1)
    {
        if (audio_queue_receive(player->preprocess_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_E("can't get preprocess msg");
            //goto  __PREPROCESS_OUT;
            goto  __PREPROCESS_ERROR;
        }

        preprocessor = msg.player.preprocessor;
        RK_AUDIO_LOG_D("preprocessor.type:%s", preprocessor.type);
        if (!preprocessor.type)
        {
            preprocessor.type = "unknown";
            RK_AUDIO_LOG_E("can't get preprocessor.type");
            //goto __PREPROCESS_OUT;
            goto  __PREPROCESS_ERROR;
        }

        //RK_AUDIO_LOG_D("msg.player.target:0x%x,%s.",&msg.player.target,msg.player.target);
        //RK_AUDIO_LOG_D("msg.player.player:0x%x,%s",&processor_cfg->type,processor_cfg->type);
        processor_cfg->target = msg.player.target;
        processor_cfg->tag = player->tag;

        res = preprocessor.init(&preprocessor, processor_cfg);
        if (msg.player.need_free && msg.player.target != NULL)
        {
            audio_free(msg.player.target);
            msg.player.target = NULL;
        }

        if (res)
        {
            RK_AUDIO_LOG_E("init fail res:%d", res);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_IDLE, player->userdata);
            }
            //goto __PREPROCESS_OUT;
            goto  __PREPROCESS_ERROR;
        }

        frame_size = processor_cfg->frame_size;
        read_buf = audio_malloc(frame_size);
        if (!read_buf)
        {
            RK_AUDIO_LOG_E("can't audio_malloc buffer");
            //goto __PREPROCESS_OUT;
            goto  __PREPROCESS_ERROR;
        }

        decode_msg.type = msg.type;
        decode_msg.player.mode = msg.player.mode;
        decode_msg.player.target = msg.player.target;
        decode_msg.player.preprocessor = msg.player.preprocessor;
        decode_msg.player.need_free = msg.player.need_free;
        decode_msg.player.end_session = msg.player.end_session;
        decode_msg.player.priv_data = processor_cfg->type;

        audio_stream_start(player->preprocess_stream);
        RK_AUDIO_LOG_D("send msg to decode");
        audio_queue_send(player->decode_queue, &decode_msg);
        int timeout = 0;
        do
        {
retry:
            read_size = preprocessor.read(&preprocessor, read_buf, frame_size);
            //RK_AUDIO_LOG_D("read read_size:%d", read_size);
            if (read_size == -101)
            {
                if ((player->state == PLAYER_STATE_IDLE) || (player->state == PLAYER_STATE_ERROR))
                {
                    audio_stream_finish(player->preprocess_stream);
                    break;
                }
                timeout++;
                if (timeout > 20)
                {
                    //more than 10s exit
                    audio_stream_finish(player->preprocess_stream);
                    break;
                }
                RK_AUDIO_LOG_D("player state = %d", player->state);
                goto retry;
            }
            else if (read_size <= 0)
            {
                audio_stream_finish(player->preprocess_stream);
                break;
            }
            timeout = 0;

        }
        while (audio_stream_write(player->preprocess_stream, read_buf, read_size) != -1);
        RK_AUDIO_LOG_D("free preprocess.");
        audio_free(read_buf);
        preprocessor.destroy(&preprocessor);
        RK_AUDIO_LOG_D("free preprocess out.");
        RK_AUDIO_LOG_V("out");
    }
#if 0
__PREPROCESS_OUT:
    preprocessor.destroy(&preprocessor);
    RK_AUDIO_LOG_D("out");
    return NULL;
#if 0
    while (1)
    {
        RK_AUDIO_LOG_D("wait delete...");
        rkos_sleep(10000);
    }
#endif
#endif
}

static SRCState *g_pSRC = NULL;
static int g_is_need_resample = 0;
static char *g_resample_buffer;
#define MAX_RESAMPLE_BUFFER_SIZE (7168)

int decoder_input(void *userdata, char *data, size_t data_len)
{
    player_handle_t player = (player_handle_t) userdata;
    return audio_stream_read(player->preprocess_stream, data, data_len);
}

void player_audio_resample_init(int samplerate, int resample_rate)
{
    g_resample_buffer = audio_malloc(2 * MAX_RESAMPLE_BUFFER_SIZE * sizeof(char));
    g_pSRC = (SRCState *)audio_malloc(sizeof(SRCState));
    if (g_pSRC != NULL)
    {
        SRCInit(g_pSRC, samplerate, resample_rate);
    }
}

static short rs_input_buffer[51 * 4 + 128];
int player_audio_resample(player_handle_t player, char *data, size_t data_len)
{
    int resample_out_len = 0;
    if ((g_resample_buffer == NULL) || (g_pSRC == NULL))
    {
        return 0;
    }
    //OS_LOG_D(player,"player_audio_resample, data_le:%d\n",data_len);
    for (int i = 0; i < data_len / 128 / 2; i++)
    {
        memset((char *)rs_input_buffer, 0, 2 * 128);
        memcpy((char *)&rs_input_buffer[51 * 2], &data[i * 128 * 2], 128 * 2);
        resample_out_len += SRCFilter(g_pSRC, (short *)&rs_input_buffer[51 * 2], (short *)&g_resample_buffer[resample_out_len * 2], 128);
    }
    //OS_LOG_D(player,"player_audio_resample, resample_out_len:%d\n",resample_out_len);
    resample_out_len *= 2;
    int ret = audio_stream_write(player->decode_stream, g_resample_buffer, resample_out_len);
    return ret;
}

int decoder_output(void *userdata, char *data, size_t data_len)
{
    player_handle_t player = (player_handle_t) userdata;
    int i = 0;
    int j = 0;
    int frame_bytes = player->bits / 8;
    int isMono = (player->channels == 1) ? 1 : 0;
    /* if MONO, copy left channel data stream to right channels. */
    if (isMono && (data_len <= MP3_MONO_MAX_LEN))
    {
        j = data_len - frame_bytes;
        data_len *= 2;
        for (i = data_len - 2 * frame_bytes; i >= 0; i = i - frame_bytes * 2)
        {
            memcpy(data + i, data + j, frame_bytes);
            memcpy(data + i + frame_bytes, data + j, frame_bytes);
            j -= frame_bytes;
        }
    }
    if (g_is_need_resample)
    {
        int ret = player_audio_resample(player, data, data_len);
        return ret;
    }
    return audio_stream_write(player->decode_stream, data, data_len);

}

void player_audio_resample_deinit(void)
{
    RK_AUDIO_LOG_D("player_audio_resample_deinit\n");
    if (g_resample_buffer)
        audio_free(g_resample_buffer);
    if (g_pSRC)
    {
        audio_free(g_pSRC);
    }
}

uint32_t player_get_target_and_seek(char *file_name)
{
    int frame_align;

    if (music_name && file_name)
    {
        memcpy(file_name, music_name, strlen(music_name) + 1);
    }

    switch (music_file_type)
    {
    case FILE_TYPE_PCM://pcm
    case FILE_TYPE_WAV://wav
    case FILE_TYPE_MP3://mp3
        music_seek = music_frame_offset;
        //music_seek = 0;
        break;
    case FILE_TYPE_AMR://amr
        music_seek = 0;
        break;
    default:
        return 0;
    }
    seek_samplerate = music_samplerate;
    seek_bits_per_sample = music_bits_per_sample;
    seek_channels = music_channels;
    return music_seek;
}

void player_set_seek(long offset)
{
    music_seek = offset;
    music_saved_argu = false;
    music_need_seek = true;
}

int decoder_post(void *userdata, int samplerate, int bits, int channels)
{
    player_handle_t player = (player_handle_t)userdata;
    media_sdk_msg_t msg;
    player->samplerate = samplerate;
    player->bits = bits;
    player->channels = channels;

    if (player->resample_rate && samplerate != player->resample_rate)
    {
        RK_AUDIO_LOG_V("%d != %d, need resample", samplerate, player->resample_rate);
        player_audio_resample_init(samplerate, player->resample_rate);
        memset((char *)rs_input_buffer, 0, 2 * (51 * 4 + 128));
        player->samplerate = player->resample_rate;
        g_is_need_resample = 1;
    }
    else
    {
        g_is_need_resample = 0;
    }

    msg.type = CMD_PLAYER_PLAY;
    msg.player.mode = PLAY_MODE_PROMPT;
    msg.player.target = NULL;
    msg.player.need_free = false;
    msg.player.end_session = false;
    audio_queue_send(player->play_queue, &msg);

    return RK_AUDIO_SUCCESS;
}

void *decoder_run(void *data)
{
    player_handle_t player = (player_handle_t) data;
    //play_decoder_cfg_t* decoder_cfg = (play_decoder_cfg_t*)audio_calloc(1,sizeof(*decoder_cfg));
    play_decoder_t decoder;
    bool is_found_decoder = false;
    int i = 0;
    play_decoder_error_t decode_res;
    media_sdk_msg_t decode_msg;
    if (!decoder_cfg)
    {
        decoder_cfg = (play_decoder_cfg_t *)audio_calloc(1, sizeof(*decoder_cfg));
        decoder_cfg->input = decoder_input;
        decoder_cfg->output = decoder_output;
        decoder_cfg->post = decoder_post;
        decoder_cfg->userdata = player;
    }

    while (1)
    {
        if (audio_queue_receive(player->decode_queue, &decode_msg) == -1)
        {
            RK_AUDIO_LOG_E("can't get msg");
            return NULL;
        }
        for (i = 0; i < MAX_DECODER_NUM; i++)
        {
            if (!strcmp(decode_msg.player.priv_data, g_default_decoder[i].type))
            {
                decoder = g_default_decoder[i];
                music_file_type = i;
                is_found_decoder = true;
                break;
            }
        }
        if (!is_found_decoder)
        {
            RK_AUDIO_LOG_E("can't found decoder");
            audio_stream_stop(player->preprocess_stream);
            audio_mutex_lock(player->state_lock);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
                player->listen(player, PLAY_INFO_IDLE, player->userdata);
            audio_mutex_unlock(player->state_lock);
        }
        else
        {
            RK_AUDIO_LOG_D("decode init.");
#ifdef CONFIG_FWANALYSIS_SEGMENT
            FW_LoadSegment(decoder.segment, SEGMENT_OVERLAY_ALL);
#endif
            if (decoder.init(&decoder, decoder_cfg))
            {
                RK_AUDIO_LOG_E("decoder init fail");
                audio_stream_stop(player->preprocess_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                    player->listen(player, PLAY_INFO_IDLE, player->userdata);
                audio_mutex_unlock(player->state_lock);
                continue;
            }
            audio_stream_start(player->decode_stream);
            decode_res = decoder.process(&decoder);
            RK_AUDIO_LOG_V("decode res %#x", decode_res);
            switch (decode_res)
            {
#if 0
            case PLAY_DECODER_INPUT_ERROR:
                RK_AUDIO_LOG_E("PLAY_DECODER_INPUT_ERROR");
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_IDLE, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
                break;
            case PLAY_DECODER_OUTPUT_ERROR:
                RK_AUDIO_LOG_E("PLAY_DECODER_OUTPUT_ERROR");
                audio_stream_stop(player->preprocess_stream);
                if (!decoder.get_post_state(&decoder))
                {
                    audio_mutex_lock(player->state_lock);
                    player->state = PLAYER_STATE_IDLE;
                    if (player->listen)
                    {
                        player->listen(player, PLAY_INFO_DECODE, player->userdata);
                    }
                    audio_mutex_unlock(player->state_lock);
                    goto _DESTROY_DECODER;
                }
                goto _DESTROY_DECODER;
            case PLAY_DECODER_DECODE_ERROR:
                RK_AUDIO_LOG_E("PLAY_DECODER_DECODE_ERROR");
                audio_stream_stop(player->preprocess_stream);
                audio_stream_stop(player->decode_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_IDLE, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
                break;
#endif

            default:
                RK_AUDIO_LOG_D("audio_stream_finish");
#ifdef CONFIG_FWANALYSIS_SEGMENT
                FW_RemoveSegment(decoder.segment);
#endif
                audio_stream_stop(player->preprocess_stream);
                audio_stream_finish(player->decode_stream);
                if (!decoder.get_post_state(&decoder))
                {
                    audio_mutex_lock(player->state_lock);
                    player->state = PLAYER_STATE_ERROR;
                    if (player->listen)
                    {
                        player->listen(player, PLAY_INFO_IDLE, player->userdata);
                    }
                    audio_mutex_unlock(player->state_lock);
                }
                // break;
            }
#if 0
            if (!decoder.get_post_state(&decoder))
            {
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_IDLE;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_DECODE, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
            }
#endif
_DESTROY_DECODER:
            RK_AUDIO_LOG_D("decoder process return value:%d\n", decode_res);
            decoder.destroy(&decoder);
            if (g_is_need_resample)
            {
                g_is_need_resample = 0;
                player_audio_resample_deinit();
            }
        }
    }
}

void *playback_run(void *data)
{
    player_handle_t player = (player_handle_t) data;
    playback_device_t device = player->device;
    playback_device_cfg_t device_cfg;
    media_sdk_msg_t msg;
    char *read_buf = NULL;
    int read_size;
    size_t frame_size;
    static size_t oddframe = 0;
    int pcm_start_ret = 0;
    while (1)
    {
        if (audio_queue_receive(player->play_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_E("receive data failed");
            return NULL;
        }
        device_cfg.samplerate = player->samplerate;
        device_cfg.bits = player->bits;
        device_cfg.channels = player->channels;
        device_cfg.card_name = player->name;
        device_cfg.frame_size = 2048;
        RK_AUDIO_LOG_V("start");
        if (device.open(&device, &device_cfg))
        {
            audio_stream_stop(player->preprocess_stream);
            audio_stream_stop(player->decode_stream);
            audio_mutex_lock(player->state_lock);
            player->state = PLAYER_STATE_ERROR;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_IDLE, player->userdata);
            }
            audio_mutex_unlock(player->state_lock);
            RK_AUDIO_LOG_E("device open fail");
            continue;
        }
        else
        {
            frame_size = device_cfg.frame_size;
            pcm_start_ret = device.start(&device);
            if (pcm_start_ret != 0)
            {
                RK_AUDIO_LOG_E("pcm device start fail.");
                audio_stream_stop(player->preprocess_stream);
                audio_stream_stop(player->decode_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_IDLE, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
                continue;
            }
            player->state = PLAYER_STATE_RUNNING;
            read_buf = audio_malloc(frame_size * 2);
            if (!read_buf)
            {
                RK_AUDIO_LOG_E("create read buf fail");
                audio_stream_stop(player->preprocess_stream);
                audio_stream_stop(player->decode_stream);
                audio_mutex_lock(player->state_lock);
                player->state = PLAYER_STATE_ERROR;
                if (player->listen)
                {
                    player->listen(player, PLAY_INFO_IDLE, player->userdata);
                }
                audio_mutex_unlock(player->state_lock);
                device.stop(&device);
                device.close(&device);
                continue;
            }
            memset(read_buf, 0, frame_size * 2);
            while (1)
            {
                //OS_LOG_D(player,"playback_run:read frame_size:%d",frame_size);
                memset(read_buf + oddframe, 0x0, frame_size);
                read_size = audio_stream_read(player->decode_stream, read_buf + oddframe, frame_size);
                if (read_size == -1)
                {
                    RK_AUDIO_LOG_E("do not read data=0...");
                    audio_stream_stop(player->decode_stream);
                    break;
                }
                if (read_size == 0)
                {
                    RK_AUDIO_LOG_E("do not read data");
                    audio_stream_stop(player->decode_stream);
                    break;
                }
                switch (player->state)
                {
                case PLAYER_STATE_PAUSED:
                {
                    RK_AUDIO_LOG_D("PLAYER_STATE_PAUSED.");
                    device.stop(&device);
                    device.close(&device);
                    RK_AUDIO_LOG_D("play pause");
                    audio_semaphore_take(player->pause_sem);
                    if (player->state == PLAYER_STATE_STOP)
                    {
                        RK_AUDIO_LOG_D("play resume->stop");
                        goto PLAYBACK_STOP;
                    }
                    device.open(&device, &device_cfg);
                    device.start(&device);
                    RK_AUDIO_LOG_D("play resume");
                    /* fall through */
                }
                case PLAYER_STATE_RUNNING:
                {
                    total_pcm_cnt += (read_size / 4);
                    device.write(&device, read_buf + oddframe, read_size);
                    break;
                }
                default:
                {
                    break;
                }
                }
                if (read_size < frame_size)
                {
                    RK_AUDIO_LOG_D("underrun.");
                    break;
                }
                oddframe = (oddframe == 0) ? frame_size : 0;
            }
            if (read_buf)
            {
                RK_AUDIO_LOG_D("free read_buf");
                audio_free(read_buf);
            }
            device.stop(&device);
            device.close(&device);
PLAYBACK_STOP:
            audio_mutex_lock(player->state_lock);
            player->state = PLAYER_STATE_IDLE;
            if (player->listen)
            {
                player->listen(player, PLAY_INFO_IDLE, player->userdata);
            }
            audio_mutex_unlock(player->state_lock);
            RK_AUDIO_LOG_V("stop");
        }
    }
}

player_handle_t player_create(player_cfg_t *cfg)
{
    player_handle_t player = (player_handle_t) audio_calloc(1, sizeof(*player));
    RK_AUDIO_LOG_D("in");
    if (player)
    {
        player->preprocess_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        player->decode_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        player->play_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        player->preprocess_stream = audio_stream_create(cfg->preprocess_buf_size);
        player->decode_stream = audio_stream_create(cfg->decode_buf_size);
        player->state_lock = audio_mutex_create();
        player->pause_sem = audio_semaphore_create();
        player->tag = cfg->tag;
        player->listen = cfg->listen;
        player->userdata = cfg->userdata;
        player->name = cfg->name;
        player->device = cfg->device;
        player->resample_rate = cfg->resample_rate;
        player->state = PLAYER_STATE_IDLE;

        audio_thread_cfg_t c =
        {
            .run = (void *)preprocess_run,
            .args = player
        };
        player->preprocess_task = audio_thread_create("preprocess_task", 1024, 28, &c);
        c.run = (void *)decoder_run;
        c.args = player;
        player->decode_task = audio_thread_create("decode_task", 2048, 28, &c);
        c.run = (void *)playback_run;
        c.args = player;
        player->play_task = audio_thread_create("play_task", 2048, 28, &c);
    }
    RK_AUDIO_LOG_V("Success");
    return player;
}

int player_play(player_handle_t self, play_cfg_t *cfg)
{
    player_handle_t player = self;
    media_sdk_msg_t msg ;
    char *targetBuf = NULL;

    g_player_freq = cfg->freq_t;
    player_freq_init();

    // audio_mutex_lock(player->state_lock);
    // player->state = PLAYER_STATE_RUNNING;
    // audio_mutex_unlock(player->state_lock);
    player->samplerate = cfg->samplerate;
    player->bits = cfg->bits;
    player->channels = cfg->channels;
    msg.type = CMD_PLAYER_PLAY;
    msg.player.mode = PLAY_MODE_PROMPT;
    RK_AUDIO_LOG_V("%s", cfg->target);
    msg.player.preprocessor = cfg->preprocessor;
    msg.player.need_free = cfg->need_free;
    audio_free(music_name);
    if (music_name = audio_malloc(strlen(cfg->target) + 1))
    {
        memcpy(music_name, cfg->target, strlen(cfg->target));
    }
    if (cfg->need_free)
    {
        targetBuf = audio_malloc(strlen(cfg->target) + 1);
        if (targetBuf == NULL)
        {
            RK_AUDIO_LOG_E("no mem!");
            return RK_AUDIO_FAILURE;
        }
        memcpy(targetBuf, cfg->target, strlen(cfg->target));
        msg.player.target = targetBuf;
    }
    else
    {
        msg.player.target = cfg->target;
    }

    if (music_need_seek)
    {
        music_pos = music_seek;
    }
    else
    {
        music_pos = 0;
    }
    music_frame_offset = 0;
    total_pcm_cnt = 0;
    msg.player.end_session = false;
    audio_queue_send(player->preprocess_queue, &msg);

    return RK_AUDIO_SUCCESS;
}

player_state_t player_get_state(player_handle_t self)
{
    player_state_t state;
    audio_mutex_lock(self->state_lock);
    state = self->state;
    audio_mutex_unlock(self->state_lock);
    return state;
}

int player_get_cur_time(player_handle_t self)
{
    return (total_pcm_cnt * 1000) / 16000;
}

int player_stop(player_handle_t self)
{
    player_state_t state;
    int result;
    player_listen_cb list_callback;
    audio_mutex_lock(self->state_lock);
    if (self->state)
    {
        list_callback = self->listen; //force stop not callback
        self->listen = NULL;
        audio_stream_stop(self->preprocess_stream);
        RK_AUDIO_LOG_D("audio_stream_stop preprocess_stream");
        audio_stream_stop(self->decode_stream);
        if (self->state == PLAYER_STATE_PAUSED)
        {
            self->state = PLAYER_STATE_STOP;
            audio_semaphore_give(self->pause_sem);
        }
        audio_mutex_unlock(self->state_lock);
        while ((self->state != PLAYER_STATE_IDLE) && (self->state != PLAYER_STATE_ERROR))
        {
            audio_sleep(10);
        }
        self->listen = list_callback;
        result = 0;
        RK_AUDIO_LOG_V("stop player,pause/running state\n");
    }
    else
    {
        self->state = PLAYER_STATE_IDLE;
        audio_mutex_unlock(self->state_lock);
        RK_AUDIO_LOG_V("stop player,idle state\n");
        result = 0;
    }
    player_freq_deinit();
    return result;
}

int player_pause(player_handle_t self)
{
    audio_mutex_lock(self->state_lock);
    if (self->state == PLAYER_STATE_RUNNING)
    {
        self->state = PLAYER_STATE_PAUSED;
    }
    audio_mutex_unlock(self->state_lock);

    player_freq_deinit();
    return RK_AUDIO_SUCCESS;
}

int player_resume(player_handle_t self)
{

    player_freq_init();

    audio_mutex_lock(self->state_lock);
    if (self->state == PLAYER_STATE_PAUSED)
    {
        self->state = PLAYER_STATE_RUNNING;
        audio_semaphore_give(self->pause_sem);
    }
    audio_mutex_unlock(self->state_lock);

    return RK_AUDIO_SUCCESS;
}

int player_wait_idle(player_handle_t self)
{
    if (self->listen)
    {
        audio_mutex_lock(self->state_lock);
        self->listen(self, self->state, self->userdata);
        if (self->state == PLAY_INFO_IDLE)
        {
            RK_AUDIO_LOG_D("idle.....");
            audio_mutex_unlock(self->state_lock);
            return PLAY_INFO_IDLE;
        }
        audio_mutex_unlock(self->state_lock);
    }
    return RK_AUDIO_SUCCESS;
}

int player_close(player_handle_t self)
{
    playback_device_t device = self->device;
    device.close(&device);

    return RK_AUDIO_SUCCESS;
}

void player_destroy(player_handle_t self)
{
    player_handle_t player = self;
    playback_device_t device = player->device;
    RK_AUDIO_LOG_D("player_destory in");
    if (player)
    {
        device.close(&device);
        audio_queue_destroy(player->preprocess_queue);
        audio_queue_destroy(player->decode_queue);
        audio_queue_destroy(player->play_queue);
        audio_stream_destroy(player->preprocess_stream);
        audio_stream_destroy(player->decode_stream);
        audio_mutex_destroy(player->state_lock);
        audio_semaphore_destroy(player->pause_sem);
        audio_thread_exit(player->preprocess_task);
        audio_thread_exit(player->decode_task);
        audio_thread_exit(player->play_task);
    }
    if (decoder_cfg)
    {
        audio_free(decoder_cfg);
        decoder_cfg = NULL;
    }
    if (processor_cfg)
    {
        audio_free(processor_cfg);
        processor_cfg = NULL;
    }
    RK_AUDIO_LOG_D("player_destory player free.");
    audio_free(player);
    player = NULL;
}

void player_deinit()
{
    if (g_default_decoder)
    {
        RK_AUDIO_LOG_D("player deinit.");
        audio_free(g_default_decoder);
        g_default_decoder = NULL;
    }
}
