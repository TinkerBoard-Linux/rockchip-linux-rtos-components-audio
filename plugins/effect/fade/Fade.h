/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 Fuzhou Rockchip Electronics Co., Ltd
 */

#ifndef _H_AUDIO_FADE
#define _H_AUDIO_FADE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#ifdef CODEC_24BIT
typedef int32_t    fade_short;
typedef int32_t int32_t   fade_int32_t;
#define fade_max_coef   16777215
#define fade_scale   24
#else
typedef short    fade_short;
typedef int32_t    fade_int32_t;
#define fade_max_coef   65535
#define fade_scale   16
#endif

#define FADE_IN     0
#define FADE_OUT    1
#define FADE_NULL   -1

//initialization.
//begin:the frist specimen serial number,len:length  type: 0-fade in 1-fade out.
void FadeInit(int32_t begin, int32_t len, int type);
void DC_filter(short *pwBuffer, unsigned short frameLen);
int32_t FadeDoOnce(void);
void FadeProcess(fade_short *pwBuffer, unsigned short frameLen);
int FadeIsFinished(void);

//#endif

#endif
