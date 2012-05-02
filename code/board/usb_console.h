/*
 * USB CDC Console
 * Based on ATMEL USB CDC Serial example.
 *
 * (2012) Remo Giermann
 */

#ifndef _USB_CONSOLE_
#define _USB_CONSOLE_

#include <usb.h>
#include "board.h"

void USBC_Configure(void);
uint8_t USBC_isConfigured(void);
uint8_t USBC_ReadBuffer(void *buffer, uint32_t size, TransferCallback callback, void *args);
uint8_t USBC_WriteBuffer(void *buffer, uint32_t size, TransferCallback callback, void *args);
int USBC_Gets(char *ptr, uint16_t len);
int USBC_Puts(char *ptr, uint16_t len);

uint8_t USBC_StartListening();
uint8_t USBC_hasData();

#endif
