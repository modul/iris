#include <string.h>
#include "conf.h"
#include "input.h"

/* AD7793 SPI Commands */
#define AD_READ_STAT 0x40
#define AD_READ_MODE 0x48
#define AD_READ_CONF 0x50
#define AD_READ_DATA 0x58
#define AD_WRITE_MODE 0x08
#define AD_WRITE_CONF 0x10
#define AD_DUMMY 0xFF

#define AD_STAT_NRDY (1 << 7)
#define AD_STAT_ERR  (1 << 6)

/* AD7793 Modes (High byte) */
#define AD_MODE_CONT 0x00
#define AD_MODE_SINGLE 0x20
#define AD_MODE_INTZERO 0x80
#define AD_MODE_INTFULL 0xA0

/* AD7793 Modes (Low byte = clk configuration etc) */
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

static unsigned spitrans(unsigned cs, unsigned data)
{
	SPI_Write(SPI, cs, data);
	return SPI_Read(SPI);
}

static int mv(int in)
{
	uint64_t result = VREF;
	return (int) ((result*in)>>RESOLUTION);
}

static void ain_config(struct chan ch)
{
	spitrans(AIN_CS, AD_WRITE_CONF);
	spitrans(AIN_CS, AD_CONF_HI|(ch.gain&0x07));
	spitrans(AIN_CS, AD_CONF_LO|(ch.num&0x07));
	spitrans(AIN_CS, AD_READ_CONF);
	TRACE_DEBUG("ADC conf ch%u w:%x%x r:%x%x\n",
			ch.num,
			AD_CONF_HI|(ch.gain&7),
			AD_CONF_LO|(ch.num&7),
			spitrans(AIN_CS, AD_DUMMY),
			spitrans(AIN_CS, AD_DUMMY));
}

static void ain_mode(uint8_t mode)
{
	spitrans(AIN_CS, AD_WRITE_MODE);
	spitrans(AIN_CS, mode);
	spitrans(AIN_CS, AD_MODE_LOW|SPI_TDR_LASTXFER);
}

static uint8_t ain_status()
{
	spitrans(AIN_CS, AD_READ_STAT);
	return (uint8_t) spitrans(AIN_CS, AD_DUMMY|SPI_TDR_LASTXFER)&0xFF;
}

static int ain_read()
{	
	input_t value = 0;

	spitrans(AIN_CS, AD_READ_DATA);
	value |= spitrans(AIN_CS, AD_DUMMY) << 16;
	value |= spitrans(AIN_CS, AD_DUMMY) << 8;
	value |= spitrans(AIN_CS|SPI_TDR_LASTXFER, AD_DUMMY);

	return value;
}

void TC0_IrqHandler()
{
	uint32_t status = TC0->TC_CHANNEL[0].TC_SR;
	static uint8_t n = 0;

	TRACE_DEBUG("n=%u\n", n);

	if ((status = ain_status()) & AD_STAT_NRDY) {
		TRACE_INFO("ADC ch%u not ready (%x)\n", channel[n].num, status);
		return; // try again next time
	}
	else if (status & AD_STAT_ERR) {
		TRACE_ERROR("ADC error ch%u (%x)\n", channel[n].num, status);
	}
	else {
		previous[n] = latest[n];
		latest[n] = ain_read();
		TRACE_DEBUG("ADC read %u (%x)\n", mv(latest[n]), status);

		if (++n == NUM_AIN)
			n = 0;
	}

	ain_config(channel[n]);
	ain_mode(AD_MODE_SINGLE);
}

/*void TC0_IrqHandler()
{
	uint32_t i = TC0->TC_CHANNEL[0].TC_SR;
	uint8_t tmp = 0;
	uint8_t status = 0;

	for (i=0; i<NUM_AIN; i++) {
		previous[i] = latest[i];
		ain_config(channel[i]);

		ain_mode(AD_MODE_SINGLE);
		if ((status = ain_status()) & AD_STAT_ERR) {
			TRACE_ERROR("ADC error ch%u (%x)\n", channel[i].num, status);
			continue;
		}

		while (((status = ain_status()) & AD_STAT_RDY) && tmp++ < 3) {
			TRACE_DEBUG("ADC busy ch%u (%x)\n", channel[i].num, status);
		}

		latest[i] = ain_read();
		TRACE_DEBUG("ADC status %x\n", ain_status());
		tmp = 0;
	}

	TRACE_DEBUG("Got Samples: %u %u %u\n", latest[0], latest[1], latest[2]);
}*/

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
	return mv(latest[index]);
}

input_t get_previous_volt(unsigned index) {
	assert(index < NUM_AIN);
	return mv(previous[index]);
}
