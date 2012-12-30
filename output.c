#include "conf.h"

void output_press() 
{
	const Pin op = PIN_OUT_pump;
	const Pin ov = PIN_OUT_valv;
	PIO_Clear(&ov); PIO_Set(&op);
}

void output_stop()
{
	const Pin pins[] = {PINS_OUT};
	PIO_Clear(pins);
}

void output_vent()
{
	const Pin op = PIN_OUT_pump;
	const Pin ov = PIN_OUT_valv;
	PIO_Clear(&op); PIO_Set(&ov);
}
