/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

int g_encoder_num;
int g_recordr_freq = 0;
int g_wav_encode;
static record_encoder_t *g_default_encoder = NULL;
static record_encoder_cfg_t *encoder_cfg;
extern struct wav_header m_wav_header;

struct recorder
{
    struct audio_player_queue *write_queue;
    struct audio_player_queue *encode_queue;
    struct audio_player_queue *record_queue;

    struct audio_player_stream *encode_stream;
    struct audio_player_stream *record_stream;

    audio_player_mutex_handle state_lock;
    audio_player_semaphore_handle pause_sem;

    audio_player_thread_handle write_task;
    audio_player_thread_handle encode_task;
    audio_player_thread_handle record_task;

    record_writer_t writer;

    const char *target;
    //os_event_group_handle_t event;

    recorder_state_t state;

    const char *tag;

    recorder_listen_cb listen;
    void *userdata;

    const char *device_name;
    capture_device_t device;

    int samplerate;
    int bits;
    int channels;
    long duration;
};

void recoder_freq_init(void)
{
    switch (g_recordr_freq)
    {
    case RECORD_FREQ_LOCAL:
        // rkpm_add_freq_scene(FREQ_LOCALRECODER);
        break;
    default:
        RK_AUDIO_LOG_E("\n==recoder_freq_init: error!!!==\n");
        while (1);
        break;
    }
}

void recoder_freq_deinit(void)
{
    switch (g_recordr_freq)
    {
    case RECORD_FREQ_LOCAL:
        // rkpm_remove_freq_scene(FREQ_LOCALRECODER);
        break;
    default:
        break;
    }
}

int recorder_init(void)
{
    g_encoder_num = 3;
    if (!g_default_encoder)
        g_default_encoder = (record_encoder_t *)audio_calloc(g_encoder_num, sizeof(*g_default_encoder));
    record_encoder_t pcm_encoder = DEFAULT_PCM_ENCODER;
    record_encoder_t amr_encoder = DEFAULT_AMR_ENCODER;
    record_encoder_t wav_encoder = DEFAULT_WAV_ENCODER;
    g_default_encoder[0] = pcm_encoder;
    g_default_encoder[1] = amr_encoder;
    g_default_encoder[2] = wav_encoder;

    return RK_AUDIO_SUCCESS;
}

int recorder_register_encoder(const char *type, record_encoder_t *encoder)
{
    return RK_AUDIO_SUCCESS;
}

