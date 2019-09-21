//****************************************************************************
//
// SRC.H - Contains function prototypes for the routines in the sample rate
//         converter library.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
//#include "../buffer/buffer.h"

//****************************************************************************
//
// The number of taps in the sample rate conversion filters.  This is an
// indication of the number of MACs in the unrolled filter loop.  There is a
// direct correlation between the number of taps and the execution time of
// the sample rate converter.  There is one MAC per tap per output sample.
// This is also the amount of extra memory required before any input data
// buffer passed to the sample rate converter.
//
//****************************************************************************
//#define NUMTAPS 32
static int NUMTAPS = 13;
//****************************************************************************
//
// The perisistent state of the sample rate converter.
//
//****************************************************************************

typedef struct
{
    //
    // The internal, persistent state of the sample rate converter.
    //
    unsigned char ucInternal[12];
    short *psFilter;
    int lFilterIncr;
    int lFilterOffset;
    int lFilterSize;
    //BufferState sInputBuffer;
    //short Left_right[NUMTAPS*2];
    int Left_right[51 * 2];
    int Last_sample[180 * 2];
    int last_sample_num;
    int process_num;
} SRCState;

//****************************************************************************
//
// Function prototypes.
//
//****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

extern int SRCInit(SRCState *pSRC, unsigned long ulInputRate,
                   unsigned long ulOutputRate);
//extern BufferState *SRCGetInputBuffer(SRCState *pSRC);
extern long SRCFilter(SRCState *pSRC, short *psLeft, short *psRight,
                      long lNumSamples);
//extern long SRCNumSamplesAvailable(SRCState *pSRC);

#ifdef __cplusplus
}
#endif /* __cplusplus */
