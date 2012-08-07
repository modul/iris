/*
 * USB CDC Console
 * Inspired by ATMEL USB CDC Serial example.
 *
 * (2012) Remo Giermann
 */

#ifndef _USB_CONSOLE_
#define _USB_CONSOLE_

#include <usb.h>
#include "board.h"

void USBC_configure(void);
int USBC_startListening();

int USBC_hasData();
int USBC_isConfigured(void);

int USBC_Gets(char *ptr, uint16_t len);
int USBC_Puts(char *ptr, uint16_t len);

#endif
