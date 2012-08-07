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

void ad_start(uint8_t channel, uint8_t gain)
{
	setup(channel, gain, AD_MODE_SINGLE, AD_RATE_FAST);
}

unsigned ad_status()
{
	spitrans(AD_READ_STAT);
	return spitrans(AD_DUMMY|SPI_TDR_LASTXFER);
}

int ad_read()
{
	int value = 0;
	uint64_t result = AD_VREF;

	spitrans(AD_READ_DATA);
	value |= spitrans(AD_DUMMY) << 16;
	value |= spitrans(AD_DUMMY) << 8;
	value |= spitrans(AD_DUMMY);

	return (int) ((result*value)>>AD_RESOLUTION);
}

int ad_temperature()
{
	ad_start(AD_CHT, 0);
	wait(AD_SETTLE_FAST);
	return ad_read();
}

int ad_voltmon()
{
	ad_start(AD_CHV, 0);
	wait(AD_SETTLE_FAST);
	return ad_read() * 6;
}

int ad_calibrate(uint8_t channel, uint8_t gain)
{
	setup(channel, gain, AD_MODE_INTZERO, AD_RATE_SLOW);
	wait(AD_SETTLE_SLOW);
	if (ad_status() & (AD_STAT_NRDY|AD_STAT_ERR))
		return 0;

	if (gain < AD_GAIN_MAX) { // FS calibration not possible for max gain
		setup(channel, gain, AD_MODE_INTFULL, AD_RATE_SLOW);
		wait(AD_SETTLE_SLOW);
		if (gain > 0)
			wait(AD_SETTLE_SLOW);
		if (ad_status() & (AD_STAT_NRDY|AD_STAT_ERR))
			return 0;
	}
	return 1;
}