void write_run(void *data)
{
    recorder_handle_t recorder = (recorder_handle_t) data;
    media_sdk_msg_t msg;
    media_sdk_msg_t encode_msg;
    record_writer_t writer;
    record_writer_cfg_t processor_cfg;
    int res;
    char *read_buf;
    char *temp_buf;
    long temp_count;
    long temp_size;
    size_t read_size = 0;
    size_t frame_size = 0;
    RK_AUDIO_LOG_D("in\n");
    while (1)
    {
        RK_AUDIO_LOG_D("writer get mssage\n");
        if (audio_queue_receive(recorder->write_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_D("write_run exit");
            return;
        }
        RK_AUDIO_LOG_D("writer queue get msg\n");
        writer = msg.recorder.writer;
        if (!writer.type)
        {
            writer.type = "unknown";
        }

        processor_cfg.target = msg.recorder.target;
        processor_cfg.tag = recorder->tag;
        RK_AUDIO_LOG_D("writer init begin writer.type:%s\n", writer.type);
        temp_size = recorder->samplerate * (recorder->bits >> 3) * recorder->channels * recorder->duration;
        RK_AUDIO_LOG_E("malloc temp_buf %d.\n", temp_size);
        temp_count = 0;
        temp_buf = audio_malloc(temp_size);
        if (!temp_buf)
        {
            RK_AUDIO_LOG_E("malloc temp_buf failed %d.\n", temp_size);
            return;
        }
        res = writer.init(&writer, &processor_cfg);
        RK_AUDIO_LOG_D("writer init after\n");
        frame_size = processor_cfg.frame_size;
        if (msg.recorder.need_free)
        {
            audio_free(msg.recorder.target);
            if (!res)
            {
                audio_mutex_lock(recorder->state_lock);
                recorder->state_lock = recorder_STATE_IDLE;
                if (recorder->listen)
                    recorder->listen(recorder, RECORD_INFO_WRITER, recorder->userdata);
                //j_os_event_group_set_bits(v1[10], v6, 2LL);
                audio_mutex_unlock(recorder->state_lock);
            }
        }
        else
        {
            if (res)
            {
                RK_AUDIO_LOG_D("writer init go to write out\n");
                goto __write_OUT;
            }
            read_buf = (char *)audio_malloc(frame_size);
            if (!read_buf)
            {
                RK_AUDIO_LOG_D("can't audio_malloc buffer");
                return;
            }
            do
            {
                read_size = audio_stream_read(recorder->encode_stream, read_buf, frame_size);
                //OS_LOG_D(recorder,"writer read data read_size:%d\n",read_size);
                if (read_size == 0)
                {
                    break;
                }
                if (read_size != frame_size)
                {
                    break;
                }
                if (temp_count < temp_size)
                {
                    memcpy(temp_buf + temp_count, read_buf, read_size);
                    temp_count += read_size;
                }
                //OS_LOG_D(recorder,"writer read_size:%d\n",read_size);
            }
            while (1);
            // while (writer.write(&writer, read_buf, read_size) != -1);
            if (g_wav_encode)
                writer.write(&writer, (char *)&m_wav_header, sizeof(m_wav_header));
            writer.write(&writer, temp_buf, temp_count);
            RK_AUDIO_LOG_D("write_stream was stopped");
            audio_free(temp_buf);
            audio_free(read_buf);
            writer.destroy(&writer);
        }
    }
__write_OUT:
    RK_AUDIO_LOG_D("writer out\n");
}


int encoder_input(void *userdata, char *data, size_t data_len)
{
    recorder_handle_t recorder = (recorder_handle_t) userdata;
    return audio_stream_read(recorder->record_stream, data, data_len);
}
int encoder_output(void *userdata, const char *data, size_t data_len)
{
    recorder_handle_t recorder = (recorder_handle_t) userdata;
    return audio_stream_write(recorder->encode_stream, data, data_len);
}
int encoder_post(void *userdata, int samplerate, int bits, int channels)
{
    // only decoder need.
    return RK_AUDIO_SUCCESS;
}

void encoder_run(void *data)
{
    recorder_handle_t recorder = (recorder_handle_t) data;
    //record_encoder_cfg_t* encoder_cfg = (record_encoder_cfg_t*)audio_calloc(1,sizeof(*encoder_cfg));
    char *audio_type;
    record_encoder_t encoder;
    bool is_found_encoder = false;
    int i = 0;
    media_sdk_msg_t encode_msg;
    media_sdk_msg_t msg;
    if (!encoder_cfg)
    {
        encoder_cfg = (record_encoder_cfg_t *)audio_calloc(1, sizeof(*encoder_cfg));
        encoder_cfg->input = encoder_input;
        encoder_cfg->output = encoder_output;
        encoder_cfg->post = encoder_post;
        encoder_cfg->userdata = recorder;
    }
    while (1)
    {
        if (audio_queue_receive(recorder->encode_queue, &encode_msg) == -1)
        {
            RK_AUDIO_LOG_D("encode_run can't get msg");
            return;
        }
        audio_type = encode_msg.recorder.type;
        RK_AUDIO_LOG_D("encode_run,get audio_type:%s\n", audio_type);
        for (i = 0; i < g_encoder_num; i++)
        {
            if (!strcmp(audio_type, g_default_encoder[i].type))
            {
                encoder = g_default_encoder[i];
                is_found_encoder = true;
                break;
            }
        }
        if (!is_found_encoder)
        {
            RK_AUDIO_LOG_D("encode_run, cant found encoder\n");
            audio_stream_stop(recorder->record_stream);
            audio_mutex_lock(recorder->state_lock);
            if (recorder->listen)
                recorder->listen(recorder, RECORD_INFO_ENCODE, recorder->userdata);
            recorder->state = recorder_STATE_IDLE;
            audio_mutex_unlock(recorder->state_lock);
        }
        else
        {
            RK_AUDIO_LOG_D("encode_run init begin\n");
            if (!strcmp(audio_type, "wav"))
            {
                struct wav_config wav_cfg;
                wav_cfg.sample_rate = recorder->samplerate;
                wav_cfg.bits = recorder->bits;
                wav_cfg.channels = recorder->channels;
                rk_dcache_ops(RK_HW_CACHE_CLEAN, recorder, sizeof(recorder));
                encoder.userdata = &wav_cfg;
                g_wav_encode = 1;
            }
            else
            {
                g_wav_encode = 0;
            }
            if (encoder.init(&encoder, encoder_cfg))
            {
                RK_AUDIO_LOG_D("encode_run, encoder init fail\n");
                audio_stream_stop(recorder->record_stream);
                audio_mutex_lock(recorder->state_lock);
                recorder->state = recorder_STATE_IDLE;
                if (recorder->listen)
                    recorder->listen(recorder, RECORD_INFO_ENCODE, recorder->userdata);
                audio_mutex_unlock(recorder->state_lock);
            }
            RK_AUDIO_LOG_D("encode_run init success\n");
            audio_stream_start(recorder->encode_stream);
            msg.type = CMD_RECORDER_START;
            msg.recorder.mode = RECORD_MODE_PROMPT;
            msg.recorder.target = encode_msg.recorder.target;
            msg.recorder.type = encode_msg.recorder.type;
            msg.recorder.writer = encode_msg.recorder.writer;
            msg.recorder.need_free = false;
            msg.recorder.end_session = false;
            audio_queue_send(recorder->write_queue, &msg);

            record_encoder_error_t encode_res = encoder.process(&encoder);
            RK_AUDIO_LOG_D("encode_run process success\n");
            RK_AUDIO_LOG_D("encode_res:%d\n", encode_res);

            switch (encode_res)
            {
            case RECORD_ENCODER_INPUT_ERROR:
                RK_AUDIO_LOG_D("record_encoder_INPUT_ERROR\n");
                break;
            case RECORD_ENCODER_OUTPUT_ERROR:
                RK_AUDIO_LOG_D("record_encoder_OUTPUT_ERROR\n");
                audio_stream_stop(recorder->record_stream);
                if (!encoder.get_post_state(&encoder))
                {
                    audio_mutex_lock(recorder->state_lock);
                    recorder->state = recorder_STATE_IDLE;
                    if (recorder->listen)
                    {
                        recorder->listen(recorder, RECORD_INFO_ENCODE, recorder->userdata);
                    }
                    audio_mutex_unlock(recorder->state_lock);
                    goto _DESTROY_encoder;
                }
                goto _DESTROY_encoder;
            case RECORD_ENCODER_ENCODE_ERROR:
                RK_AUDIO_LOG_D("record_encoder_encode_ERROR\n");
                audio_stream_stop(recorder->record_stream);
                audio_stream_stop(recorder->encode_stream);
                break;
            default:
                RK_AUDIO_LOG_D("audio_stream_finish\n");
                audio_stream_finish(recorder->encode_stream);
                goto _DESTROY_encoder;
                break;
            }
            if (!encoder.get_post_state(&encoder))
            {
                audio_mutex_lock(recorder->state_lock);
                recorder->state = recorder_STATE_IDLE;
                if (recorder->listen)
                {
                    recorder->listen(recorder, RECORD_INFO_ENCODE, recorder->userdata);
                }
                audio_mutex_unlock(recorder->state_lock);
            }
_DESTROY_encoder:
            RK_AUDIO_LOG_D("encoder process return value:%d\n", encode_res);
            encoder.destroy(&encoder);
        }
    }
}

void capture_run(void *data)
{
    recorder_handle_t recorder = (recorder_handle_t) data;
    capture_device_t device = recorder->device;
    capture_device_cfg_t device_cfg;
    media_sdk_msg_t msg, encode_msg;
    char *read_buf;
    int read_size, write_size;
    size_t frame_size;
    RK_AUDIO_LOG_D("capture_run start\n");
    while (1)
    {
        if (audio_queue_receive(recorder->record_queue, &msg) == -1)
        {
            RK_AUDIO_LOG_D("capture_run receive data failed\n");
            return;
        }
        RK_AUDIO_LOG_D("capture_run:msg.type = %x,%d\n", &msg.type, msg.type);

        // if(encode_msg.type == CMD_RECORDER_START)  {
        if (msg.type == CMD_RECORDER_START)
        {
            device_cfg.samplerate = recorder->samplerate;
            device_cfg.bits = recorder->bits;
            device_cfg.channels = recorder->channels;
            device_cfg.device_name = recorder->device_name;
            device_cfg.frame_size = 4096;
            audio_stream_start(recorder->record_stream);
            encode_msg.type = msg.type;
            encode_msg.recorder.mode = msg.recorder.mode;
            encode_msg.recorder.target = msg.recorder.target;
            encode_msg.recorder.type = msg.recorder.type;
            encode_msg.recorder.writer = msg.recorder.writer;
            encode_msg.recorder.need_free = msg.recorder.need_free;
            encode_msg.recorder.end_session = msg.recorder.end_session;
            encode_msg.recorder.priv_data = "pcm";
            audio_queue_send(recorder->encode_queue, &encode_msg);

            RK_AUDIO_LOG_D("device open start.");
            if (device.open(&device, &device_cfg))
            {
                audio_stream_stop(recorder->record_stream);
                RK_AUDIO_LOG_E("failed\n");
                break;
            }
            else
            {
                frame_size = device_cfg.frame_size;
                device.start(&device);
                read_buf = (char *)audio_malloc(frame_size);
                if (!read_buf)
                {
                    RK_AUDIO_LOG_D("create read buf failed\n");
                    break;
                }
                memset(read_buf, 0, frame_size);
                while (1)
                {
                    // RK_AUDIO_LOG_D("audio_stream_read frame_size:%d\n",frame_size);
                    read_size = device.read(&device, read_buf, frame_size);
                    if (read_size == -1)
                    {
                        audio_stream_stop(recorder->record_stream);
                        break;
                    }
                    if (read_size == 0)
                    {
                        RK_AUDIO_LOG_D("do not read data\n");
                        audio_stream_finish(recorder->record_stream);
                        break;
                    }
                    write_size = audio_stream_write(recorder->record_stream, read_buf, read_size);
                    memset(read_buf, 0, frame_size);

                    if (write_size < frame_size)
                    {
                        RK_AUDIO_LOG_D("read read_size < frame_size, go to stop\n");
                        break;
                    }

                    if (write_size == -1)
                    {
                        audio_stream_stop(recorder->record_stream);
                        break;
                    }
                }
                audio_free(read_buf);
                device.stop(&device);
                device.close(&device);
                audio_mutex_lock(recorder->state_lock);
                recorder->state = recorder_STATE_IDLE;
                if (recorder->listen)
                {
                    recorder->listen(recorder, RECORD_INFO_STOP, recorder->userdata);
                }
                audio_mutex_unlock(recorder->state_lock);
                RK_AUDIO_LOG_D("record_run close");
            }
        }
        else if (encode_msg.type == CMD_RECORDER_STOP)
        {
            RK_AUDIO_LOG_D("record_run stop");
            break;
        }
    }
}

recorder_handle_t recorder_create(recorder_cfg_t *cfg)
{
    recorder_handle_t recorder = (recorder_handle_t) audio_calloc(1, sizeof(*recorder));
    RK_AUDIO_LOG_D("recorder_create in");
    if (recorder)
    {
        recorder->write_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        recorder->encode_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        recorder->record_queue = audio_queue_create(1, sizeof(media_sdk_msg_t));
        recorder->record_stream = audio_stream_create(cfg->record_buf_size);
        recorder->encode_stream = audio_stream_create(cfg->encode_buf_size);
        recorder->state_lock = audio_mutex_create();
        recorder->pause_sem = audio_semaphore_create();
        recorder->tag = cfg->tag;
        recorder->listen = cfg->listen;
        recorder->userdata = cfg->userdata;
        recorder->device_name = cfg->device_name;
        recorder->device = cfg->device;
        recorder->state = recorder_STATE_IDLE;

        audio_thread_cfg_t c =
        {
            .run = write_run,
            .args = recorder
        };
        recorder->write_task = audio_thread_create("write_task", 1024, 28, &c);
        c.run = encoder_run;
        c.args = recorder;
        recorder->encode_task = audio_thread_create("encode_task", 2048, 28, &c);
        c.run = capture_run;
        c.args = recorder;
        recorder->record_task = audio_thread_create("record_task", 1024, 28, &c);
    }
    RK_AUDIO_LOG_D("recorder_create out");
    return recorder;
}

int recorder_record(recorder_handle_t self, record_cfg_t *cfg)
{
    RK_AUDIO_LOG_D("start type %s\n", cfg->type);

    g_recordr_freq = cfg->freq_t;
    recoder_freq_init();

    recorder_handle_t recorder = self;
    recorder_state_t state;
    media_sdk_msg_t msg;

    audio_mutex_lock(recorder->state_lock);
    recorder->state = recorder_STATE_RUNNING;
    audio_mutex_unlock(recorder->state_lock);
    recorder->samplerate = cfg->samplerate;
    recorder->bits = cfg->bits;
    recorder->channels = cfg->channels;
    recorder->duration = cfg->duration;
    msg.type = CMD_RECORDER_START;
    msg.recorder.mode = RECORD_MODE_PROMPT;
    msg.recorder.type = cfg->type;
    msg.recorder.writer = cfg->writer;
    msg.recorder.need_free = cfg->need_free;
    if (cfg->need_free)
    {
        msg.recorder.target = audio_malloc(strlen(cfg->target) + 1);
        if (msg.recorder.target == NULL)
        {
            RK_AUDIO_LOG_V("no mem!");
            return RK_AUDIO_FAILURE;
        }
        memcpy(msg.recorder.target, cfg->target, strlen(cfg->target));
    }
    else
    {
        msg.recorder.target = cfg->target;
    }

    msg.recorder.end_session = false;
    RK_AUDIO_LOG_D("msg.type =%x, %d,", &msg.type, msg.type);
    audio_queue_send(recorder->record_queue, &msg);
    return RK_AUDIO_SUCCESS;
}

recorder_state_t recorder_get_state(recorder_handle_t self)
{
    recorder_state_t state;
    audio_mutex_lock(self->state_lock);
    state = self->state;
    audio_mutex_unlock(self->state_lock);
    return state;
}
int recorder_stop(recorder_handle_t self)
{
    recorder_state_t state;
    recorder_handle_t recorder = self;
    media_sdk_msg_t msg;

    int result;
    audio_mutex_lock(self->state_lock);
    if (self->state)
    {
        audio_stream_stop(self->record_stream);
        if (self->state == RECORD_INFO_PAUSED)
        {
            audio_semaphore_give(self->pause_sem);
        }
        audio_mutex_unlock(self->state_lock);
        result = 0;
        msg.type = CMD_RECORDER_STOP;
        audio_queue_send(recorder->record_queue, &msg);
        RK_AUDIO_LOG_D("recorder_stop stop recorder,pause/running state\n");
    }
    else
    {
        audio_mutex_unlock(self->state_lock);
        RK_AUDIO_LOG_D("recorder_stop stop recorder,idle state\n");
        result = 0;
    }

    recoder_freq_deinit();

    return result;
}
int recorder_pause(recorder_handle_t self)
{
    recoder_freq_deinit();
    return RK_AUDIO_SUCCESS;
}
int recorder_resume(recorder_handle_t self)
{
    recoder_freq_init();
    return RK_AUDIO_SUCCESS;
}
int recorder_wait_idle(recorder_handle_t self)
{
    return RK_AUDIO_SUCCESS;
}
void recorder_destroy(recorder_handle_t self)
{
    recorder_state_t state;
    recorder_handle_t recorder = self;
    RK_AUDIO_LOG_D("in.");
    if (recorder)
    {
        // device.close(&device);
        audio_queue_destroy(recorder->write_queue);
        audio_queue_destroy(recorder->encode_queue);
        audio_queue_destroy(recorder->record_queue);
        audio_stream_destroy(recorder->record_stream);
        audio_stream_destroy(recorder->encode_stream);
        audio_mutex_destroy(recorder->state_lock);
        audio_semaphore_destroy(recorder->pause_sem);
        audio_thread_exit(recorder->write_task);
        audio_thread_exit(recorder->encode_task);
        audio_thread_exit(recorder->record_task);
        audio_free(recorder);
        recorder = NULL;
    }
    if (encoder_cfg)
    {
        audio_free(encoder_cfg);
        encoder_cfg = NULL;
    }
}
void recorder_deinit(void)
{
    if (g_default_encoder)
    {
        audio_free(g_default_encoder);
        g_default_encoder = NULL;
    }

}
