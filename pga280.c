#include "conf.h"
#include "pga280.h"

/* Send software reset and set gain to minimum */
void PGA_setup()
{
	SPI_Write(SPI, PGA_CS, (uint16_t) PGA280_RESET);
	SPI_Read(SPI);
	SPI_Write(SPI, PGA_CS, (uint16_t) PGA280_WGAIN);
	SPI_Read(SPI);
}

/* Set gain */
void PGA_set_gain(uint8_t setting)
{
	assert(setting >= PGA280_GAIN_SETMIN && setting <= PGA280_GAIN_SETMAX);

	SPI_Write(SPI, PGA_CS, (uint16_t) (PGA280_WGAIN|(setting<<PGA280_GAIN_Pos)));
	SPI_Read(SPI);
}

/* Request status byte */
uint8_t PGA_status()
{
	SPI_Write(SPI, PGA_CS, (uint16_t) PGA280_FLAGS);
	return (uint8_t) (SPI_Read(SPI) & 0xFF);
}

