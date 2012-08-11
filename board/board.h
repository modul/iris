#ifndef _BOARD_
#define _BOARD_

#include <chip.h>
#include <exceptions.h>

#include "lowlevel/system.h"
#include "lowlevel/syscalls.h" 

#include "timetick.h"
#include "uart_console.h"
#include "usb_console.h"
#include "ad7793.h"

#define LEDS_ACTIVE_LOW
#include "led.h"

/* Name of the board */
#define BOARD_NAME "IRIS"

/* Family definition (already defined) */
#define sam3s
/* Core definition */
#define cortexm3

#define BOARD_MAINOSC 12000000
#define BOARD_MCK     48000000

/* LED pins **/
#define PIN_STATUS {PIO_PA8, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_ALARM {PIO_PA31, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PINS_LEDS PIN_STATUS, PIN_ALARM

/* Blinker configuration (LED Flashing) **/
#define LEDS_NUM 2
#define STATUS 0
#define ALARM 1
#define BLINK_TIME 100

/* USB configuration **/
#define PIN_USB_VBUS    {PIO_PA24, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP}

/* USB attributes configuration descriptor (bus or self powered, remote wakeup) */
#define BOARD_USB_BMATTRIBUTES  USBConfigurationDescriptor_SELFPOWERED_RWAKEUP

/* DBGU configuration (UART console) **/
#define UART_BAUDRATE 115200
#define UART_UART     UART0
#define UART_ID       ID_UART0
#define PINS_UART  { PIO_PA9A_URXD0|PIO_PA10A_UTXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}

/* standard IO configuration for syscalls **/
#define read_stdin   USBC_Gets
#define write_stdout USBC_Puts
#define read_stderr  UART_Gets // For convenience, stderr is used both ways
#define write_stderr UART_Puts
#define TRACE_OUT stderr

/* Valve output pins */
#define PIN_VAL_vent  {PIO_PA2, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_VAL_press {PIO_PA3, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PINS_VAL PIN_VAL_vent, PIN_VAL_press

/* Input pins */
#define PIN_STOP {PIO_PA0, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP}

/* SPI Configuration */
#define PIN_SPI_MISO {PIO_PA12A_MISO, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_SPI_MOSI {PIO_PA13A_MOSI, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_SPCK {PIO_PA14A_SPCK, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define PIN_SPI_CS1  {PIO_PB14A_NPCS1, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_PULLUP}
#define PIN_SPI_CS2  {PIO_PA30B_NPCS2, PIOA, ID_PIOA, PIO_PERIPH_B, PIO_PULLUP}

#define PINS_SPI PIN_SPI_MISO, PIN_SPI_MOSI, PIN_SPI_SPCK, PIN_SPI_CS1, PIN_SPI_CS2

#define ADC_CS          1
#define ADC_BAUD        1000000
#define ADC_SPICONF     SPI_SCBR(ADC_BAUD, BOARD_MCK)|SPI_CSR_CSAAT|SPI_CSR_BITS_8_BIT|SPI_CSR_CPOL

#endif