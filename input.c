#include <string.h>
#include "conf.h"
#include "input.h"

#define mV(b) ((b*VREF)>>RESOLUTION)

/* AD7793 SPI Commands */
#define AD_READ_STAT 0x40
#define AD_READ_MODE 0x48
#define AD_READ_CONF 0x50
#define AD_READ_DATA 0x58
#define AD_WRITE_MODE 0x08
#define AD_WRITE_CONF 0x10

#define AD_STAT_RDY (1 << 7)
#define AD_STAT_ERR (1 << 6)

/* AD7793 Modes (High byte) */
#define AD_MODE_CONT 0x00
#define AD_MODE_SINGLE 0x20
#define AD_MODE_INTZERO 0x80
#define AD_MODE_INTFULL 0xA0

/* AD7793 Modes (Low byte) */
#define AD_MODE_LOW 0x0A

/* AD7793 Settings */
#define AD_CONF_HI 0x10
#define AD_CONF_LO 0x90

static input_t latest[NUM_AIN] = {0};  
static input_t previous[NUM_AIN] = {0}; 

struct chan {
	uint8_t num;
	uint8_t gain;
};

static struct chan channel[NUM_AIN] = {
	{Fchan, Fgain}, {pchan, pgain}, {schan, sgain}
};

static void ain_config(struct chan ch)
{
	SPI_Write(SPI, AIN_CS, AD_WRITE_CONF);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_CONF_HI|(ch.gain&0x07));
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_CONF_LO|(ch.num&0x07));
	SPI_Read(SPI);
}

static void ain_mode(uint8_t mode)
{
	SPI_Write(SPI, AIN_CS, AD_WRITE_MODE);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, mode);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, AD_MODE_LOW);
	SPI_Read(SPI);
}

static uint8_t ain_status()
{
	SPI_Write(SPI, AIN_CS, AD_READ_STAT);
	SPI_Read(SPI);
	SPI_Write(SPI, AIN_CS, 0xFF);
	return SPI_Read(SPI);
}

static int ain_read()
{	
	union {
		input_t w;
		uint8_t b[sizeof(input_t)];
	} value;
	value.w = 0;

	SPI_Write(SPI, AIN_CS, AD_READ_DATA);
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
	uint8_t tmp = 0;

	for (i=0; i<NUM_AIN; i++) {
		previous[i] = latest[i];
		ain_config(channel[i]);

		tmp = 0;
		ain_mode(AD_MODE_SINGLE);
		while (ain_status() & AD_STAT_ERR && tmp++ < 3) {
			TRACE_ERROR("ADC reported error on channel %u\n", channel[i].num);
			ain_mode(AD_MODE_SINGLE);
		}
		
		while (ain_status() & AD_STAT_RDY) {
			TRACE_DEBUG("waiting for ADC to finish conversion (%u)\n", channel[i].num);
			Wait(1);
		}

		latest[i] = ain_read();
	}

	TRACE_DEBUG("Got Samples: %u %u %u\n", latest[0], latest[1], latest[2]);
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
