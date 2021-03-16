/*
 * Copyright (c) 2021 Fuzhou Rockchip Electronic Co.,Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-03-12     Jair Wu      First version
 *
 */

#ifndef _AUDIO_MAIN_H_
#define _AUDIO_MAIN_H_

#include <stdint.h>
#include <string.h>

/************************************************************
1. Select any Audio CODEC plug-in
*************************************************************/
#ifdef _AMR_DECODE_
#define AMR_DEC_INCLUDE
#endif

#ifdef _MP3_DECODE_
#define MP3_DEC_INCLUDE
#endif

#ifdef _WMA_DECODE_
#define WMA_DEC_INCLUDE
#endif

#ifdef _WAV_DECODE_
#define WAV_DEC_INCLUDE
#endif

#ifdef _APE_DECODE_
#define APE_DEC_INCLUDE
#endif

#ifdef _FLAC_DECODE_
#define FLAC_DEC_INCLUDE
#endif

#ifdef _AAC_DECODE_
#define AAC_DEC_INCLUDE
#endif

#ifdef _HIFI_APE_DEC
#define HIFI_APE_DECODE
#define DISABLE_24BIT_OUT   1
#endif

#ifdef _HIFI_FLAC_DEC
#define HIFI_FlAC_DECODE
#endif

#ifdef _HIFI_ALAC_DEC
#define HIFI_AlAC_DECODE
#endif

#ifdef _OGG_DECODE_
#define OGG_DEC_INCLUDE
#endif

#ifdef _SBC_DECODE_
#define SBC_INCLUDE
#endif

#ifdef _DSDIFF_DECODE_
#define DSDIFF_DEC_INCLUDE
#endif

#ifdef _DSF_DECODE_
#define DSF_DEC_INCLUDE
#endif

#ifdef VIDEO_MP2_DECODE
#define MP2_INCLUDE
#endif

/************************************************************
2. Include MP3 Encoding  or ADPCM Encoding
                   Use Only One Encoding
*************************************************************/
/*------------------------------------------------------------
- Select One of Encoding
------------------------------------------------------------*/
#ifdef _WAV_ENCODE_
#define WAV_ENC_INCLUDE
#endif

#ifdef _AMR_ENCODE_
#define AMR_ENC_INCLUDE
#endif

#ifdef _MP3_ENCODE_
#define MP3_ENC_INCLUDE
#endif


#ifdef _RK_EQ_
#define RK_MP3_EQ_WAIT_SYNTH
#endif


#ifdef RK_MP3_EQ_WAIT_SYNTH
#define MP3_EQ_WAIT_SYNTH  1
#else
#define MP3_EQ_WAIT_SYNTH  0
#endif


typedef enum _MEDIA_MSGBOX_DECODE_CMD
{
    MEDIA_MSGBOX_CMD_DECODE_NULL,

    MEDIA_MSGBOX_CMD_FILE_OPEN,
    MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL,

    MEDIA_MSGBOX_CMD_FILE_CLOSE,
    MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL,

    MEDIA_MSGBOX_CMD_DEC_OPEN,
    MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL,    /*������Ƶ�ļ�ͷ��Ϣ�ɹ�*/

    MEDIA_MSGBOX_CMD_DECODE,
    MEDIA_MSGBOX_CMD_DECODE_CMPL,      /*���һ�ν������*/

    MEDIA_MSGBOX_CMD_DECODE_GETBUFFER,
    MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_GETTIME,
    MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_SEEK,
    MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_CLOSE,
    MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL,

    MEDIA_MSGBOX_CMD_FLAC_SEEKFAST,
    MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_CMPL,

    MEDIA_MSGBOX_CMD_FLAC_GETSEEK_INFO,
    MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_INFO_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_NUM


} MEDIA_MSGBOX_DECODE_CMD;


