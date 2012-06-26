#include <string.h>
#include "conf.h"
#include "input.h"

#define mV(b) ((b*VREF)>>RESOLUTION)

#define WRITE 0 << 6
#define READ  1 << 6
#define AD_STATREG 0 << 3
#define AD_MODEREG 1 << 3
#define AD_CONFREG 2 << 3
#define AD_DATAREG 3 << 3
#define AD_OFFSREG 6 << 3
#define AD_FULLREG 7 << 3

#define AD_MODE_HI 0x20
#define AD_MODE_LO 0x0A

#define AD_CONF_HI 0x00
#define AD_CONF_LO 0x90

static input_t latest[NUM_AIN] = {0};  
static input_t previous[NUM_AIN] = {0}; 

static input_t ain_read(unsigned chan, unsigned gain)
{	
	const Pin rdy = PIN_AIN_RDY;
	union {
		input_t w;
		uint8_t b[sizeof(input_t)];
	} value;

	SPI_Write(SPI, AIN_CS, AD_CONFREG|WRITE);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_CONF_HI|(gain&0x07));
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_CONF_LO|(chan&0x0F));
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_MODEREG|WRITE);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_MODE_HI);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_MODE_LO);
	SPI_Read(SPI);

	while (PIO_Get(&rdy)); // white for conversion to complete
	SPI_Write(SPI, AIN_CS, AD_DATAREG|READ);
	SPI_Read(SPI);

	SPI_Write(SPI, AIN_CS, 0xFF);
	value.b[2] = SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, 0xFF);
	value.b[1] = SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS|SPI_TDR_LASTXFER, 0xFF);
	value.b[0] = SPI_Read(SPI);

	return value.w;
}

void TC0_IrqHandler()
{
	uint32_t i = TC0->TC_CHANNEL[0].TC_SR;

	for (i=0; i<NUM_AIN; i++) {
		previous[i] = latest[i];
		latest[i] = ain_read(i, 1);
	}


}

void start_sampling()
{
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_SetPriority(TC0_IRQn, 1);
}

void stop_sampling()
{
	NVIC_DisableIRQ(TC0_IRQn);
}

input_t get_latest_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mV(latest[index]);
}

input_t get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mV(previous[index]);
}
