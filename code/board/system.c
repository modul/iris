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
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 * \file
 *
 * Provides the low-level initialization function that called on chip startup.
 */

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "board.h"

/*----------------------------------------------------------------------------
 *        Local definitions
 *----------------------------------------------------------------------------*/
/* Clock settings at 48MHz */
#if (BOARD_MCK == 48000000)
#define BOARD_OSCOUNT   (CKGR_MOR_MOSCXTST(0x8))
#define BOARD_PLLAR     (CKGR_PLLAR_STUCKTO1 \
                       | CKGR_PLLAR_MULA(0x7) \
                       | CKGR_PLLAR_PLLACOUNT(0x1) \
                       | CKGR_PLLAR_DIVA(0x1))
#define BOARD_MCKR      (PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_PLLA_CLK)

/* Clock settings at 64MHz */
#elif (BOARD_MCK == 64000000)
#define BOARD_OSCOUNT   (CKGR_MOR_MOSCXTST(0x8))
#define BOARD_PLLAR     (CKGR_PLLAR_STUCKTO1 \
                       | CKGR_PLLAR_MULA(0x0f) \
                       | CKGR_PLLAR_PLLACOUNT(0x1) \
                       | CKGR_PLLAR_DIVA(0x3))
#define BOARD_MCKR      (PMC_MCKR_PRES_CLK | PMC_MCKR_CSS_PLLA_CLK)

#else
    #error "No settings for current BOARD_MCK."
#endif

/* Define clock timeout */
#define CLOCK_TIMEOUT    0xFFFFFFFF

/*----------------------------------------------------------------------------
 *        Exported functions
 *----------------------------------------------------------------------------*/

volatile uint32_t SystemCoreClock = 4000001;

/*
 * Calculates system frequency from
 * measurement and register settings.
 *
 * This only works with a precise slow clock xtal
 * connected!
 */
void SystemCoreClockUpdate()	
{
#ifndef HAS_SLOWCLK
	SystemCoreClock = BOARD_MCK;
#else
	uint32_t mainf;
	uint32_t sysf;
	uint16_t mul;
	uint8_t div;
	uint8_t prescaler;

	while(!(PMC->CKGR_MCFR & CKGR_MCFR_MAINFRDY));
	/* MAINF holds MAINCLK cycles within 16 slow clock periods:
	 * f = MAINF/(16 * 1/32768 Hz) = (MAINF * 2^15 Hz) / 2^4
	 * f = MAINF * 2^11
	 */
	mainf = (PMC->CKGR_MCFR & CKGR_MCFR_MAINF_Msk) << 11;

	switch(PMC->PMC_MCKR & PMC_MCKR_CSS_Msk) {
		case PMC_MCKR_CSS_SLOW_CLK: // SLOW CLOCK selected
			sysf = 32768; // lucky guess 
			break;
		case PMC_MCKR_CSS_MAIN_CLK: // MAIN CLOCK selected
			sysf = mainf;
			break;
		case PMC_MCKR_CSS_PLLA_CLK: // PLLA CLOCK selected
			mul = ((BOARD_PLLAR & CKGR_PLLAR_MULA_Msk) >> CKGR_PLLAR_MULA_Pos);
			mul = (uint16_t)((PMC->CKGR_PLLAR & CKGR_PLLAR_MULA_Msk) >> CKGR_PLLAR_MULA_Pos);
			div = (uint8_t) (PMC->CKGR_PLLAR & CKGR_PLLAR_DIVA_Msk);
			if (mul == 0) // plla disabled? 
				sysf = mainf;
			else {
				sysf = div == 0 ? 0 : mainf * (mul+1)/div;
				if (PMC->PMC_MCKR & PMC_MCKR_PLLADIV2)
					sysf = sysf >> 1;
			}
			break;
		case PMC_MCKR_CSS_PLLB_CLK: // PLLB CLOCK selected
			mul = (uint16_t)((PMC->CKGR_PLLBR & CKGR_PLLBR_MULB_Msk) >> CKGR_PLLAR_MULA_Pos);
			div = (uint8_t) (PMC->CKGR_PLLBR & CKGR_PLLBR_DIVB_Msk);
			if (mul == 0) // pllb disabled? 
				sysf = mainf;
			else {
				sysf = div == 0 ? 0 : mainf * (mul+1)/div;
				if (PMC->PMC_MCKR & PMC_MCKR_PLLBDIV2)
					sysf = sysf >> 1;
			}
			break;
	}

	prescaler = (PMC->PMC_MCKR & PMC_MCKR_PRES_Msk) >> PMC_MCKR_PRES_Pos;
	SystemCoreClock = sysf >> prescaler;
#endif
}

/**
 * \brief Performs the low-level initialization of the chip.
 * This includes EFC and master clock configuration.
 * It also enable a low level on the pin NRST triggers a user reset.
 */
WEAK void SystemInit( void )
{
    uint32_t timeout = 0;

    /* Set 3 FWS for Embedded Flash Access */
    EFC->EEFC_FMR = EEFC_FMR_FWS(3);

    /* Select external slow clock */
#ifdef HAS_SLOWCLK
    if ((SUPC->SUPC_SR & SUPC_SR_OSCSEL) != SUPC_SR_OSCSEL_CRYST)
    {
        SUPC->SUPC_CR = (uint32_t)(SUPC_CR_XTALSEL_CRYSTAL_SEL | SUPC_CR_KEY(0xA5));
        for ( timeout = 0; !(SUPC->SUPC_SR & SUPC_SR_OSCSEL_CRYST) ; );
    }
#endif

    /* Initialize main oscillator */
    if ( !(PMC->CKGR_MOR & CKGR_MOR_MOSCSEL) )
    {
        PMC->CKGR_MOR = CKGR_MOR_KEY(0x37) | BOARD_OSCOUNT | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN;
        for ( timeout = 0; !(PMC->PMC_SR & PMC_SR_MOSCXTS) && (timeout++ < CLOCK_TIMEOUT); );
    }

    /* Switch to 3-20MHz Xtal oscillator */
    PMC->CKGR_MOR = CKGR_MOR_KEY(0x37) | BOARD_OSCOUNT | CKGR_MOR_MOSCRCEN | CKGR_MOR_MOSCXTEN | CKGR_MOR_MOSCSEL;
    for ( timeout = 0; !(PMC->PMC_SR & PMC_SR_MOSCSELS) && (timeout++ < CLOCK_TIMEOUT); );
    PMC->PMC_MCKR = (PMC->PMC_MCKR & ~(uint32_t)PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
    for ( timeout = 0; !(PMC->PMC_SR & PMC_SR_MCKRDY) && (timeout++ < CLOCK_TIMEOUT) ; );

    /* Initialize PLLA */
    PMC->CKGR_PLLAR = BOARD_PLLAR;
    for (timeout = 0; !(PMC->PMC_SR & PMC_SR_LOCKA) && (timeout++ < CLOCK_TIMEOUT); );

    /* Switch to main clock */
    PMC->PMC_MCKR = (BOARD_MCKR & ~PMC_MCKR_CSS_Msk) | PMC_MCKR_CSS_MAIN_CLK;
    for ( timeout = 0; !(PMC->PMC_SR & PMC_SR_MCKRDY) && (timeout++ < CLOCK_TIMEOUT) ; );

    PMC->PMC_MCKR = BOARD_MCKR ;
    for ( timeout = 0; !(PMC->PMC_SR & PMC_SR_MCKRDY) && (timeout++ < CLOCK_TIMEOUT) ; );

}
