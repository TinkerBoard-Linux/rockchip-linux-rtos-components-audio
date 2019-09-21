/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

#define MUSIC_MAX 5

void player_callback_test(player_handle_t self, play_info_t info, void *userdata);

static char *file_name;
player_handle_t player_test = NULL;
static play_cfg_t *cfg_test = NULL;
static int play_loop;
static int music_index;
static player_cfg_t player_cfg =
{
    .preprocess_buf_size = 1024 * 5,
    .decode_buf_size = 1024 * 5,
    .tag = "one",
    .device = DEFAULT_PLAYBACK_DEVICE,
    .listen = player_callback_test,
};

void player_callback_test(player_handle_t self, play_info_t info, void *userdata)
{
    if (play_loop)
    {
        music_index++;
        if (music_index >= MUSIC_MAX)
            music_index = 0;
        sprintf(file_name, "/music/%d.mp3", music_index);
    }
    cfg_test->target = file_name;
    player_stop(player_test);
    player_play(player_test, cfg_test);
}

void *play_test_start(int argc, void *argv[])
{
    if (argc < 2)
    {
        RK_AUDIO_LOG_D("play_test_start <filepath> [-D card] [-r resample rate]\n");
        return 0;
    }
    if (!cfg_test)
    {
        cfg_test = audio_malloc(sizeof(play_cfg_t));
        file_name = audio_malloc(64);
    }
    if (strcmp("loop", argv[1]) == 0)
    {
        play_loop = 1;
        music_index = 0;
        sprintf(file_name, "/music/%d.mp3", music_index);
    }
    else
    {
        play_loop = 0;
        memcpy(file_name, argv[1], 64);
    }
    if (!player_test)
    {
        player_init();
        player_cfg.name = audio_malloc(10);
        player_cfg.resample_rate = 0;
        sprintf(player_cfg.name, "es8388p");
        while (*argv)
        {
            if (strcmp(*argv, "-D") == 0)
            {
                argv++;
                if (*argv)
                    strcpy(player_cfg.name, *argv);
            }
            else if (strcmp(*argv, "-r") == 0)
            {
                argv++;
                if (*argv)
                    player_cfg.resample_rate = atoi(*argv);
            }
            if (*argv)
                argv++;
        }
        player_test = player_create(&player_cfg);
    }
    else
    {
        player_stop(player_test);
    }
    cfg_test->target = "memory_data";
    cfg_test->preprocessor = (play_preprocessor_t)DEFAULT_FILE_PREPROCESSOR;
    cfg_test->freq_t = PLAY_FREQ_LOCALPLAY;
    cfg_test->need_free = false;
    RK_AUDIO_LOG_D("%s:now play %s\n", __func__, cfg_test->target);
    player_play(player_test, cfg_test);

    return 0;
}

void *play_test_stop(int argc, void *argv[])
{
    if (!player_test)
        return 0;

    player_stop(player_test);
    if (strcmp(argv[1], "free") == 0)
    {
        audio_free(file_name);
        audio_free(cfg_test);
        cfg_test = NULL;
        audio_free(player_cfg.name);
        player_cfg.name = NULL;
        player_destroy(player_test);
        player_test = NULL;
        player_deinit();
    }

    return 0;
}

#ifdef OS_IS_RTTHREAD
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(play_test_start, play test start);
MSH_CMD_EXPORT(play_test_stop, play test stop);
#endif
#endif