typedef enum _EFFECT_MSGBOX_CMD
{
    EFFECT_MSGBOX_CMD_OPEN = 200,
    EFFECT_MSGBOX_CMD_OPEN_CMPL,

    EFFECT_MSGBOX_CMD_SET_RESAMPLE,
    EFFECT_MSGBOX_CMD_SET_RESAMPLE_CMPL,


    EFFECT_MSGBOX_CMD_OPEN_SBC_ENCODE,
    EFFECT_MSGBOX_CMD_OPEN_SBC_ENCODE_CMPL,

    EFFECT_MSGBOX_CMD_GET_SRCBUF,
    EFFECT_MSGBOX_CMD_GET_SRCBUF_CMPL,

    EFFECT_MSGBOX_CMD_GET_SBCBUF,
    EFFECT_MSGBOX_CMD_GET_SBCBUF_CMPL,

    EFFECT_MSGBOX_CMD_REQUEST,
    EFFECT_MSGBOX_CMD_RESPOND,

    EFFECT_MSGBOX_CMD_NUM,

} EFFECT_MSGBOX_CMD;



/* sections define */


//------------------------------------------------------------------------------
//Music Section define
#define     _ATTR_AUDIO_TEXT_          __attribute__((section("AudioCode")))
#define     _ATTR_AUDIO_DATA_          __attribute__((section("AudioData")))
#define     _ATTR_AUDIO_BSS_           __attribute__((section("AudioBss"),zero_init))

//-------------------------------------------WMA----------------------------------------------------------
#define     _ATTR_WMADEC_TEXT_          __attribute__((section("WmaCommonCode")))
#define     _ATTR_WMADEC_DATA_          __attribute__((section("WmaCommonData")))
#define     _ATTR_WMADEC_BSS_           __attribute__((section("WmaCommonBss"),zero_init))

//-------------------------------------------SBC----------------------------------------------------------
#define _ATTR_SBCDEC_TEXT_     __attribute__((section("SbcDecCode")))
#define _ATTR_SBCDEC_DATA_     __attribute__((section("SbcDecData")))
#define _ATTR_SBCDEC_BSS_      __attribute__((section("SbcDecBss"),zero_init))

//-------------------------------------Aec algorithm----------------------------------------------------------
#define     _ATTR_AECALG_TEXT_          __attribute__((section("AecAlgCode")))
#define     _ATTR_AECALG_DATA_          __attribute__((section("AecAlgData")))
#define     _ATTR_AECALG_BSS_           __attribute__((section("AecAlgBss"),zero_init))

//-------------------------------------------AMR----------------------------------------------------------
#define _ATTR_AMRDEC_TEXT_     __attribute__((section("AmrDecCode")))
#define _ATTR_AMRDEC_DATA_     __attribute__((section("AmrDecData")))
#define _ATTR_AMRDEC_BSS_      __attribute__((section("AmrDecBss"),zero_init))

#define _ATTR_AMRENC_TEXT_     __attribute__((section("EncodeAmrCode")))
#define _ATTR_AMRENC_DATA_     __attribute__((section("EncodeAmrData")))
#define _ATTR_AMRENC_BSS_      __attribute__((section("EncodeAmrBss"),zero_init))



//-------------------------------------------MP3----------------------------------------------------------
#define _ATTR_MP3DEC_TEXT_     __attribute__((section("Mp3DecCode")))
#define _ATTR_MP3DEC_DATA_     __attribute__((section("Mp3DecData")))
#define _ATTR_MP3DEC_BSS_      __attribute__((section("Mp3DecBss"),zero_init))

#define _ATTR_MP3ENC_TEXT_     __attribute__((section("EncodeMP3Code")))
#define _ATTR_MP3ENC_DATA_     __attribute__((section("EncodeMP3Data")))
#define _ATTR_MP3ENC_BSS_      __attribute__((section("EncodeMP3Bss"),zero_init))


//-------------------------------------------WAV----------------------------------------------------------
#define _ATTR_WAVDEC_TEXT_     __attribute__((section("WavDecCode")))
#define _ATTR_WAVDEC_DATA_     __attribute__((section("WavDecData")))
#define _ATTR_WAVDEC_BSS_      __attribute__((section("WavDecBss"),zero_init))

#define _ATTR_WAVENC_TEXT_     __attribute__((section("WavEncCode")))
#define _ATTR_WAVENC_DATA_     __attribute__((section("WavEncData")))
#define _ATTR_WAVENC_BSS_      __attribute__((section("WavEncBss"),zero_init))


