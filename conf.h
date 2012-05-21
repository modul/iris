#ifndef _CONF_
#define _CONF_

#include "board.h"

#define SAMPLING_FREQ 2
#define TIMER_FREQ (NUM_AIN * SAMPLING_FREQ)

#define RESOLUTION 12
#define MAX (1 << RESOLUTION)
#define VREF 3300

/** ADC input pins **/
/*#define PIN_ADC0 {PIO_PA17X1_AD0, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC1 {PIO_PA18X1_AD1, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PIN_ADC2 {PIO_PA19X1_AD2_WKUP9, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PINS_ADCIN PIN_ADC0, PIN_ADC1, PIN_ADC2
*/
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

/** SPI Configuration **/
#define PIN_SPI_MISO {PIO_PA12A_MISO, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_MOSI {PIO_PA13A_MOSI, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_SPCK {PIO_PA14A_SPCK, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_CS0  {PIO_PA11A_NPCS0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_CS1  {PIO_PA31A_NPCS1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_CS2  {PIO_PA30B_NPCS2, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}
#define PINS_SPI PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_SPCK, PIN_SPI_CS0, PIN_SPI_CS1, PIN_SPI_CS2

#define PGA_CS          1
#define PGA_BAUD        1000000
#define PGA_SPICONF     SPI_SCBR(PGA_BAUD, BOARD_MCK)|SPI_CSR_CSAAT|SPI_CSR_BITS_16_BIT
#define MEMORY_CS       2
#define MEMORY_BAUD     1000000
#define MEMORY_SPICONF  SPI_SCBR(MEMORY_BAUD, BOARD_MCK)|SPI_CSR_CSAAT|SPI_CSR_BITS_16_BIT

/** Configuration Parameters **/
#define PAR_PSET   50
#define PAR_PEAK   2

#define F AIN0
#define p AIN1
#define s AIN2

typedef struct _conf_t {
	uint16_t pmax;
	uint16_t smax;
	uint16_t fmax;
	uint8_t gainid;
} conf_t;

#define CONF_INIT VREF, VREF, VREF, 0

void store_configuration(conf_t *src);
void load_configuration(conf_t *dest);

#endif
