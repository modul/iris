#include "conf.h"
#include "ad7793.h"

static unsigned spitrans(unsigned data)
{
	SPI_Write(SPI, AIN_CS, data);
	return SPI_Read(SPI);
}

void ain_start(uint8_t channel, uint8_t gain, uint8_t mode)
{
	spitrans(AD_WRITE_CONF);
	spitrans(AD_CONF_HI|(gain&0x07));
	spitrans(AD_CONF_LO|(channel&0x07));
	spitrans(AD_READ_CONF);

	TRACE_DEBUG("ADC conf ch%u w:%x%x r:%x%x\n",
			channel,
			AD_CONF_HI|(gain&7),
			AD_CONF_LO|(channel&7),
			spitrans(AD_DUMMY),
			spitrans(AD_DUMMY));

	spitrans(AD_WRITE_MODE);
	spitrans(mode);
	spitrans(AD_MODE_LOW|SPI_TDR_LASTXFER);
}

uint8_t ain_status()
{
	spitrans(AD_READ_STAT);
	return (uint8_t) spitrans(AD_DUMMY|SPI_TDR_LASTXFER)&0xFF;
}

int ain_read()
{
	int value = 0;
	uint64_t result = VREF;

	spitrans(AD_READ_DATA);
	value |= spitrans(AD_DUMMY) << 16;
	value |= spitrans(AD_DUMMY) << 8;
	value |= spitrans(AD_DUMMY);

	return (int) ((result*value)>>RESOLUTION);
}

int ad_temperature()
{
	ain_start(AD_CHT, 0, AD_MODE_SINGLE);
	Wait(AD_WAIT);
	return ain_read();
}

int ad_voltmon()
{
	ain_start(AD_CHV, 0, AD_MODE_SINGLE);
	Wait(AD_WAIT);
	return ain_read() * 6;
}

int ad_calibrate(uint8_t channel, uint8_t gain)
{
	ain_start(channel, gain, AD_MODE_INTZERO);
	Wait(AD_WAIT);
	if (ain_status() & (AD_STAT_NRDY|AD_STAT_ERR))
		return 0;

	if (gain == 7)
		return 1;
	else {
		ain_start(channel, gain, AD_MODE_INTFULL);
		Wait(AD_WAIT);
		if (gain > 0)
			Wait(AD_WAIT);
		if (ain_status() & (AD_STAT_NRDY|AD_STAT_ERR))
			return 0;
	}
	return 1;
}
