#include "conf.h"
#include "ad7793.h"

static unsigned spitrans(unsigned data)
{
	SPI_Write(SPI, ADC_CS, data);
	return SPI_Read(SPI);
}

static void setup(uint8_t channel, uint8_t gain, uint8_t mode, uint8_t rate)
{
	spitrans(AD_WRITE_CONF);
	spitrans(AD_CONF_HI|(gain&0x07));
	spitrans(AD_CONF_LO|(channel&0x07));

	spitrans(AD_WRITE_MODE);
	spitrans(mode);
	spitrans(rate|SPI_TDR_LASTXFER);
}

void AD7793_start(uint8_t channel, uint8_t gain)
{
	setup(channel, gain, AD_MODE_SINGLE, AD_RATE_FAST);
}

unsigned AD7793_status()
{
	spitrans(AD_READ_STAT);
	return spitrans(AD_DUMMY|SPI_TDR_LASTXFER);
}

int AD7793_read()
{
	int value = 0;
	long int result = AD_VREF;

	spitrans(AD_READ_DATA);
	value |= spitrans(AD_DUMMY) << 16;
	value |= spitrans(AD_DUMMY) << 8;
	value |= spitrans(AD_DUMMY);

	result = result*value;
	result = (result>>(AD_RESOLUTION-1)) - AD_VREF;
	return (int) result;
}

int AD7793_temperature()
{
	AD7793_start(AD_CHT, 0);
	wait(AD_SETTLE_FAST);
	return AD7793_read();
}

int AD7793_voltmon()
{
	AD7793_start(AD_CHV, 0);
	wait(AD_SETTLE_FAST);
	return AD7793_read() * 6;
}

int AD7793_calibrate(uint8_t channel, uint8_t gain)
{
	setup(channel, gain, AD_MODE_INTZERO, AD_RATE_SLOW);
	wait(AD_SETTLE_SLOW);
	if (AD7793_status() & (AD_STAT_NRDY|AD_STAT_ERR))
		return 0;

	if (gain < AD_GAIN_MAX) { // FS calibration not possible for max gain
		setup(channel, gain, AD_MODE_INTFULL, AD_RATE_SLOW);
		wait(AD_SETTLE_SLOW);
		if (gain > 0)
			wait(AD_SETTLE_SLOW);
		if (AD7793_status() & (AD_STAT_NRDY|AD_STAT_ERR))
			return 0;
	}
	return 1;
}
