/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF UARTE, DATA,
 * OR PROFITS; OR BUARTINESS INTERRUPTION) HOWEVER CAUARTED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE UARTE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 *
 * (2012) Modifications by R. Giermann
 *
 */

/**
 * \file
 *
 * Implements UART console (aka DBGU).
 *
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"

#include <stdio.h>
#include <stdint.h>

/*----------------------------------------------------------------------------
 *        Definitions
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *        Variables
 *----------------------------------------------------------------------------*/

/** Is Console Initialized. */
static uint8_t _ucIsConsoleInitialized = 0;
static uint32_t _xferTimeout = 0xFFFFFFFF;

/**
 * \brief Set Timeout for UART blocking I/O.
 *
 */
extern void UART_SetTimeout(uint32_t ms)
{
	_xferTimeout = ms;
}

/**
 * \brief Configures an UART peripheral with the specified parameters.
 *
 * \param baudrate  Baudrate at which the UART should operate (in Hz).
 * \param masterClock  Frequency of the system master clock (in Hz).
 */
extern void UART_Configure( uint32_t baudrate, uint32_t masterClock)
{
    const Pin pPins[] = UART_PINS;
    Uart *pUart = UART_UART;

    /* Configure PIO */
    PIO_Configure(pPins, PIO_LISTSIZE(pPins));

    /* Configure PMC */
    PMC->PMC_PCER0 = 1 << UART_ID;

    /* Reset and disable receiver & transmitter */
    pUart->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX
                   | UART_CR_RXDIS | UART_CR_TXDIS;

    /* Configure mode */
    pUart->UART_MR =  UART_MR_PAR_NO;

    /* Configure baudrate */
    /* Asynchronous, no oversampling */
    pUart->UART_BRGR = (masterClock / baudrate) / 16;

    /* Disable PDC channel */
    pUart->UART_PTCR = UART_PTCR_RXTDIS | UART_PTCR_TXTDIS;

    /* Enable receiver and transmitter */
    pUart->UART_CR = UART_CR_RXEN | UART_CR_TXEN;

    _ucIsConsoleInitialized=1 ;
}

/**
 * \brief Outputs a character on the UART line.
 *
 * \note This function uses polling.
 *       Use UART_SetTimeout() to set a timeout.
 * \param c  Character to send.
 * \return -1 if timed out, 1 otherwise
 */
extern int UART_Put( uint8_t c )
{
    Uart *pUart=UART_UART ;
	uint32_t started;

    if ( !_ucIsConsoleInitialized )
        UART_Configure(UART_BAUDRATE, BOARD_MCK);

    /* Wait for the transmitter to be ready */
	started = GetTickCount();
    while ( (pUart->UART_SR & UART_SR_TXRDY) == 0 ) 
		if ((GetTickCount() - started) > _xferTimeout)
			return -1;

    /* Send character */
    pUart->UART_THR=c ;
	return 1;

}

/**
 * \brief Input a character from the UART line.
 *
 * \note This function uses polling.
 *       Use UART_SetTimeout() to set a timeout.
 * \return character received or -1 if timed out.
 */
extern int UART_Get( void )
{
    Uart *pUart=UART_UART ;
	uint32_t started;

    if ( !_ucIsConsoleInitialized )
        UART_Configure(UART_BAUDRATE, BOARD_MCK);

    /* Wait for the receiver to be ready */
	started = GetTickCount();
	while ( (pUart->UART_SR & UART_SR_RXRDY) == 0 ) 
		if ((GetTickCount() - started) > _xferTimeout) 
			return -1;

    return pUart->UART_RHR ;
}

/**
 * \brief Write 'len' characters from 'ptr'.
 *
 * \return Returns number of characters processed ('len')
 *         or -1 on timeout.
 */
extern int UART_Puts(char *ptr, int len)
{
	int i;
	for (i=0; i<len; i++, ptr++)
			if (UART_Put(*ptr) == -1)
				return -1;
	return i;
}

/**
 * \brief Read 'len' characters to 'ptr'.
 *
 * \return Returns number of characters processed ('len')
 *         or -1 on timeout.
 */
extern int UART_Gets(char *ptr, int len)
{
	int i, j;
	for (i=0; i<len; i++, ptr++) {
			j = UART_Get();
			if (j == -1)
				return -1;
			else 
				*ptr = (char) j;
	}
	return i;
}