//-----------------------------------------FLAC----------------------------------------------------------
#define     _ATTR_FLACDEC_TEXT_          __attribute__((section("FlacDecCode")))
#define     _ATTR_FLACDEC_DATA_          __attribute__((section("FlacDecData")))
#define     _ATTR_FLACDEC_BSS_           __attribute__((section("FlacDecBss"),zero_init))
//-----------------------------------------AAC----------------------------------------------------------
#define     _ATTR_AACDEC_TEXT_          __attribute__((section("AacDecCode")))
#define     _ATTR_AACDEC_DATA_          __attribute__((section("AacDecData")))
#define     _ATTR_AACDEC_BSS_           __attribute__((section("AacDecBss"),zero_init))
#define     _ATTR_AACDEC_DATA_RO        __attribute__((section("AacROData")))

//-----------------------------------------APE----------------------------------------------------------
#define     _ATTR_APEDEC_TEXT_          __attribute__((section("ApeDecCode")))
#define     _ATTR_APEDEC_DATA_          __attribute__((section("ApeDecData")))
#define     _ATTR_APEDEC_BSS_           __attribute__((section("ApeDecBss"),zero_init))

//-----------------------------------------OGG----------------------------------------------------------
#define     _ATTR_OGGDEC_TEXT_          __attribute__((section("OggDecCode")))
#define     _ATTR_OGGDEC_DATA_          __attribute__((section("OggDecData")))
#define     _ATTR_OGGDEC_BSS_           __attribute__((section("OggDecBss"),zero_init))


//-----------------------------------------HIFI APE----------------------------------------------------------
#define     _ATTR_HIFI_APEDEC_TEXT_          __attribute__((section("ApeHDecCode")))
#define     _ATTR_HIFI_APEDEC_DATA_          __attribute__((section("ApeHDecData")))
#define     _ATTR_HIFI_APEDEC_BSS_           __attribute__((section("ApeHDecBss"),zero_init))


//-----------------------------------------HIFI FLAC----------------------------------------------------------
#define     _ATTR_HIFI_FLACDEC_TEXT_          __attribute__((section("FlacHDecCode")))
#define     _ATTR_HIFI_FLACDEC_DATA_          __attribute__((section("FlacHDecData")))
#define     _ATTR_HIFI_FLACDEC_BSS_           __attribute__((section("FlacHDecBss"),zero_init))


//-----------------------------------------HIFI ALAC----------------------------------------------------------
#define     _ATTR_HIFI_ALACDEC_TEXT_          __attribute__((section("AlacHDecCode")))
#define     _ATTR_HIFI_ALACDEC_DATA_          __attribute__((section("AlacHDecData")))
#define     _ATTR_HIFI_ALACDEC_BSS_           __attribute__((section("AlacHDecBss"),zero_init))

//-----------------------------------------DSDIFF----------------------------------------------------------
#define     _ATTR_DSDIFFDEC_TEXT_          __attribute__((section("DsdiffDecCode")))
#define     _ATTR_DSDIFFDEC_DATA_          __attribute__((section("DsdiffDecData")))
#define     _ATTR_DSDIFFDEC_BSS_           __attribute__((section("DsdiffDecBss"),zero_init))

//-----------------------------------------DSF----------------------------------------------------------
#define     _ATTR_DSFDEC_TEXT_          __attribute__((section("DsfDecCode")))
#define     _ATTR_DSFDEC_DATA_          __attribute__((section("DsfDecData")))
#define     _ATTR_DSFDEC_BSS_           __attribute__((section("DsfDecBss"),zero_init))

//-----------for B core-----------
//-------------------------------------------WMA BIN----------------------------------------------------------
#define     _ATTR_WMADEC_BIN_TEXT_          __attribute__((used, section("WmaCommonBinCode")))
#define     _ATTR_WMADEC_BIN_DATA_          __attribute__((used, section("WmaCommonBinData")))
#define     _ATTR_WMADEC_BIN_BSS_           __attribute__((used, section("WmaCommonBinBss"),zero_init))

//-------------------------------------------SBC BIN----------------------------------------------------------
#define _ATTR_SBCDEC_BIN_TEXT_     __attribute__((used, section("SbcDecBinCode")))
#define _ATTR_SBCDEC_BIN_DATA_     __attribute__((used, section("SbcDecBinData")))
#define _ATTR_SBCDEC_BIN_BSS_      __attribute__((used, section("SbcDecBinBss"),zero_init))


