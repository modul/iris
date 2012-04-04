/*
 * Blink LED routines.
 *
 */

#include <string.h>
#include "board.h"

#define stoptimer()  TC_Stop(TC1, BLINK_TC)
#define starttimer() TC_Start(TC1, BLINK_TC)

static uint8_t configured = 0;

static const Pin leds[] = {PINS_LEDS};
static volatile uint32_t blinks[LEDS_NUM];

void TC1_IrqHandler()
{
    volatile uint32_t i;
	uint8_t j = 0;

    // clear status bit to ack interrupt 
   	i = TC1->TC_CHANNEL[BLINK_TC].TC_SR;

	// toggle LEDs
	for (i=0; i<LEDS_NUM; i++) {
		if (blinks[i] > 0) {
			blinks[i]--; 
			LED_tgl(i);
		}
		else
			j++;
	}
	// all counts to zero, stop timer
	if (j == LEDS_NUM)
		stoptimer();
}

static void configure()
{
    uint32_t div;
    uint32_t tcclks;

    /* Enable peripheral clock. */
    PMC->PMC_PCER0 = 1 << ID_TC1;

    /* Configure TC */
    TC_FindMckDivisor(BLINK_FREQ, BOARD_MCK, &div, &tcclks, BOARD_MCK);
    TC_Configure(TC1, BLINK_TC, tcclks | TC_CMR_CPCTRG);
    TC1->TC_CHANNEL[BLINK_TC].TC_RC = (BOARD_MCK/div) / BLINK_FREQ;

    /* Configure and enable interrupt on RC compare */
    NVIC_EnableIRQ((IRQn_Type) ID_TC1);
    TC1->TC_CHANNEL[BLINK_TC].TC_IER = TC_IER_CPCS;

	/* Init flash counts */
	memset((void *)blinks, 0, LEDS_NUM);

	configured = 1;
}

/*-----------------------------------------------------------------------------
 *         Exported Functions
 *----------------------------------------------------------------------------*/

void LEDs_configure()
{
	PIO_Configure(leds, PIO_LISTSIZE(leds));
}

void LED_tgl(uint8_t led)
{
	uint32_t mask;
	Pio *pio;

	assert(led < LEDS_NUM);

	pio  = leds[led].pio;
	mask = leds[led].mask;

	if (pio->PIO_ODSR & mask)
		pio->PIO_CODR = mask;
	else 
		pio->PIO_SODR = mask;
}

void LED_set(uint8_t led)
{
	Pio *pio;
	uint32_t mask;

	assert(led < LEDS_NUM);

	pio  = leds[led].pio;
	mask = leds[led].mask;
	pio->PIO_SODR = mask;
}

void LED_clr(uint8_t led)
{
	Pio *pio;
	uint32_t mask;

	assert(led < LEDS_NUM);

	pio  = leds[led].pio;
	mask = leds[led].mask;
	pio->PIO_CODR = mask;
}

uint8_t LED_get(uint8_t led)
{
	Pio *pio;
	uint32_t mask;

	assert(led < LEDS_NUM);

	pio  = leds[led].pio;
	mask = leds[led].mask;
	return (uint8_t) (pio->PIO_ODSR & mask);
}

/*
 * Flash LED 'num' times (non-blocking).
 */
void LED_blink(uint8_t led, uint32_t num)
{
	if (configured == 0) 
		configure();

	stoptimer();
	blinks[led] += 2*num;
	starttimer();
}

/*
 * Flash LED 'num' times (blocking).
 */
void LED_blinkwait(uint8_t led, uint32_t num)
{
	LED_blink(led, num);
	while(LED_blinking(led));
}

/*
 * Stop flashing.
 */
void LED_blinkstop(uint8_t led)
{
	stoptimer();
	blinks[led] = 0;
	starttimer(); // stops itself when ALL are zero
}

/*
 * Check if LED is flashing
 */
uint8_t LED_blinking(uint8_t led)
{
	return (uint8_t) blinks[led];
}

