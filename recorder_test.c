#include <stdio.h>
#include "AudioConfig.h"

void recorder_callback(recorder_handle_t self, record_info_t info, void *userdata);

static int record_loop;
static int record_time;
static int recording;
static char *file_name;
audio_player_timer_handle rec_timer;
audio_player_thread_handle rec_timeout_tid;
recorder_handle_t recorder_test = NULL;
static record_cfg_t *cfg_test = NULL;
static recorder_cfg_t recorder_cfg =
{
    .record_buf_size = 1024 * 5,
    .encode_buf_size = 1024 * 5,
    .tag = "one",
    .listen = recorder_callback,
    .device = DEFAULT_CAPTURE_DEVICE,
};

void recorder_callback(recorder_handle_t self, record_info_t info, void *userdata)
{
}

void record_timeout(void *arg)
{
    while (1)
    {
        while (recording)
        {
            audio_sleep(100);
        }
        recorder_stop(recorder_test);
        RK_AUDIO_LOG_D("%s:now record %s\n", __func__, cfg_test->target);
        recorder_record(recorder_test, cfg_test);
        recording = 1;
    }
}

void record_timer_iqr(void *arg)
{
    recording = 0;
}

void *record_start(int argc, char **argv)
{
    if (argc < 2)
    {
        RK_AUDIO_LOG_D("record_start <filepath>\n");
        return RK_AUDIO_SUCCESS;
    }
    if (!cfg_test)
    {
        cfg_test = audio_malloc(sizeof(record_cfg_t));
        cfg_test->type = audio_malloc(10);
        file_name = audio_malloc(64);
    }
    memcpy(file_name, argv[1], 64);
    if (strstr(file_name, ".pcm"))
    {
        sprintf(cfg_test->type, "pcm");
    }
    else if (strstr(file_name, ".wav"))
    {
        sprintf(cfg_test->type, "wav");
    }
    else
    {
        RK_AUDIO_LOG_D("Not support this type:%s.\n", file_name);
        audio_free(cfg_test->type);
        audio_free(cfg_test);
        audio_free(file_name);
        cfg_test = NULL;
        return RK_AUDIO_SUCCESS;
    }
    cfg_test->channels = 2;
    cfg_test->samplerate = 16000;
    cfg_test->bits = 16;
    record_loop = 0;
    if (recorder_cfg.device_name == NULL)
    {
        recorder_cfg.device_name = audio_malloc(10);
        strcpy(recorder_cfg.device_name, "adcc");
    }
    while (*argv)
    {
        if (strcmp(*argv, "-D") == 0)
        {
            argv++;
            if (*argv)
                strcpy(recorder_cfg.device_name, *argv);
        }
        else if (strcmp(*argv, "-c") == 0)
        {
            argv++;
            if (*argv)
                cfg_test->channels = atoi(*argv);
        }
        else if (strcmp(*argv, "-r") == 0)
        {
            argv++;
            if (*argv)
                cfg_test->samplerate = atoi(*argv);
        }
        else if (strcmp(*argv, "-b") == 0)
        {
            argv++;
            if (*argv)
                cfg_test->bits = atoi(*argv);
        }
        else if (strcmp(*argv, "-l") == 0)
        {
            record_loop = 1;
            argv++;
            if (*argv)
                record_time = atoi(*argv) * AUDIO_TICK_PER_SEC;
            if (record_time < 5)
                record_time = 5;
            if (rec_timer)
            {
                audio_timer_stop(rec_timer);
                audio_timer_control(rec_timer, record_time, 0);
            }
            else
            {
                rec_timer =  audio_timer_create("rectime", record_time,
                                                AUDIO_TIMER_PERIOD,
                                                NULL, record_timer_iqr);
            }
        }
        if (*argv)
            argv++;
    }
    if (!recorder_test)
    {
        recorder_init();
        recorder_test = recorder_create(&recorder_cfg);
    }
    else
    {
        recorder_stop(recorder_test);
    }
    cfg_test->target = file_name;
    cfg_test->writer = (record_writer_t)DEFAULT_FILE_WRITER;
    cfg_test->freq_t = RECORD_FREQ_LOCAL;
    cfg_test->need_free = false;
    RK_AUDIO_LOG_D("%s:now record %s\n", __func__, cfg_test->target);
    if (record_loop)
    {
        recording = 1;
        if (!rec_timeout_tid)
        {
            audio_thread_cfg_t cfg;
            cfg.run = record_timeout;
            rec_timeout_tid = audio_thread_create("rect", 512, 18, &cfg);
        }
        audio_timer_start(rec_timer);
    }
    recorder_record(recorder_test, cfg_test);

    return RK_AUDIO_SUCCESS;
}

void *record_stop(int argc, char **argv)
{
    if (!recorder_test)
        return RK_AUDIO_SUCCESS;
    recorder_stop(recorder_test);
    if (rec_timer)
    {
        audio_timer_delete(rec_timer);
        rec_timer = NULL;
    }
    if (rec_timeout_tid)
    {
        audio_thread_exit(rec_timeout_tid);
        rec_timeout_tid = NULL;
    }
    if (strcmp(argv[1], "free") == 0)
    {
        audio_free(file_name);
        file_name = NULL;
        audio_free(cfg_test->type);
        audio_free(cfg_test);
        cfg_test = NULL;
        audio_free(recorder_cfg.device_name);
        recorder_cfg.device_name = NULL;
        recorder_destroy(recorder_test);
        recorder_test = NULL;
        recorder_deinit();
    }

    return RK_AUDIO_SUCCESS;
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(record_start, record test start);
MSH_CMD_EXPORT(record_stop, record test stop);
#endif
