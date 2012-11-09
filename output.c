#include "conf.h"

void output_press() 
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&vn); PIO_Set(&pr);
}

void output_stop()
{
	const Pin pins[] = {PINS_VAL};
	PIO_Clear(pins);
}

void output_vent()
{
	const Pin pr = PIN_VAL_press;
	const Pin vn = PIN_VAL_vent;
	PIO_Clear(&pr); PIO_Set(&vn);
}