//-------------------------------------------MP3 BIN----------------------------------------------------------
#define _ATTR_MP3DEC_BIN_TEXT_     __attribute__((used, section("Mp3DecBinCode")))
#define _ATTR_MP3DEC_BIN_DATA_     __attribute__((used, section("Mp3DecBinData")))
#define _ATTR_MP3DEC_BIN_BSS_      __attribute__((used, section("Mp3DecBinBss"),zero_init))

#define _ATTR_MP3ENC_BIN_TEXT_     __attribute__((used, section("Mp3EncBinCode")))
#define _ATTR_MP3ENC_BIN_DATA_     __attribute__((used, section("Mp3EncBinData")))
#define _ATTR_MP3ENC_BIN_BSS_      __attribute__((used, section("Mp3EncBinBss"),zero_init))

//-------------------------------------------WAV BIN----------------------------------------------------------
#define _ATTR_WAVDEC_BIN_TEXT_     __attribute__((used, section("WavDecBinCode")))
#define _ATTR_WAVDEC_BIN_DATA_     __attribute__((used, section("WavDecBinData")))
#define _ATTR_WAVDEC_BIN_BSS_      __attribute__((used, section("WavDecBinBss"),zero_init))

#define _ATTR_WAVENC_BIN_TEXT_     __attribute__((used, section("WavEncBinCode")))
#define _ATTR_WAVENC_BIN_DATA_     __attribute__((used, section("WavEncBinData")))
#define _ATTR_WAVENC_BIN_BSS_      __attribute__((used, section("WavEncBinBss"),zero_init))

//-------------------------------------------AMR BIN----------------------------------------------------------
#define _ATTR_AMRDEC_BIN_TEXT_     __attribute__((used, section("AmrDecBinCode")))
#define _ATTR_AMRDEC_BIN_DATA_     __attribute__((used, section("AmrDecBinData")))
#define _ATTR_AMRDEC_BIN_BSS_      __attribute__((used, section("AmrDecBinBss"),zero_init))

#define _ATTR_AMRENC_BIN_TEXT_     __attribute__((used, section("AmrEncBinCode")))
#define _ATTR_AMRENC_BIN_DATA_     __attribute__((used, section("AmrEncBinData")))
#define _ATTR_AMRENC_BIN_BSS_      __attribute__((used, section("AmrEncBinBss"),zero_init))

//-----------------------------------------FLAC BIN----------------------------------------------------------
#define     _ATTR_FLACDEC_BIN_TEXT_          __attribute__((used, section("FlacDecBinCode")))
#define     _ATTR_FLACDEC_BIN_DATA_          __attribute__((used, section("FlacDecBinData")))
#define     _ATTR_FLACDEC_BIN_BSS_           __attribute__((used, section("FlacDecBinBss"),zero_init))


//-----------------------------------------AAC BIN----------------------------------------------------------
#define     _ATTR_AACDEC_BIN_TEXT_          __attribute__((used, section("AacDecBinCode")))
#define     _ATTR_AACDEC_BIN_DATA_          __attribute__((used, section("AacDecBinData")))
#define     _ATTR_AACDEC_BIN_BSS_           __attribute__((used, section("AacDecBinBss"),zero_init))


//-----------------------------------------APE BIN----------------------------------------------------------
#define     _ATTR_APEDEC_BIN_TEXT_          //__attribute__((used, section("ApeDecBinCode")))
#define     _ATTR_APEDEC_BIN_DATA_          //__attribute__((used, section("ApeDecBinData")))
#define     _ATTR_APEDEC_BIN_BSS_           //__attribute__((used, section("ApeDecBinBss"),zero_init))

//-----------------------------------------OGG BIN----------------------------------------------------------
#define     _ATTR_OGGDEC_BIN_TEXT_          __attribute__((used,section("OggDecBinCode")))
#define     _ATTR_OGGDEC_BIN_DATA_          __attribute__((used,section("OggDecBinData")))
#define     _ATTR_OGGDEC_BIN_BSS_           __attribute__((used,section("OggDecBinBss"),zero_init))


