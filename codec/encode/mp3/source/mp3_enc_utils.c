/* utils.c
 *
 * 32 bit fractional multiplication. Requires 64 bit integer support.
 */

#include <stdint.h>

/* Fractional multiply. */
int32_t mul(int32_t x, int32_t y)
{
    return (int32_t)(((int64_t)x * (int64_t)y) >> 32);
}

/* Left justified fractional multiply. */
int32_t muls(int32_t x, int32_t y)
{
    return (int32_t)(((int64_t)x * (int64_t)y) >> 31);
}

/* Fractional multiply with rounding. */
int32_t mulr(int32_t x, int32_t y)
{
    return (int32_t)((((int64_t)x * (int64_t)y) + 0x80000000) >> 32);
}

/* Left justified fractional multiply with rounding. */
int32_t mulsr(int32_t x, int32_t y)
{
    return (int32_t)((((int64_t)x * (int64_t)y) + 0x40000000) >> 31);
}

