#include <stdio.h>

#define DCB_FILTER 1
#if DCB_FILTER

#define A1 32511 // (1-2^(-7))     Q32:1.31 // 32752=>0.99951171875

#define TO_16BIT_SHIFT 15
#define MAX_Uint_PCMBIT_SIZE 4294967296
#define MAX_UNSIGN_PCMBIT_SIZE 65536
#define MAX_SIGN_POS_PCMBIT_SIZE 32768
#define MAX_SIGN_NEG_PCMBIT_SIZE -32768

/* static variables for previous values */
/* left 1 */
static short x_prev_l1 = 0;
static short   y_prev_l1 = 0;
/* right 1 */
static short x_prev_r1 = 0;
static short   y_prev_r1 = 0;
/* left 2 */
static short x_prev_l2 = 0;
static short   y_prev_l2 = 0;
/* right 1 */
static short x_prev_r2 = 0;
static short   y_prev_r2 = 0;
/* left 1 */
static short x_prev_l3 = 0;
static short   y_prev_l3 = 0;
/* right 1 */
static short x_prev_r3 = 0;
static short   y_prev_r3 = 0;

void dc_filter_left1(short *pcmIn)
{
    short sampleIn, delta_x, sampleOut;
    short   a1_y_prev;

    sampleIn = *pcmIn;
    delta_x = sampleIn - x_prev_l1;
    a1_y_prev = A1 * y_prev_l1 / MAX_SIGN_POS_PCMBIT_SIZE;
    sampleOut = delta_x + a1_y_prev;

    x_prev_l1 = sampleIn;
    y_prev_l1 = sampleOut;

    *pcmIn = sampleOut;
}

void dc_filter_right1(short *pcmIn)
{
    short sampleIn, delta_x, sampleOut;
    short a1_y_prev;

    sampleIn = *pcmIn;
    delta_x = sampleIn - x_prev_r1;
    a1_y_prev = A1 * y_prev_r1 / MAX_SIGN_POS_PCMBIT_SIZE;
    sampleOut = delta_x + a1_y_prev;

    x_prev_r1 = sampleIn;
    y_prev_r1 = sampleOut;

    *pcmIn = sampleOut;
}

void dc_filter_left2(short *pcmIn)
{
    short sampleIn, delta_x, sampleOut;
    short a1_y_prev;

    sampleIn = *pcmIn;
    delta_x = sampleIn - x_prev_l2;
    a1_y_prev = A1 * y_prev_l2 / MAX_SIGN_POS_PCMBIT_SIZE;
    sampleOut = delta_x + a1_y_prev;

    x_prev_l2 = sampleIn;
    y_prev_l2 = sampleOut;

    *pcmIn = sampleOut;
}

void dc_filter_right2(short *pcmIn)
{
    short sampleIn, delta_x, sampleOut;
    short a1_y_prev;

    sampleIn = *pcmIn;
    delta_x = sampleIn - x_prev_r2;
    a1_y_prev = A1 * y_prev_r2 / MAX_SIGN_POS_PCMBIT_SIZE;
    sampleOut = delta_x + a1_y_prev;

    x_prev_r2 = sampleIn;
    y_prev_r2 = sampleOut;

    *pcmIn = sampleOut;
}

void dc_filter_left3(short *pcmIn)
{
    short sampleIn, delta_x, sampleOut;
    short a1_y_prev;

    sampleIn = *pcmIn;
    delta_x = sampleIn - x_prev_l3;
    a1_y_prev = A1 * y_prev_l3 / MAX_SIGN_POS_PCMBIT_SIZE;
    sampleOut = delta_x + a1_y_prev;

    x_prev_l3 = sampleIn;
    y_prev_l3 = (short)sampleOut;

    *pcmIn = sampleOut;
}

void dc_filter_right3(short *pcmIn)
{
    short sampleIn, delta_x, sampleOut;
    short a1_y_prev;

    sampleIn = *pcmIn;
    delta_x = sampleIn - x_prev_r3;
    a1_y_prev = A1 * y_prev_r3 / MAX_SIGN_POS_PCMBIT_SIZE;
    sampleOut = delta_x + a1_y_prev;

    x_prev_r3 = sampleIn;
    y_prev_r3 = (short)sampleOut;

    *pcmIn = sampleOut;
}


void DCB_Doing(void *pIn, int length, int channels)
{
    int i = 0;
    short *pInBuf  = (short *)pIn;

    //printf("vicent-DCB_Doing---------------length = %d\n",length);

    for (i = 0; i < length; i ++)
    {

        int curChannel = i % channels;

        switch (curChannel)
        {
        case 0:
            dc_filter_left1(pInBuf);
            break;
        case 1:
            dc_filter_right1(pInBuf);
            break;
        case 2:
            dc_filter_left2(pInBuf);
            break;
        case 3:
            dc_filter_right2(pInBuf);
            break;
        case 4:
            dc_filter_left3(pInBuf);
            break;
        case 5:
            dc_filter_right3(pInBuf);
            break;
        default:
            break;
        }
        pInBuf++;
    }
}
#endif


