/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "AudioConfig.h"

/* int http_socket = -1; */
/* unsigned char g_is_chunked = 0; */
/* unsigned long http_totalsize; */

static int check_http_audio_type(char *target, char **type)
{
    if (!strcmp(target, "audio/mpeg") || !strcmp(target, "audio/mp3"))
    {
        *type = "mp3";
    }
    else if (!strcmp(target, "application/octet-stream"))
    {
        *type = "mp3";  /* consider echocloud platform just provide mp3 */
        RK_AUDIO_LOG_D("# Content-Type: = %s #\n", target);
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
    return 0;
#if 0
    bool bSucc = FALSE;
    int mem_size = HTTP_DOWNLOAD_FILE_NAME_LENGTH;
    char *response = NULL;

    response = (char *) rkos_memory_malloc(mem_size * sizeof(char));
    http_socket = -1;
    if (http_open(cfg->target, response, &http_socket) != HTTP_STATUS_OK)
    {
        goto END;
    }

    g_is_chunked = FALSE;
    struct HTTP_RES_HEADER resp = parse_header(response);
    printf("\n>>>>http header parse success:<<<<\n");
    printf("\tHTTP reponse: %d\n", resp.status_code);
    if (resp.status_code != 200 && resp.status_code != 206)
    {
        printf("file can't be download,status code: %d\n", resp.status_code);
        bSucc = FALSE;
        goto END;
    }
    printf("\tHTTP file type: %s\n", resp.content_type);

    g_is_chunked = is_chunked(resp);

    if (g_is_chunked)
    {
        printf("\tHTTP transfer-encoding: %s\n", resp.transfer_encoding);
    }
    else
    {
        printf("\tHTTP file length: %ld byte\n\n", resp.content_length);
        http_totalsize = resp.content_length;
    }
    cfg->file_size = resp.content_length;
    if (!cfg->isOta)
    {
        int ret = check_http_audio_type(resp.content_type, &cfg->type);
        if (ret != 0)
        {
            printf("can't find decode type");
            bSucc = FALSE;
            goto END;
        }
    }

    self->userdata = (void *)&http_socket;
    cfg->frame_size = HTTP_PREPROCESSOR_FRAME_SIZE;
    printf("[%s] open native http ok, http: %s, audio type:%s",
           cfg->tag,
           cfg->target,
           cfg->type);
    bSucc = TRUE;

END:
    if (response)
        voice_os_free(response);
    if (!bSucc)
    {
        if (http_socket != -1)
        {
            close(http_socket);
            printf("****** close socket. ******\n");
        }
        return FAIL;
    }
    return OK;
#endif
}

int http_preprocessor_read_impl(struct play_preprocessor *self, char *data,
                                size_t data_len)
{
    return 0;
#if 0
    //OS_LOG_D(http_preprocessor,"read data_len:%d",data_len);
    int rs = FAIL;

    if (!self->userdata)
    {
        return FAIL;
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
                printf("read data underrun,data_len:%d has_read:%d", data_len, rs);
            }
            http_totalsize -= rs;
        }
        else if (rs == 0)
        {
            if (http_totalsize > 0)
            {
                printf("http read data fail");
                return -101; //http not end, return this val for app stop player
            }
            else
            {
                printf("http read data over");
            }
        }
        else if (rs == -1)
        {
            //socket error, if return -101, watch dog will reset system
            return 0;
        }

    }

    return rs;
#endif
}
void http_preprocessor_destroy_impl(struct play_preprocessor *self)
{
#if 0
    if (!self)
        return;
    if (!self->userdata)
    {
        return;
    }

    int client_socket = *(int *)self->userdata;
    http_close(client_socket);
#endif
}
