/*
 * USB CDC Serial Console
 * Based on ATMEL's USB CDC Serial example.
 *
 * (2012) Remo Giermann
 */

/*----------------------------------------------------------------------------
 *         Headers
 *----------------------------------------------------------------------------*/

#include <string.h>

#include <usb.h>
#include "board.h"

extern const USBDDriverDescriptors cdcdSerialDriverDescriptors;

/*----------------------------------------------------------------------------
 *         Configuration
 *----------------------------------------------------------------------------*/

/** Parameters **/

static volatile uint32_t _rxCount = 0;
static volatile uint32_t _txCount = 0;
static volatile uint8_t _cfgdone = 0;

/** Receive Buffer **/

#define RXBUFFERSIZE 256
static char _rxBuffer[RXBUFFERSIZE];

/** VBus pin instance. */
static const Pin pinVbus = PIN_USB_VBUS;

/**
 * Handles interrupts coming from PIO controllers.
 */
static void ISR_Vbus(const Pin *pPin)
{
    /* Check current level on VBus */
    if (PIO_Get(&pinVbus)) {

        TRACE_INFO("USB VBUS connected\n");
        USBD_Connect();
    }
    else {

        TRACE_INFO("USB VBUS disconnected\n");
        USBD_Disconnect();
    }
}

/**
 * Configures the VBus pin to trigger an interrupt when the level on that pin
 * changes.
 */
static void VBus_Configure( void )
{

    /* Configure PIO */
    PIO_Configure(&pinVbus, 1);
    PIO_ConfigureIt(&pinVbus, ISR_Vbus);
    PIO_EnableIt(&pinVbus);

    /* Check current level on VBus */
    if (PIO_Get(&pinVbus)) {

        /* if VBUS present, force the connect */
        USBD_Connect();
    }
    else {
        USBD_Disconnect();
    }
}

/**
 * Configure 48MHz Clock for USB
 */
static void ConfigureUsbClock(void)
{
    /* Enable PLLB for USB */
    PMC->CKGR_PLLBR = CKGR_PLLBR_DIVB(1)
                    | CKGR_PLLBR_MULB(7)
                    | CKGR_PLLBR_PLLBCOUNT_Msk;
    while((PMC->PMC_SR & PMC_SR_LOCKB) == 0);
    /* USB Clock uses PLLB */
    PMC->PMC_USB = PMC_USB_USBDIV(1)       /* /2   */
                 | PMC_USB_USBS;           /* PLLB */
}

/** 
 * Callback re-implementation
 * Invoked after the USB driver has been initialized. By default, configures
 * the UDP/UDPHS interrupt.
 */
void USBDCallbacks_Initialized(void)
{
    NVIC_EnableIRQ(UDP_IRQn);
}

/** 
 * Callback re-implementation
 * Invoked when the configuration of the device changes. Parse used endpoints.
 * \param cfgnum New configuration number.
 */
void USBDDriverCallbacks_ConfigurationChanged(unsigned char cfgnum)
{
    CDCDSerialDriver_ConfigurationChangedHandler(cfgnum);
}

/** 
 * Callback re-implementation
 * Invoked when a new SETUP request is received from the host. 
 */
void USBDCallbacks_RequestReceived(const USBGenericRequest *request)
{
    CDCDSerialDriver_RequestHandler(request);
}

/*
 * Transfer callback used internally from USBC_Puts.
 */
static void UsbWriteDone(uint32_t unused,
			             uint8_t status,
			             uint32_t transferred,
						 uint32_t remaining)
{
	if (status == USBD_STATUS_SUCCESS) {
		_txCount = transferred;
		TRACE_DEBUG("USB sent %u bytes.\n", _txCount);
	}
	else
        TRACE_WARNING("USBC Write unsuccessful.\n");
}

/*
 * Transfer callback used internally from USBC_Gets.
 */
static void UsbReadDone(uint32_t unused,
						uint8_t status,
						uint32_t received,
						uint32_t remaining)
{
    if (status == USBD_STATUS_SUCCESS) {
		_rxCount = received;
		TRACE_DEBUG("USB read %u bytes.\n", _rxCount);
	}
	else
        TRACE_WARNING("USBC Read unsuccessful.\n");
}

/*----------------------------------------------------------------------------
 *
 *         Exported Functions
 *----------------------------------------------------------------------------*/

/*
 * Return 1 if USB Driver is configured,
 * return 0 otherwise.
 */
uint8_t USBC_isConfigured(void)
{
	if (USBD_GetState() == USBD_STATE_CONFIGURED) {
		if (_cfgdone == 0) {
			TRACE_INFO("USB Device Driver is configured\n");
			_cfgdone = 1;
			USBC_StartListening();
		}
		return 1;
	}
	else {
		_cfgdone = 0;
		return 0;
	}
}

/*
 * Return 1 if data present, 
 * return 0 otherwise.
 */
uint8_t USBC_hasData()
{
	return (_rxCount > 0);
}

/*
 * Configure USB serial.
 */
void USBC_Configure(void)
{
	TRACE_INFO("USB Serial Console configuration\n");
	PIO_InitializeInterrupts(0);
	ConfigureUsbClock();
	CDCDSerialDriver_Initialize(&cdcdSerialDriverDescriptors);
	VBus_Configure();
}

/*
 * Start reading to buffer.
 * Returns 1 on success,
 * returns 0 otherwise.
 */
uint8_t USBC_StartListening()
{
	if(CDCDSerialDriver_Read(_rxBuffer, RXBUFFERSIZE, (TransferCallback) UsbReadDone, 0)
			!= USBD_STATUS_SUCCESS) {
		TRACE_WARNING("USBC listening failed.\n");
		return 0;
	}
	return 1;
}

/*
 * Read input
 * returns -1 on failure,
 * returns number of bytes read otherwise.
 */ 
int USBC_Gets(char *ptr, uint16_t len)
{
	uint16_t done;

	if(!USBC_isConfigured())
		return -1;

	while(_rxCount == 0);

	done = len >= _rxCount ? _rxCount : len;
	if (done > 0) {
		memcpy(ptr, _rxBuffer, done);
		_rxCount -= done;
		if (_rxCount == 0) // no data left
			USBC_StartListening();
		else               // re-arrange data left
			memmove(_rxBuffer, _rxBuffer+done, _rxCount); 
	}
	return done;
}

/*
 * Write output 
 * returns -1
 * returns number of bytes that will be sent otherwise.
 */ 
int USBC_Puts(char *ptr, uint16_t len)
{
	if(!USBC_isConfigured())
		return -1;

	_txCount = 0;
	if(CDCDSerialDriver_Write(ptr, len, (TransferCallback) UsbWriteDone, 0)
		!= USBD_STATUS_SUCCESS)
		return -1;
	return len;
}
