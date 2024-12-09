/**
 * @file function.c
 * @brief Implementation of function used to write to 7seg display
 *
 * @date 2017
 * @author Strahinja Jankovic (jankovics@etf.bg.ac.rs)
 * @author Marija Bezulj (meja@etf.bg.ac.rs)
 *
 * @version [2.0 - 04/2021] Hardware change
 * @version [1.0 - 03/2017] Initial version
 */

#include <msp430.h>
#include "function.h"

/**
 * Table of settings for a-g lines for appropriate digit
 */
const unsigned int segtab2[] = {
        0x48,
        0x40,
        0x08,
        0x40,
        0x40,
        0x40,
        0x48,
        0x40,
        0x48,
        0x40,
        0x08,
        0x48
};

const unsigned int segtab3[] = {
        0x80,
        0x00,
        0x80,
        0x80,
        0x00,
        0x80,
        0x80,
        0x80,
        0x80,
        0x80,
        0x80,
        0x00
};

const unsigned int segtab4[] = {
        0x09,
        0x08,
        0x08,
        0x08,
        0x09,
        0x01,
        0x01,
        0x08,
        0x09,
        0x09,
        0x01,
        0x00
};

const unsigned int segtab8[] = {
        0x02,
        0x00,
        0x06,
        0x06,
        0x04,
        0x06,
        0x06,
        0x00,
        0x06,
        0x06,
        0x06,
        0x04
};

void WriteLed(unsigned int digit)
{
    P2OUT |= 0x48;
    P2OUT &= ~segtab2[digit];

    P3OUT |= 0x80;
    P3OUT &= ~segtab3[digit];

    P4OUT |= 0x09;
    P4OUT &= ~segtab4[digit];

    P8OUT |= 0x06;
    P8OUT &= ~segtab8[digit];
}