//-----------------------------------------HIFI APE BIN----------------------------------------------------------
#define     _ATTR_HIFI_APEDEC_BIN_TEXT_          __attribute__((used,section("ApeHDecBinCode")))
#define     _ATTR_HIFI_APEDEC_BIN_DATA_          __attribute__((used,section("ApeHDecBinData")))
#define     _ATTR_HIFI_APEDEC_BIN_BSS_           __attribute__((used,section("ApeHDecBinBss"),zero_init))


//-----------------------------------------HIFI FLAC BIN----------------------------------------------------------
#define     _ATTR_HIFI_FLACDEC_BIN_TEXT_          __attribute__((used,section("FlacHDecBinCode")))
#define     _ATTR_HIFI_FLACDEC_BIN_DATA_          __attribute__((used,section("FlacHDecBinData")))
#define     _ATTR_HIFI_FLACDEC_BIN_BSS_           __attribute__((used,section("FlacHDecBinBss"),zero_init))


//-----------------------------------------HIFI ALAC BIN----------------------------------------------------------
#define     _ATTR_HIFI_ALACDEC_BIN_TEXT_          __attribute__((used,section("AlacHDecBinCode")))
#define     _ATTR_HIFI_ALACDEC_BIN_DATA_          __attribute__((used,section("AlacHDecBinData")))
#define     _ATTR_HIFI_ALACDEC_BIN_BSS_           __attribute__((used,section("AlacHDecBinBss"),zero_init))

//-----------------------------------------DSDIFF BIN----------------------------------------------------------
#define     _ATTR_DSDIFFDEC_BIN_TEXT_           __attribute__((used,section("DsdiffDecBinCode")))
#define     _ATTR_DSDIFFDEC_BIN_DATA_           __attribute__((used,section("DsdiffDecBinData")))
#define     _ATTR_DSDIFFDEC_BIN_BSS_            __attribute__((used,section("DsdiffDecBinBss"),zero_init))

//-----------------------------------------DSF BIN----------------------------------------------------------
#define     _ATTR_DSFDEC_BIN_TEXT_           __attribute__((used,section("DsfDecBinCode")))
#define     _ATTR_DSFDEC_BIN_DATA_           __attribute__((used,section("DsfDecBinData")))
#define     _ATTR_DSFDEC_BIN_BSS_            __attribute__((used,section("DsfDecBinBss"),zero_init))

//-------------------------------------------ID3----------------------------------------------------------
#define _ATTR_ID3_TEXT_     __attribute__((section("Id3Code")))
#define _ATTR_ID3_DATA_     __attribute__((section("Id3Data")))
#define _ATTR_ID3_BSS_      __attribute__((section("Id3Bss"),zero_init))

//-------------------------------------------ID3 JPG------------------------------------------------------
//#define _ATTR_ID3JPG_TEXT_     __attribute__((section("Id3JpgCode")))
//#define _ATTR_ID3JPG_DATA_     __attribute__((section("Id3JpgData")))
//#define _ATTR_ID3JPG_BSS_      __attribute__((section("Id3JpgBss"),zero_init))


//-------------------------------------------MP3 Encode----------------------------------------------------------
#define _ATTR_MSEQ_TEXT_     __attribute__((section("MsEqCode")))
#define _ATTR_MSEQ_DATA_     __attribute__((section("MsEqData")))
#define _ATTR_MSEQ_BSS_      __attribute__((section("MsEqBss"),zero_init))

//-------------------------------------------MP2 Encode----------------------------------------------------------
#define _ATTR_MP2DEC_TEXT_     __attribute__((section("Mp2Code"/*"MP3DEC_CODE_SEG"*/)))
#define _ATTR_MP2DEC_DATA_     __attribute__((section("Mp2Data"/*"MP3DEC_DATA_SEG"*/)))
#define _ATTR_MP2DEC_BSS_      __attribute__((section("Mp2Bss"/*"MP3DEC_BSS_SEG"*/),zero_init))
//-------------------------------------------OGG Decode----------------------------------------------------------




//aac
#ifdef B_CORE_DECODE
#define aac_DEBUG  DEBUG2
#define aac_printf printf2
#define aac_delayms DelayMs2

