//****************************************************************************
//
// FILT0181.H - Polyphase sample rate conversion filter with a ratio of
//              ~0.1814.
//
// Copyright (c) 1999 Cirrus Logic, Inc.
//
//****************************************************************************
static const struct
{
    short sNumPolyPhases;
    short sSampleIncrement;
    short sNumTaps;
    short sCoefs[1][13];
} SRCFilter_0_0005 =
{
    1,
    2,
    13,
    {
        {-46, 276, -913, 2080, -3600, 4933, 23694, 4933, -3600, 2080, -913, 276, -46 },
    }
};
