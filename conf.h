#ifndef _CONF_
#define _CONF_

#include "board.h"

#define TIMER_FREQ 6

#define RESOLUTION 24
#define MAX (1 << RESOLUTION)
#define VREF 1170
#define NUM_AIN 3

/** Configuration Parameters **/
#define PAR_PSET   50
#define PAR_PEAK   2

#define Fchan 0
#define pchan 1
#define schan 2

#define Fgain 6
#define pgain 1
#define sgain 6

#define Fmax VREF-1
#define pmax VREF-1
#define smax VREF-1

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
#define PIN_SPI_CS1  {PIO_PA31A_NPCS1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_CS2  {PIO_PA30B_NPCS2, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_DEFAULT}

#define PIN_AIN_RDY  {PIO_PA11, PIOA, ID_PIOA, PIO_INPUT, PIO_DEFAULT}
#define PINS_SPI PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_SPCK, PIN_SPI_CS1, PIN_SPI_CS2, PIN_AIN_RDY

#define AIN_CS          1
#define AIN_BAUD        1000000
#define AIN_SPICONF     SPI_SCBR(AIN_BAUD, BOARD_MCK)|SPI_CSR_CSAAT|SPI_CSR_BITS_8_BIT
#define MEMORY_CS       2
#define MEMORY_BAUD     1000000
#define MEMORY_SPICONF  SPI_SCBR(MEMORY_BAUD, BOARD_MCK)|SPI_CSR_CSAAT|SPI_CSR_BITS_16_BIT

#endif
