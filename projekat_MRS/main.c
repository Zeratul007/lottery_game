/**
 * @file main.c
 * @brief Demo Timer multiplex
 *
 * In this example number 12 is displayed on multiplexed 7 segment LED display
 *
 * @date 2021
 * @author  Marija Bezulj [meja@etf.bg.ac.rs]
 *
 * @version [1.0 - 04/2021] Initial version
 */

#include <msp430.h> 
#include <stdint.h>
#include "function.h"

/**
 * Choose whether to use
 * 0 - DIVMOD
 * 1 - Double Dabble (shift and add 3)
 * 2 - MSP430 DADD - decimal add
 */

/**
 * @brief Timer period
 *
 * Timer is clocked by ACLK (32768Hz)
 * It takes 32768 cycles to count to 1s.
 * If we need a period of X ms, then number of cycles
 * that is written to the CCR0 register is
 * 32768/1000 * X
 */
#define TIMER_PERIOD_MUX        (163)  /* ~5ms (4.97ms)  */
#define TIMER_PERIOD_DEB        (600)
#define TIMER_PERIOD_LSFR        (2500)

#define BR9600      (3)
/**
 * @brief Number to be displayed
 */
//#define NUMBER          (67)

/**
 * @brief array of digits used for display
 */
static volatile uint8_t digits[2] = {0};
static volatile uint8_t indicators[32] = {0};
static volatile uint16_t LSFR_5b;
static volatile int8_t cnt = 0;

/**
 * @brief Function that populates digit array
 * @author Strahinja Jankovic
 */
void display(const uint16_t number)
{
    uint16_t nr = number;
    uint8_t tmp;

    uint8_t data[2] = {0};
    int8_t count;

    for (count = 7; count >= 0; count--)
    {
        uint8_t next = (nr & BIT7) ? 1 : 0;
        nr <<= 1;
        for (tmp = 0; tmp < 2; tmp++)
        {
            uint8_t nibble = data[tmp];
            nibble += (nibble >= 5) ? 3 : 0;
            nibble <<= 1;
            nibble |= next;
            next = (nibble & BIT4) ? 1 : 0;
            data[tmp] = nibble & 0xf;
        }
    }
    for (tmp = 0; tmp < 2; tmp++)
    {
        digits[tmp] = data[tmp];
    }

}

