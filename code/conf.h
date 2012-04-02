#include "board.h"
#include "controller.h"

/** ADC input pins **/
#define PIN_ADC0 {PIO_PA17X1_AD0, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC1 {PIO_PA18X1_AD1, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PINS_ADCIN PIN_ADC0, PIN_ADC1

#define AIN0 0
#define AIN1 1
#define NUM_AIN 2

/** Time-proportional control output pins (PWM) **/
#define PIN_TPOUT_up   {PIO_PA0A_PWMH0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_TPOUT_down {PIO_PA1A_PWMH1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PINS_TPOUT     PIN_TPOUT_up, PIN_TPOUT_down

#define TPOUT_up 0
#define TPOUT_down 1
#define NUM_TPOUT 2

#define PWM_FREQ   20
#define PWM_PERIOD 100

/** Threepoint control output pins **/
#define PIN_C3OUT_up   {PIO_PA2, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_C3OUT_down {PIO_PA3, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PINS_C3OUT     PIN_C3OUT_up, PIN_C3OUT_down

#define C3OUT_up 0
#define C3OUT_down 1
#define NUM_C3OUT 2

