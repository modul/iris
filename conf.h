#ifndef _CONF_
#define _CONF_

#include "board.h"

#define SAMPLING_FREQ 2
#define TIMER_FREQ (NUM_AIN * SAMPLING_FREQ)

#define RESOLUTION 12
#define MAX (1 << RESOLUTION)
#define VREF 3300

/** ADC input pins **/
#define PIN_ADC0 {PIO_PA17X1_AD0, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC1 {PIO_PA18X1_AD1, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC2 {PIO_PA19X1_AD2_WKUP9, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PINS_ADCIN PIN_ADC0, PIN_ADC1, PIN_ADC2

#define AIN0 0
#define AIN1 1
#define AIN2 2
#define NUM_AIN 3

/** Valve output pins **/
#define PIN_VAL_vent  {PIO_PA2, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_VAL_press {PIO_PA3, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PINS_VAL PIN_VAL_vent, PIN_VAL_press
#define VAL_vent  0
#define VAL_press 1

/** Parameters **/
#define PAR_PSET 50
#define PAR_PMAX VREF-1 // VREF-1 is maximum, but these should be set by the user
#define PAR_SMAX VREF
#define PAR_FMAX VREF

typedef struct _conf_t {
	uint16_t pset;
	uint16_t pmax;
	uint16_t smax;
	uint16_t fmax;
	uint8_t fpeakdiv;
	uint8_t F;
	uint8_t P;
	uint8_t S;
} conf_t;

#define CONF_INIT PAR_PSET, PAR_PMAX, PAR_SMAX, PAR_FMAX, 2, AIN0, AIN1, AIN2 

void store_configuration(conf_t *src);
void load_configuration(conf_t *dest);

#endif
