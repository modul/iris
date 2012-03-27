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

#ifndef USBD_HAL_H
#define USBD_HAL_H

/**
 *  \file
 *
 *  This file defines functions for USB Device Hardware Access Level.
 */

/** \addtogroup usbd_hal
 *@{*/

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

/* Introduced in C99 by working group ISO/IEC JTC1/SC22/WG14. */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "USBD.h"
#include "USBDescriptors.h"
#include "USBRequests.h"

/** Indicates chip has an UDP Full Speed. */
#define CHIP_USB_UDP

/** Indicates chip has an internal pull-up. */
#define CHIP_USB_PULLUP_INTERNAL

/** Number of USB endpoints */
#define CHIP_USB_NUMENDPOINTS 8

/** Endpoints max paxcket size */
#define CHIP_USB_ENDPOINTS_MAXPACKETSIZE(i) \
   ((i == 0) ? 64 : \
   ((i == 1) ? 64 : \
   ((i == 2) ? 64 : \
   ((i == 3) ? 64 : \
   ((i == 4) ? 512 : \
   ((i == 5) ? 512 : \
   ((i == 6) ? 64 : \
   ((i == 7) ? 64 : 0 ))))))))

/** Endpoints Number of Bank */
#define CHIP_USB_ENDPOINTS_BANKS(i) \
   ((i == 0) ? 1 : \
   ((i == 1) ? 2 : \
   ((i == 2) ? 2 : \
   ((i == 3) ? 1 : \
   ((i == 4) ? 2 : \
   ((i == 5) ? 2 : \
   ((i == 6) ? 2 : \
   ((i == 7) ? 2 : 0 ))))))))

/**
 *  \section UDP_registers_sec "UDP Register field values"
 *
 *  This section lists the initialize values of UDP registers.
 *
 *  \subsection Values
 *  - UDP_RXDATA
 */
/** Bit mask for both banks of the UDP_CSR register. */
#define UDP_CSR_RXDATA_BK      (UDP_CSR_RX_DATA_BK0 | UDP_CSR_RX_DATA_BK1)

/**
 * \section endpoint_states_sec "UDP Endpoint states"
 *
 *  This page lists the endpoint states.
 *
 *  \subsection States
 *  - UDP_ENDPOINT_DISABLED
 *  - UDP_ENDPOINT_HALTED
 *  - UDP_ENDPOINT_IDLE
 *  - UDP_ENDPOINT_SENDING
 *  - UDP_ENDPOINT_RECEIVING
 *  - UDP_ENDPOINT_SENDINGM
 *  - UDP_ENDPOINT_RECEIVINGM
 */

/**  Endpoint states: Endpoint is disabled */
#define UDP_ENDPOINT_DISABLED       0
/**  Endpoint states: Endpoint is halted (i.e. STALLs every request) */
#define UDP_ENDPOINT_HALTED         1
/**  Endpoint states: Endpoint is idle (i.e. ready for transmission) */
#define UDP_ENDPOINT_IDLE           2
/**  Endpoint states: Endpoint is sending data */
#define UDP_ENDPOINT_SENDING        3
/**  Endpoint states: Endpoint is receiving data */
#define UDP_ENDPOINT_RECEIVING      4
/**  Endpoint states: Endpoint is sending MBL */
#define UDP_ENDPOINT_SENDINGM       5
/**  Endpoint states: Endpoint is receiving MBL */
#define UDP_ENDPOINT_RECEIVINGM     6



/*----------------------------------------------------------------------------
 *        Consts
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *        Macros
 *----------------------------------------------------------------------------*/

/** Get bitmap for an endpoint */
#define bmEP(bEP)   (1 << (bEP))

/*----------------------------------------------------------------------------
 *        Types
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *        Exported functoins
 *----------------------------------------------------------------------------*/

extern void USBD_HAL_Init(void);
extern void USBD_HAL_Connect(void);
extern void USBD_HAL_Disconnect(void);

extern void USBD_HAL_RemoteWakeUp(void);
extern void USBD_HAL_SetConfiguration(uint8_t cfgnum);
extern void USBD_HAL_SetAddress(uint8_t address);
extern uint8_t USBD_HAL_IsHighSpeed(void);

extern void USBD_HAL_Suspend(void);
extern void USBD_HAL_Activate(void);

extern void USBD_HAL_ResetEPs(uint32_t bmEPs,uint8_t bStatus, uint8_t bKeepCfg);
extern void USBD_HAL_CancelIo(uint32_t bmEPs);
extern uint8_t USBD_HAL_ConfigureEP(const USBEndpointDescriptor * pDescriptor);

extern uint8_t USBD_HAL_SetTransferCallback(uint8_t bEP,
                                            TransferCallback fCallback,
                                            void * pCbData);
extern uint8_t USBD_HAL_SetupMblTransfer(uint8_t bEndpoint,
                                         USBDTransferBuffer * pMbList,
                                         uint16_t mblSize,
                                         uint16_t startOffset);
extern uint8_t USBD_HAL_Write(uint8_t bEndpoint,
                              const void * pData,
                              uint32_t dLength);
extern uint8_t USBD_HAL_Read(uint8_t bEndpoint,
                             void * pData,
                             uint32_t dLength);
extern uint8_t USBD_HAL_Stall(uint8_t bEP);
extern uint8_t USBD_HAL_Halt(uint8_t bEndpoint,uint8_t ctl);
/**@}*/

#endif // #define USBD_HAL_H
