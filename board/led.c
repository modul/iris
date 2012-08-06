/*
 * Blink LED routines.
 *
 */

#include <string.h>
#include "board.h"

#define stoptimer()  TC_Stop(TC1, BLINK_TC)
#define starttimer() TC_Start(TC1, BLINK_TC)

static const Pin leds[] = {PINS_LEDS};
static volatile uint32_t blinks[LEDS_NUM];

// To be called periodically
void blinkhandler(uint32_t ms)
{
	if (ms % BLINK_TIME == 0) {

		uint32_t i;

		for (i=0; i<LEDS_NUM; i++) {
			if (blinks[i] > 0) {
				blinks[i]--; 
				LED_tgl(i);
			}
		}
	}
}

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
	blinks[led] += 2*num;
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
	blinks[led] = 0;
}

/*
 * Check if LED is flashing
 */
uint8_t LED_blinking(uint8_t led)
{
	return (uint8_t) blinks[led];
}

