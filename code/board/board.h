#ifndef _BOARD_
#define _BOARD_

#include "chip.h"

#include "system.h"
#include "exceptions.h"
#include "syscalls.h" 
#include "timetick.h"
#include "led.h"
#include "uart_console.h"
#include "usb_console.h"

/** Name of the board */
#define BOARD_NAME "SAM3-H256"
/** Board definition */
#define sam3sek
/** Family definition (already defined) */
#define sam3s
/** Core definition */
#define cortexm3

#define BOARD_MAINOSC 12000000
#define BOARD_MCK     48000000

/** LED pins **/
#define PIN_STATUS {PIO_PA8, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define PIN_ALARM {PIO_PA31, PIOA, ID_PIOA, PIO_OUTPUT_1, PIO_DEFAULT}
#define LEDS_NUM 2
#define statusled 0
#define alarmled 1

#define PINS_LEDS PIN_STATUS, PIN_ALARM

/** Blinker configuration (LED Flashing) **/
#define BLINK_FREQ 6
#define BLINK_TC 0

/** USB configuration **/
#define PIN_USB_VBUS    {PIO_PA24, PIOA, ID_PIOA, PIO_INPUT, PIO_PULLUP}

/** DBGU configuration (UART console) **/
#define PINS_UART  { PIO_PA9A_URXD0|PIO_PA10A_UTXD0, PIOA, ID_PIOA, PIO_PERIPH_A, PIO_DEFAULT}
#define UART_BAUDRATE 115200
#define UART_UART     UART0
#define UART_ID       ID_UART0
#define UART_PINS     {PINS_UART}

/** Fileno, syscalls, standard IO configuration **/
#define read_stdin   USBC_Gets
#define write_stdout USBC_Puts
#define read_stderr  UART_Gets // For convenience, stderr is used both ways
#define write_stderr UART_Puts
#define TRACE_OUT stderr

/** USB attributes configuration descriptor (bus or self powered, remote wakeup) */
#define BOARD_USB_BMATTRIBUTES  USBConfigurationDescriptor_SELFPOWERED_RWAKEUP

#endif
