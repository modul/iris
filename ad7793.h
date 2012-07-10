#ifndef _AD7793_
#define _AD7793_

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

/* AD7793 Channels */
#define AD_CH0 0
#define AD_CH1 1
#define AD_CH2 2
#define AD_CHT 6
#define AD_CHV 7

int mv(int in);

void ain_start(uint8_t channel, uint8_t gain, uint8_t mode);
uint8_t ain_status();
int ain_read();

int ad_temperature();
int ad_voltmon();

#endif
