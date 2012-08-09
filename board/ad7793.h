#ifndef _AD7793_
#define _AD7793_

#define AD_SETTLE_SLOW 150
#define AD_SETTLE_FAST  15

#define AD_RESOLUTION 24
#define AD_VREF 1170000
#define AD_VMAX AD_VREF-1
#define AD_VMIN -AD_VREF

/* AD7793 SPI Commands */
#define AD_READ_STAT 0x40
#define AD_READ_MODE 0x48
#define AD_READ_CONF 0x50
#define AD_READ_DATA 0x58
#define AD_WRITE_MODE 0x08
#define AD_WRITE_CONF 0x10
#define AD_DUMMY 0xFF

/* AD7793 Status Bits */
#define AD_STAT_NRDY (1 << 7)
#define AD_STAT_ERR  (1 << 6)

/* AD7793 Modes (High byte of mode register) */
#define AD_MODE_CONT 0x00
#define AD_MODE_SINGLE 0x20
#define AD_MODE_INTZERO 0x80
#define AD_MODE_INTFULL 0xA0

/* AD7793 Update Rate (Low byte of mode register) */
#define AD_RATE_SLOW 0x0A
#define AD_RATE_FAST 0x02

/* AD7793 Configuration */
#define AD_CONF_HI 0x00 // set bipolar; here goes gain selection
#define AD_CONF_LO 0x90 // set buffered, use int. ref.; here goes channel select

#define AD_GAIN_MIN 0
#define AD_GAIN_MAX 7

#define AD_CH0 0
#define AD_CH1 1
#define AD_CH2 2
#define AD_CHT 6
#define AD_CHV 7
#define AD_CHANNELS 3


unsigned AD7793_status();

void AD7793_start(uint8_t channel, uint8_t gain);
int AD7793_read();

int AD7793_temperature();
int AD7793_voltmon();
int AD7793_calibrate(uint8_t channel, uint8_t gain);

#endif
