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
#include "dsp_include/8k_to_44k.h"
#include "dsp_include/8k_to_48k.h"
#include "dsp_include/11k_to_44k.h"
#include "dsp_include/11k_to_48k.h"
#include "dsp_include/16k_to_44k.h"
#include "dsp_include/16k_to_48k.h"
#include "dsp_include/22k_to_48k.h"
#include "dsp_include/32k_to_44k.h"
#include "dsp_include/32k_to_48k.h"
#include "dsp_include/44k_to_48k.h"
#include "dsp_include/48k_to_44k.h"

//****************************************************************************
//
// SRCInit initializes the persistent state of the sample rate converter.  It
// must be called before SRCFilter or SRCFilter_S. It is the responsibility
// of the caller to allocate the memory needed by the sample rate converter,
// for both the persistent state structure and the delay line(s).  The number
// of elements in the delay line array must be (Max input samples per
// SRCFilter call) + NUMTAPS (less will result in errors and more will go
// unused).
//
//****************************************************************************
short lastSampleLeft = 0, lastSampleRight = 0;
int SRCInit(SRCState *pSRC, unsigned long ulInputRate, unsigned long ulOutputRate)
{
    long lNumPolyPhases, lSampleIncrement, lNumTaps;
    short sTap;

    lastSampleLeft = 0;
    lastSampleRight = 0;
    pSRC->last_sample_num = 0;
    pSRC->process_num = 160;

    switch ((ulOutputRate << 16) / ulInputRate)
    {
    //32k->16k
    case _32K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_0005.sNumPolyPhases;
        lNumTaps = SRCFilter_0_0005.sNumTaps;
        lSampleIncrement = SRCFilter_0_0005.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_0005.sCoefs;
        break;
    }
    //44.1k->16k
    case _44K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_3628.sNumPolyPhases;
        lNumTaps = SRCFilter_0_3628.sNumTaps;
        lSampleIncrement = SRCFilter_0_3628.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_3628.sCoefs;
        break;
    }
    //48k->16k
    case _48K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_3333.sNumPolyPhases;
        lNumTaps = SRCFilter_0_3333.sNumTaps;
        lSampleIncrement = SRCFilter_0_3333.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_3333.sCoefs;
        break;
    }
    // 8k->16k. 22k->44k
    case _8K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_2_0.sNumPolyPhases;
        lNumTaps = SRCFilter_2_0.sNumTaps;
        lSampleIncrement = SRCFilter_2_0.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_2_0.sCoefs;
        break;
    }

    //22k->16k
    case _22K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_7256.sNumPolyPhases;
        lNumTaps = SRCFilter_0_7256.sNumTaps;
        lSampleIncrement = SRCFilter_0_7256.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_7256.sCoefs;
        break;
    }
    //24k->16k
    case _24K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_6666.sNumPolyPhases;
        lNumTaps = SRCFilter_0_6666.sNumTaps;
        lSampleIncrement = SRCFilter_0_6666.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_6666.sCoefs;
        break;
    }
    //11k->16k
    case _11K_TO_16K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_4512.sNumPolyPhases;
        lNumTaps = SRCFilter_1_4512.sNumTaps;
        lSampleIncrement = SRCFilter_1_4512.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_4512.sCoefs;
        break;
    }
    //8k->44k
    case _8K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_5_5125.sNumPolyPhases;
        lNumTaps = SRCFilter_5_5125.sNumTaps;
        lSampleIncrement = SRCFilter_5_5125.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_5_5125.sCoefs;
        break;
    }
    //11k->44k
    case _11K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_4_0000.sNumPolyPhases;
        lNumTaps = SRCFilter_4_0000.sNumTaps;
        lSampleIncrement = SRCFilter_4_0000.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_4_0000.sCoefs;
        break;
    }
    //16k->44k
    case _16K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_2_7562.sNumPolyPhases;
        lNumTaps = SRCFilter_2_7562.sNumTaps;
        lSampleIncrement = SRCFilter_2_7562.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_2_7562.sCoefs;
        break;
    }
    //32k->44k
    case _32K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_3781.sNumPolyPhases;
        lNumTaps = SRCFilter_1_3781.sNumTaps;
        lSampleIncrement = SRCFilter_1_3781.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_3781.sCoefs;
        break;
    }
    //48k->44k
    case _48K_TO_44K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_0_9187.sNumPolyPhases;
        lNumTaps = SRCFilter_0_9187.sNumTaps;
        lSampleIncrement = SRCFilter_0_9187.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_0_9187.sCoefs;
        break;
    }
    //8k->48k
    case _8K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_6_0000.sNumPolyPhases;
        lNumTaps = SRCFilter_6_0000.sNumTaps;
        lSampleIncrement = SRCFilter_6_0000.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_6_0000.sCoefs;
        break;
    }
    //11k->48k
    case _11K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_4_3537.sNumPolyPhases;
        lNumTaps = SRCFilter_4_3537.sNumTaps;
        lSampleIncrement = SRCFilter_4_3537.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_4_3537.sCoefs;
        break;
    }
    //16k->48k
    case _16K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_3_0000.sNumPolyPhases;
        lNumTaps = SRCFilter_3_0000.sNumTaps;
        lSampleIncrement = SRCFilter_3_0000.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_3_0000.sCoefs;
        break;
    }
    //22k->48k
    case _22K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_2_1768.sNumPolyPhases;
        lNumTaps = SRCFilter_2_1768.sNumTaps;
        lSampleIncrement = SRCFilter_2_1768.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_2_1768.sCoefs;
        break;
    }
    //32k->48k
    case _32K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_5000.sNumPolyPhases;
        lNumTaps = SRCFilter_1_5000.sNumTaps;
        lSampleIncrement = SRCFilter_1_5000.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_5000.sCoefs;
        break;
    }
    //44k->48k
    case _44K_TO_48K:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        lNumPolyPhases = SRCFilter_1_0884.sNumPolyPhases;
        lNumTaps = SRCFilter_1_0884.sNumTaps;
        lSampleIncrement = SRCFilter_1_0884.sSampleIncrement;
        pSRC->psFilter = (short *)SRCFilter_1_0884.sCoefs;
        break;
    }
    default:
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        RK_AUDIO_LOG_D("Not support this rate :%d %lx\n", ulInputRate, (ulOutputRate << 16) / ulInputRate);
        return (0);
    }
    }
    RK_AUDIO_LOG_D("SRCInit:%d, NUMTAPS:%d ", __LINE__, NUMTAPS);
    memset((void *)pSRC->Left_right, 0, NUMTAPS * 4);
    RK_AUDIO_LOG_D("0x%x\n", (int)((ulOutputRate << 16) / ulInputRate));
    RK_AUDIO_LOG_D("ulOutputRate:%ld,ulInputRate:%ld\n", ulOutputRate, ulInputRate);

    //
    // Make sure that the number of taps in the filter matches the number of
    // taps supported by our filtering code.
    //
    NUMTAPS = lNumTaps;
    if (lNumTaps != NUMTAPS)
    {
        RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
        return (0);
    }

    //
    // Initialize the persistent SRC state.
    //
    RK_AUDIO_LOG_D("SRCInit :%d\n", __LINE__);
    pSRC->lFilterOffset = 0;
    pSRC->lFilterIncr = lSampleIncrement * NUMTAPS;//eg:48k->44k,25*13
    pSRC->lFilterSize = lNumPolyPhases * NUMTAPS;//23*13

    //
    // Set the initial state of the delay lines to silence.
    //
    for (sTap = 0; sTap < NUMTAPS; sTap++)
    {
        pSRC->Left_right[2 * sTap] = 0;
        pSRC->Left_right[2 * sTap + 1] = 0;
    }
    RK_AUDIO_LOG_D("SRCInit end\n");

    //
    // We've successfully initialized the requested sample rate converter.
    //
    return (1);
}

