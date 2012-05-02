#ifndef _LED_H
#define _LED_H_

#define FOREVER 0xFFFFFFFF

void LEDs_configure();
void LED_set(uint8_t led);
void LED_clr(uint8_t led);
void LED_tgl(uint8_t led);
uint8_t LED_get(uint8_t led);

void blinkhandler(uint32_t ms);
void LED_blink(uint8_t led, uint32_t count);
void LED_blinkwait(uint8_t led, uint32_t count);
void LED_blinkstop(uint8_t led);
uint8_t LED_blinking(uint8_t led);

#endif
