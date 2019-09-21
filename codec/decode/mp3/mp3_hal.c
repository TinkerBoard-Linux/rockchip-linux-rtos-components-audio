/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#include "mp3_hal.h"
#if defined(AUDIO_DECODER_MP3)

static MP3PLAYERINFO mpi;
static short *g_DecPCMbuf = NULL;

int32_t AudioDecMP3Open(uint32_t A2B_ShareDatAddr)
{
    printf("mp3 decode open\n");
    struct audio_server_data_share *pDecDat = (struct audio_server_data_share *)A2B_ShareDatAddr;
    pDecDat->dat[1] = MP3_AB_CORE_SHARE_ADDR_INVALID;
    mpi.mpi_mp3dec = MP3InitDecoder();
    if (mpi.mpi_mp3dec)
    {
        g_DecPCMbuf = (short *)pDecDat->dat[0];
        pDecDat->dat[1] = (int32_t)&mpi;
        return 0;
    }
    else
    {
        return MP3_AB_CORE_SHARE_ADDR_INVALID;
    }
}

int32_t AudioDecMP3Close(void)
{
    printf("mp3 decode close\n");
    if (mpi.mpi_mp3dec)
    {
        MP3FreeDecoder(mpi.mpi_mp3dec);
        mpi.mpi_mp3dec = NULL;
    }

    return 0;
}

int32_t AudioDecMP3Process(uint32_t decode_dat)
{
    int ret = -1;
    int skip = 0;
    struct audio_server_data_share *pDecDat = (struct audio_server_data_share *)decode_dat;
    unsigned char *decode_ptr = (unsigned char *)pDecDat->dat[0];
    int bytes_left = pDecDat->dat[1];
    int bytes_left_max = pDecDat->dat[1];

    do
    {

        if ((skip = MP3FindSyncWord(decode_ptr, bytes_left)) < 0)
        {
            printf("mp3 decode don't find sync word\n");
            bytes_left = 0;
            ret = PLAY_DECODER_DECODE_ERROR;
            goto end_decode;
        }
        bytes_left -= skip;
        decode_ptr += skip;
        /* printf("[debug] decode_ptr [%p] bytes_left [%x]\n\n", decode_ptr, bytes_left); */
        ret = MP3Decode(mpi.mpi_mp3dec, &decode_ptr, &bytes_left, g_DecPCMbuf, 0);
        if (ret != 0)
        {
            printf("mp3 decode failed ret:%d\n", ret);
            if ((bytes_left == bytes_left_max) && (skip == 0))
            {
                printf("skip one bytes in Bcore.\n");
                bytes_left --;
                decode_ptr ++;
            }
            if (ret == ERR_MP3_INDATA_UNDERFLOW)
            {
                bytes_left = 0;
                ret = PLAY_DECODER_DECODE_ERROR;
                goto end_decode;
            } /*else if (ret == ERR_MP3_MAINDATA_UNDERFLOW) {
                ret = PLAY_DECODER_DECODE_ERROR;
                goto end_decode;
            } else if (ret == ERR_MP3_INVALID_FRAMEHEADER) {
                ret = PLAY_DECODER_DECODE_ERROR;
                goto end_decode;
            } else if (ret == ERR_MP3_INVALID_HUFFCODES) {
                ret = PLAY_DECODER_DECODE_ERROR;
                goto end_decode;
            }*/
            ret = PLAY_DECODER_DECODE_ERROR;
            goto end_decode;
        }
        MP3GetLastFrameInfo(mpi.mpi_mp3dec, &mpi.mpi_frameinfo);
        ret = PLAY_DECODER_SUCCESS;
    }
    while (0);

end_decode:
    pDecDat->dat[0] = (uint32_t)decode_ptr;
    pDecDat->dat[1] = bytes_left;
    return ret;
}
#endif