#define aac_MemSet MemSet2
#define aac_Memcpy MemCpy2
#else
#define aac_DEBUG
#define aac_printf printf
#define aac_delayms DelayMs

#define aac_MemSet memset
#define aac_Memcpy memcpy
#endif

//hifi ape
#ifdef B_CORE_DECODE
#define Hifi_Ape_MemSet MemSet2
#define Hifi_Ape_DelayMs DelayMs2
#define Hifi_Ape_Memcpy  MemCpy2

#define Hifi_Ape_Printf printf2
#else
#define Hifi_Ape_MemSet memset
#define Hifi_Ape_DelayMs DelayMs
#define Hifi_Ape_Memcpy  memcpy

#define Hifi_Ape_Printf printf
#endif

//hifi alac
#ifdef B_CORE_DECODE
#define Hifi_Alac_MemSet MemSet2
#define Hifi_Alac_DelayMs DelayMs2
#define Hifi_Alac_Memcpy  MemCpy2
#define Hifi_Alac_Printf printf2
#else
#define Hifi_Alac_MemSet memset
#define Hifi_Alac_DelayMs DelayMs
#define Hifi_Alac_Memcpy  memcpy
#define Hifi_Alac_Printf printf
#endif

//hifi flac
#ifdef B_CORE_DECODE
#define Hifi_Flac_MemSet MemSet2
#define Hifi_Flac_DelayMs DelayMs2
#define Hifi_Flac_Memcpy  MemCpy2
#define Hifi_Flac_Printf printf2
#else
#define Hifi_Flac_MemSet memset
#define Hifi_Flac_DelayMs DelayMs
#define Hifi_Flac_Memcpy  memcpy
#define Hifi_Flac_Printf printf
#endif

//mp3
#ifdef B_CORE_DECODE
#ifdef _MP3_LOG_
#define mp3_printf printf2
#else
#define mp3_printf
#endif
#else
#ifdef _MP3_LOG_
#define mp3_printf printf
#else
#define mp3_printf
#endif
#endif
//wav
#ifdef B_CORE_DECODE
#define wav_DEBUG DEBUG2
#define wav_printf printf2
#else
#define wav_DEBUG DEBUG2
#define wav_printf printf
#endif
//wma
#ifdef B_CORE_DECODE
#define WMA_DEBUG DEBUG2
#else
#define WMA_DEBUG
#endif

//flac
#ifdef B_CORE_DECODE
#define flac_MemSet MemSet2
#define flac_DEBUG DEBUG2
#else
#define flac_MemSet memset
#define flac_DEBUG
#endif

//ape
#ifdef B_CORE_DECODE
#define ape_MemSet MemSet2
#define ape_DEBUG DEBUG2
#define ape_Memcpy MemCpy2
#else
#define ape_MemSet memset
#define ape_DEBUG
#define ape_Memcpy memcpy
#endif

//ogg
#ifdef B_CORE_DECODE
#define ogg_MemSet MemSet2
#define ogg_DEBUG DEBUG2
#define ogg_Memcpy MemCpy2
#else
#define ogg_MemSet memset
#define ogg_DEBUG
#define ogg_Memcpy memcpy
#endif

//dsdiff
#ifdef B_CORE_DECODE
#define dsd_DEBUG DEBUG2
#define dsd_printf printf2
#define DSD_Memcpy  MemCpy2
#else
#define dsd_DEBUG DEBUG2
#define dsd_printf printf
#define DSD_Memcpy  memcpy
#endif

//dsf
#ifdef B_CORE_DECODE
#define dsf_DEBUG DEBUG2
#define dsf_printf printf2
#define DSF_Memcpy  MemCpy2
#else
#define dsf_DEBUG
#define dsf_printf printf
#define DSF_Memcpy  memcpy
#endif

//amr
#ifdef B_CORE_DECODE
//#define amr_DEBUG DEBUG2
//#define amr_printf printf2
//#define amr_Memcpy  MemCpy2
#define amr_DEBUG(format,...)
#define amr_printf(format,...)
#define amr_Memcpy(format,...)
#else
#define amr_DEBUG
#define amr_printf printf
#define amr_Memcpy  memcpy
#endif

#endif        // _AUDIO_MAIN_H_