//****************************************************************************
//
// SRCFilterStereo runs the sample rate conversion filter over the given
// streams of samples, thereby performing the sample rate conversion requested
// by the initial call to SRCInit.    This function works on a pair of mono
// streams of samples.
//
//****************************************************************************
void SRCFilterStereo(SRCState *pSRC, short *psInDataLeft, short *psInDataRight,
                     short *psOutDataLeft, short *psOutDataRight,
                     long lNumInputSamples, long *plNumOutputSamples)//lNumInputSamples为单声道长度
//plNumOutputSamples为单声道输出
{
    long lOutDataLeft, lOutDataRight;
    short *psPtr1, *psPtr2, *psPtr3;
    short *psSampleLeft, *psSampleRight, sCoeff;
    int iLoop;
    int i;
    int count = 0;
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,NUMTAPS);
    memcpy(psInDataLeft - (NUMTAPS << 1), pSRC->Left_right, (NUMTAPS << 2));
#if 1
#if 1
    i = 1;
    while (1)
    {
        psSampleLeft = psInDataLeft - 2 * i;
        psSampleRight = psInDataRight - 2 * i;
        i++;
        if ((*psSampleLeft == lastSampleLeft) && (*psSampleRight == lastSampleRight))
        {
            break;
        }

    }
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,NUMTAPS);
#endif

    //
    // Compute the number of output samples which we will produce.
    //

    //RK_AUDIO_LOG_D("SRCFilterStereo:%d\n",__LINE__);
    iLoop = ((((lNumInputSamples + i - 1) * pSRC->lFilterSize) -
              pSRC->lFilterOffset - 1) / pSRC->lFilterIncr);

    *plNumOutputSamples = iLoop;

    // Loop through each output sample.
    //
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,NUMTAPS);
    while (iLoop--)
    {
        //
        // Increment the offset into the filter.
        //
        pSRC->lFilterOffset += pSRC->lFilterIncr;

        // See if we've passed the entire filter, indicating that we've
        // consumed one of the input samples.
        //
        while (pSRC->lFilterOffset >= pSRC->lFilterSize)
        {
            //
            // We have, so wrap the filter offset (i.e. treat the filter as if
            // it were a circular buffer in memory).
            //
            pSRC->lFilterOffset -= pSRC->lFilterSize;

            //
            // Consume the input sample.
            //
            //if (flag == 1)
            {
                psSampleLeft += 2;
                psSampleRight += 2;
                //flag = 0;
            }

        }
        //flag = 1;
        //RK_AUDIO_LOG_D("%d\n",pSRC->lFilterOffset);
        //
        // Get pointers to the filter and the two sample data streams.
        //
        psPtr1 = pSRC->psFilter + pSRC->lFilterOffset;
        //RK_AUDIO_LOG_D("pSRC->lFilterOffset = %d\n", pSRC->lFilterOffset);
        count++;

        psPtr2 = psSampleLeft;
        psPtr3 = psSampleRight;

        //
        // Run the filter over the two sample streams.    The number of MACs here
        // must agree with NUMTAPS.
        //
        lOutDataLeft = 0;
        lOutDataRight = 0;
        sCoeff = *psPtr1++;
        i = 0;
        while (i < NUMTAPS)
        {

            lOutDataLeft += sCoeff * *psPtr2;
            lOutDataRight += sCoeff * *psPtr3;
            if (i != (NUMTAPS - 1))
            {
                sCoeff = *psPtr1++;
                psPtr2 -= 2;
                psPtr3 -= 2;

            }
            i++;

        }

        // Clip the filtered samples if necessary.
        //
        if ((lOutDataLeft + 0x40000000) < 0)
        {
            lOutDataLeft = (lOutDataLeft & 0x80000000) ? 0xc0000000 :
                           0x3fffffff;
        }
        if ((lOutDataRight + 0x40000000) < 0)
        {
            lOutDataRight = (lOutDataRight & 0x80000000) ? 0xc0000000 :
                            0x3fffffff;
        }

        //
        // Write out the samples.
        //
        *psOutDataLeft = (short)(lOutDataLeft >> 15);
        psOutDataLeft += 2;
        *psOutDataRight = (short)(lOutDataRight >> 15);
        psOutDataRight += 2;
    }
    //RK_AUDIO_LOG_D("%d SRCFilterStereo, NUMTAPS:%d\n",__LINE__,NUMTAPS);
    //RK_AUDIO_LOG_D("SRCFilterStereo:%d\n",__LINE__);
    //pSRC->lFilterOffset = 0;//cdd 20161012

    // Copy the tail of the filter memory buffer to the beginning, therefore
    // "remembering" the last few samples to use when processing the next batch
    // of samples.
    //
    //if (pSRC->lFilterOffset   == 286)
    lastSampleLeft = *psSampleLeft;
    lastSampleRight = *psSampleRight;
    memcpy(pSRC->Left_right, psInDataLeft + (lNumInputSamples << 1) - (NUMTAPS << 1), (NUMTAPS << 2));
    //RK_AUDIO_LOG_D("SRCFilterStereo:%d\n",__LINE__);
#endif
}

//****************************************************************************
//
// SRCFilter runs the sample rate conversion filter over the given streams of
// samples, thereby performing the sample rate conversion requested
// by the initial call to SRCInit.
//long lNumSamples 双声道长度
// psInLeft指向buffer的26的位置
//****************************************************************************
long SRCFilter(SRCState *pSRC, short *psInLeft, short *psLeft, long lNumSamples)
{
    //lNumSamples 双声道short数据长度
    long lLength;
    long lNumOutputSamples;

    lLength = lNumSamples >> 1;
    SRCFilterStereo(pSRC, psInLeft, &psInLeft[1], &psLeft[0], &psLeft[1],
                    lLength, &lNumOutputSamples);
    return (lNumOutputSamples << 1); //返回双声道长度
}
