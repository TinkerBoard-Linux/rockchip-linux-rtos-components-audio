/*
 *  Copyright (C) 2020, Fuzhou Rockchip Electronics Co., Ltd.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Fuzhou Rockchip Electronics Co., Ltd. nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __AP_AEC_H__
#define __AP_AEC_H__

#include "AP_Common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RKAP_AEC_State_S
{
    /* Basic info */
    int swSampleRate;        /* 8k~48k */
    int swFrameLen;           /* frame time only 10ms|16ms|20ms */
    const char *pathPara;
} RKAP_AEC_State;

typedef enum AecTransType
{
    AEC_TX_TYPE = 0,  // aec
    AEC_RX_TYPE       // adc
} RKAP_AEC_TRANS_ENUM;

extern RKAP_Handle AEC_Init(RKAP_AEC_State *st, RKAP_AEC_TRANS_ENUM transType);
extern void AEC_Destroy(RKAP_Handle handle);
extern int AEC_Process(RKAP_Handle handle, short *pfSigIn,
                       short *pfSigRef, short *pfSigOut);
extern void AEC_DumpVersion(void);

#ifdef __cplusplus
}
#endif

#endif /* __AP_AEC_H__ */
