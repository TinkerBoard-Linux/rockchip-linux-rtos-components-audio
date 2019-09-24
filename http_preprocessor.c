/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"
#include "net/http/http_api.h"

#define DBG_HTTP_PRE_INFO 0

#if DBG_HTTP_PRE_INFO
#define DBG_HTTP_PRE(fmt, arg...) rk_printf("[http_player] " fmt "\n", ##arg)
#else
#define DBG_HTTP_PRE(fmt, arg...) NULL
#endif

int http_socket = -1;
unsigned char g_is_chunked = 0;
unsigned long http_totalsize;

static int check_http_audio_type(char *target, char **type)
{
    if (!strcmp(target, "audio/mpeg") || !strcmp(target, "audio/mp3"))
    {
        *type = "mp3";
    }
    else if (!strcmp(target, "application/octet-stream"))
    {
        *type = "mp3";  /* consider echocloud platform just provide mp3 */
        DBG_HTTP_PRE("# Content-Type: = %s #\n", target);
    }
    else
    {
        return RK_AUDIO_FAILURE;
    }
    return RK_AUDIO_SUCCESS;
}

int http_preprocessor_init_impl(struct play_preprocessor *self,
                                play_preprocessor_cfg_t *cfg)
{
    bool bSucc = RK_AUDIO_FAIL;
    int mem_size = HTTP_DOWNLOAD_FILE_NAME_LENGTH;
    char *response = NULL;

    response = (char *) audio_malloc(mem_size * sizeof(char));
    http_socket = -1;
    if (http_open(cfg->target, response, &http_socket) != HTTP_STATUS_OK)
    {
        goto END;
    }

    g_is_chunked = RK_AUDIO_FAIL;
    struct HTTP_RES_HEADER resp = parse_header(response);
    DBG_HTTP_PRE("\n>>>>http header parse success:<<<<\n");
    DBG_HTTP_PRE("\tHTTP reponse: %d\n", resp.status_code);
    if (resp.status_code != 200 && resp.status_code != 206)
    {
        DBG_HTTP_PRE("file can't be download,status code: %d\n", resp.status_code);
        bSucc = RK_AUDIO_FAIL;
        goto END;
    }
    DBG_HTTP_PRE("\tHTTP file type: %s\n", resp.content_type);

    g_is_chunked = is_chunked(resp);

    if (g_is_chunked)
    {
        DBG_HTTP_PRE("\tHTTP transfer-encoding: %s\n", resp.transfer_encoding);
    }
    else
    {
        DBG_HTTP_PRE("\tHTTP file length: %ld byte\n\n", resp.content_length);
        http_totalsize = resp.content_length;
    }
    cfg->file_size = resp.content_length;
    if (!cfg->isOta)
    {
        int ret = check_http_audio_type(resp.content_type, &cfg->type);
        if (ret != 0)
        {
            DBG_HTTP_PRE("can't find decode type");
            bSucc = RK_AUDIO_FAIL;
            goto END;
        }
    }

    self->userdata = (void *)&http_socket;
    cfg->frame_size = HTTP_PREPROCESSOR_FRAME_SIZE;
    DBG_HTTP_PRE("[%s] open native http ok, http: %s, audio type:%s",
           cfg->tag,
           cfg->target,
           cfg->type);
    bSucc = RK_AUDIO_TRUE;

END:
    if (response)
        audio_free(response);
    if (!bSucc)
    {
        if (http_socket != -1)
        {
            close(http_socket);
            DBG_HTTP_PRE("****** close socket. ******\n");
        }
        return RK_AUDIO_FAILURE;
    }
    return RK_AUDIO_SUCCESS;
}

int http_preprocessor_read_impl(struct play_preprocessor *self, char *data,
                                size_t data_len)
{
    //DBG_HTTP_PRE("\nread data_len:%d",data_len);
    int rs = RK_AUDIO_FAIL;

    if (!self->userdata)
    {
        return RK_AUDIO_FAIL;
    }

    if (g_is_chunked)
    {
        rs = http_socket_read_chunk(*(int *)self->userdata, data, data_len);
    }
    else
    {
        if (http_totalsize == 0)
            return 0;

        if (data_len > http_totalsize)
            data_len = http_totalsize;
        rs = http_socket_read(*(int *)self->userdata, data, data_len);

        if (rs > 0)
        {
            if (rs != data_len)
            {
                DBG_HTTP_PRE("read data underrun,data_len:%d has_read:%d", data_len, rs);
            }
            http_totalsize -= rs;
        }
        else if (rs == 0)
        {
            if (http_totalsize > 0)
            {
                DBG_HTTP_PRE("http read data fail");
                return -101; //http not end, return this val for app stop player
            }
            else
            {
                DBG_HTTP_PRE("http read data over");
            }
        }
        else if (rs == -1)
        {
            //socket error, if return -101, watch dog will reset system
            return 0;
        }

    }

    return rs;
}
void http_preprocessor_destroy_impl(struct play_preprocessor *self)
{
    if (!self)
        return;
    if (!self->userdata)
    {
        return;
    }

    int client_socket = *(int *)self->userdata;
    http_close(client_socket);
}