/**
 * @brief Main function
 * Initialize the 7seg display and timer in compare mode.
 * ISR will multiplex the display
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    // sevenseg 1
    P7DIR |= BIT0;              // set P7.0 as out (SEL1)
    P7OUT |= BIT0;             // disable display 1
    // sevenseg 2
    P6DIR |= BIT4;              // set P6.4 as out (SEL2)
    P6OUT |= BIT4;              // disable display 2

    // init button S2
    // S2 (switch 2) init
    P1REN |= BIT5;              // enable pull up/down
    P1OUT |= BIT5;              // set pull up
    P1DIR &= ~BIT5;             // configure P1.5 as in
    P1IES |= BIT5;              // interrupt on falling edge
    P1IFG &= ~BIT5;             // clear flag
    P1IE  |= BIT5;              // enable interrupt
    // Switch S1
    P1DIR &= ~BIT4;             // configure P1.5 as in
    P1REN |= BIT4;              // enable pull up/down
    P1OUT |= BIT4;              // configure pull up
    P1IES |= BIT4;              // falling edge interrupt
    P1IFG &= ~BIT4;             // clear P1.5 flag
    P1IE  |= BIT4;              // interrupt on P1.5 enabled
    // Switch S3
    P1DIR &= ~BIT1;             // configure P1.5 as in
    P1REN |= BIT1;              // enable pull up/down
    P1OUT |= BIT1;              // configure pull up
    P1IES |= BIT1;              // falling edge interrupt
    P1IFG &= ~BIT1;             // clear P1.5 flag
    P1IE  |= BIT1;              // interrupt on P1.5 enabled

    // a,b,c,d,e,f,g

    P2DIR |= BIT6 | BIT3;              // configure P2.3 and P2.6 as out
    P3DIR |= BIT7;                     // configure P3.7 as out
    P4DIR |= BIT3 | BIT0;              // configure P4.0 and P4.3 as out
    P8DIR |= BIT2 | BIT1;              // configure P8.1 and P8.2 as out

    // init TA0 as compare in up mode
    TA0CCR0 = TIMER_PERIOD_MUX;     // set timer period in CCR0 register
    TA0CCTL0 = CCIE;            // enable interrupt for TA10CCR0
    TA0CTL |= TASSEL__ACLK | MC__UP ; //clock select and up mode

    // init TA1 as compare in up mode

    TA1CCR0 = TIMER_PERIOD_LSFR;// set timer period in CCR0 register
    TA1CCTL0 = CCIE;            // enable interrupt for TA10CCR1
    TA1CTL |= TASSEL__ACLK | MC__UP ; //clock select and up mode

    // use timer B0 for debouncing
    TBCCR0 = TIMER_PERIOD_DEB;
    TBCCTL0 = CCIE;             // enable interrupt for CCR0
    TBCTL = TBSSEL__ACLK;       // set TB clock source

    // configure UART
    P4SEL |= BIT4 | BIT5;       // configure P4.4 and P4.5 for uart

    UCA1CTL1 |= UCSWRST;        // enter sw reset

    UCA1CTL0 = 0;
    UCA1CTL1 |= UCSSEL__ACLK;   // select ACLK as clk source
    UCA1BRW = BR9600;           // same as UCA1BR0 = 3; UCA1BR1 = 0
    UCA1MCTL |= UCBRS_3 + UCBRF_0;         // configure 9600 bps

    UCA1CTL1 &= ~UCSWRST;       // leave sw reset
    // after enabling uart enable interrupt
    //UCA1IE |= UCTXIE;           // enable RX interrupt

    __enable_interrupt();

    while(1);
}

/**
 * @brief TA0CCR0 ISR
 *
 * Multiplex the 7seg display. Each ISR activates one digit.
 */
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) A0CCR0ISR (void)
{
    static uint8_t current_digit = 0;

    /* algorithm:
     * - turn off previous display (SEL signal)
     * - set a..g for current display
     * - activate current display
     */

    if (current_digit == 1)
    {
        P6OUT |= BIT4;          // turn off SEL2
        // DAT AIN'T OPTIMAL:
        WriteLed(digits[current_digit]);    // define seg a..g
        P7OUT &= ~BIT0;         // turn on SEL1
    }
    else if (current_digit == 0)
    {
        P7OUT |= BIT0;
        // DAT AIN'T OPTIMAL:
        WriteLed(digits[current_digit]);
        P6OUT &= ~BIT4;
    }
    current_digit = (current_digit + 1) & 0x01;

    return;
}
/*void __attribute__ ((interrupt(PORT1_VECTOR))) P1ISR (void)
{
    if ((P1IFG & (BIT5 | BIT4 | BIT1)) != 0)
    {
        TBCTL |= MC__UP;        // start timer B
        P1IFG &= ~BIT5;         // clear flag
        P1IFG &= ~BIT4;         // clear flag
        P1IFG &= ~BIT1;         // clear flag
    }
}
*/
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) TBCCR0ISR (void)
{
    int8_t i;
    if ((P1IN & BIT5) == 0)     // button2 is pressed
    {
        TBCTL &= ~MC__UP;       // stop debounce timer
        TBCTL |= TBCLR;         // clear debounce timer
        TA1CTL &= ~MC__UP;      // stop LFSR timer
        TA1CTL |= TACLR;         // clear LFSR timer
        cnt++;
        if (cnt < 8){
            indicators[LSFR_5b]  = 1;
            UCA1TXBUF = LSFR_5b;
        }
    }
    else
        if ((P1IN & BIT4) == 0)  // button1 is pressed
        {
            TBCTL &= ~MC__UP;       // stop debounce timer
            TBCTL |= TBCLR;         // clear debounce timer
            if (cnt >= 7){
                digits[1] = 10;
                digits[0] = 11;
                UCA1TXBUF = '\n';
            }
            else
                TA1CTL |= MC__UP;       //start  timer

        }
        else
            if ((P1IN & BIT1) == 0)
            {
                TBCTL &= ~MC__UP;       // stop debounce timer
                TBCTL |= TBCLR;         // clear debounce timer
                TA1CTL |= TACLR;         // clear LFSR timer
                TA1CTL |= MC__UP;       //start  timer
                for(i = 31; i >= 0; i--)
                    indicators[i] = 0;
                cnt = 0;
                UCA1TXBUF = '\n';
            }
}
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) A1CCR0ISR (void)
{
    static uint16_t LSFR = 1;
    while(1){
        LSFR = (LSFR >> 1) ^ ((LSFR ^ (LSFR >> 2) ^ (LSFR >> 3) ^ (LSFR >> 5)) & 1) << 15;
        LSFR_5b = LSFR & 0x001F;
        if (indicators[LSFR_5b] == 0){
            display(LSFR_5b);
            break;
        }
    }
}
