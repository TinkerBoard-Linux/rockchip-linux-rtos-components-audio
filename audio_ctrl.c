/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */
#include "AudioConfig.h"

#if defined(AUDIO_DECODER_MP3)
#include "mp3_hal.h"
#endif

#if defined(AUDIO_DECODER_AMR)
#include "amr_dec_hal.h"
#endif

#if defined(AUDIO_ENCODER_AMR)
#include "amr_enc_hal.h"
#endif

struct audio_server_data_share g_ABCoreDat =
{
    0
};
struct audio_server_data_share g_ABCoreDat;
struct audio_server_data_share g_asr_ABCoreDat;

uint32_t g_BcoreRetval = 0;

#if !defined(RK_AUDIO_CODEC_RUN_ON_M4) && defined(AUDIO_USING_DSP)
struct dsp_work *work_gp;
pSemaphore g_dsp_semaphore;
uint32_t g_DSPLoadCnt = 0;
HDC g_DSPDevHandle = NULL;
#endif

HDC AudioLoadDsp(void)
{
    HDC DSPDevHandle = NULL;
#if !defined(RK_AUDIO_CODEC_RUN_ON_M4) && defined(AUDIO_USING_DSP)
    int ret = RK_AUDIO_SUCCESS;
    if (g_dsp_semaphore == NULL)
    {
        g_dsp_semaphore = rkos_semaphore_create(1, 1);
        if (g_dsp_semaphore == NULL)
            return NULL;
    }

    rkos_semaphore_take(g_dsp_semaphore, MAX_DELAY);

    if (g_DSPLoadCnt == 0)
    {
        if (rkdev_create(DEV_CLASS_DSP, 0, NULL) != RK_AUDIO_SUCCESS)
        {
            RK_AUDIO_LOG_D("\r\n  dsp create fail");
        }

        DSPDevHandle = rkdev_open(DEV_CLASS_DSP, 0, NOT_CARE);
        if (DSPDevHandle == NULL)
        {
            RK_AUDIO_LOG_E("\r\n	dsp %d open fail", 0);
            rkos_semaphore_give(g_dsp_semaphore);
            return NULL;
        }
        rk_dsp_open(DSPDevHandle, 0);
    }
    g_DSPLoadCnt++;
    rkos_semaphore_give(g_dsp_semaphore);
#endif

    return DSPDevHandle;
}

void AudioUnloadDsp(void)
{
#if !defined(RK_AUDIO_CODEC_RUN_ON_M4) && defined(AUDIO_USING_DSP)
    rkos_semaphore_take(g_dsp_semaphore, MAX_DELAY);
    if (g_DSPLoadCnt == 0)
    {
        RK_AUDIO_LOG_D("\r\n [%s] DSP close failure", __func__);
    }
    else
    {
        g_DSPLoadCnt--;
        if (g_DSPLoadCnt == 0)
        {
            if (g_DSPDevHandle)
            {
                rk_dsp_close(g_DSPDevHandle);
                rkdev_close(g_DSPDevHandle);

                if (rkdev_delete(DEV_CLASS_DSP, 0, NULL) != RK_AUDIO_UCCESS)
                {
                    RK_AUDIO_LOG_E("\r\n  dsp delete fail");
                }
                g_DSPDevHandle = NULL;
            }
        }
    }
    rkos_semaphore_give(g_dsp_semaphore);
#endif
}

int AudioSendMsg(audio_data_type id, MSGBOX_SYSTEM_CMD mesg)
{
#ifdef RK_AUDIO_CODEC_RUN_ON_M4
    int ret = RK_AUDIO_FAILURE;

    switch (RK_AUDIO_CASE_CHECK(id, mesg))
    {
#if defined(AUDIO_DECODER_MP3)
    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN):
        ret = AudioDecMP3Open((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE):
        ret = AudioDecMP3Process((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_MP3_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE):
        ret = AudioDecMP3Close();
        break;
#endif

#if defined(AUDIO_DECODER_AMR)
    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE_OPEN):
        ret = AudioDecAmrOpen((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE):
        ret = AudioDecAmrProcess((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_DEC, MEDIA_MSGBOX_CMD_DECODE_CLOSE):
        ret = AudioDecAmrClose();
        break;
#endif

#if defined(AUDIO_ENCODER_AMR)
    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE_OPEN):
        ret = AudioAmrEncodeOpen((uint32_t)&g_ABCoreDat);
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE):
        ret = AudioAmrEncode();
        break;

    case RK_AUDIO_CASE_CHECK(TYPE_AUDIO_AMR_ENC, MEDIA_MSGBOX_CMD_ENCODE_CLOSE):
        ret = AudioAmrEncodeClose();
        break;
#endif
    default:
        RK_AUDIO_LOG_E("AudioSendMsg error.");
        break;
    }
    g_BcoreRetval = ret;
    return ret;
#else      /* -----  not RK_AUDIO_CODEC_RUN_ON_M4  ----- */
    rk_err_t ret = RK_AUDIO_SUCCESS;

    switch (mesg)
    {
    case MEDIA_MSGBOX_CMD_ENCODE_OPEN:
    case MEDIA_MSGBOX_CMD_DECODE_OPEN:
        g_DSPDevHandle = AudioLoadDsp();
        work_gp = rk_dsp_work_create(RKDSP_ALGO_WORK);
        if (work_gp)
        {
            work_gp->id = id;
            work_gp->algo_type = mesg;
            work_gp->param = (uint32_t)&g_ABCoreDat;
            work_gp->param_size = sizeof(g_ABCoreDat);
        }
        if (g_DSPDevHandle == NULL || work_gp == NULL)
        {
            ret = -1;
            break;
        }
        printf("Loading dsp ...");
    case MEDIA_MSGBOX_CMD_ENCODE:
    case MEDIA_MSGBOX_CMD_DECODE:
        work_gp->id = id;
        work_gp->algo_type = mesg;
        ret = rk_dsp_control(g_DSPDevHandle, RKDSP_CTL_QUEUE_WORK, work_gp);
        ret |= rk_dsp_control(g_DSPDevHandle, RKDSP_CTL_DEQUEUE_WORK, work_gp);
        break;
    case MEDIA_MSGBOX_CMD_ENCODE_CLOSE:
    case MEDIA_MSGBOX_CMD_DECODE_CLOSE:
        work_gp->id = id;
        work_gp->algo_type = mesg;
        ret = rk_dsp_control(g_DSPDevHandle, RKDSP_CTL_QUEUE_WORK, work_gp);
        ret |= rk_dsp_control(g_DSPDevHandle, RKDSP_CTL_DEQUEUE_WORK, work_gp);
        ret = rk_dsp_work_destroy(work_gp);
        if (g_DSPDevHandle)
        {
            printf("Close Dsp ...");
            AudioUnloadDsp();
        }
        break;
    default:
        printf("AudioSendMsg error.");
        ret = -1;
        break;
    }
    g_BcoreRetval = ret;
    return ret;
#endif     /* -----  end of RK_AUDIO_CODEC_RUN_ON_M4  ----- */
}
