#ifndef _PGA280_
#define _PGA280_

#include "conf.h"

/* SPI commands */
#define PGA280_RESET 0x4101  // Write Reg. 1: Software Reset
#define PGA280_FLAGS 0x8400  // Read Reg. 4: Error/Status Flags
#define PGA280_ERRCL 0x4400  // Write Reg. 4: Clear Error/Status Flags
#define PGA280_RGAIN 0x8000  // Read Reg. 0: Gain Setting
#define PGA280_WGAIN 0x6000  // Write Reg. 0: Gain Setting

/* Gain mask */
#define PGA280_GAIN_Pos 3
#define PGA280_GAIN_Msk 0xF1

#define PGA280_GAIN_SETMIN 0
#define PGA280_GAIN_SETMAX 10

/* Error and status flags */
#define PGA280_FIOV  (1<<0)
#define PGA280_FGAIN (1<<1)
#define PGA280_FOUT  (1<<2)
#define PGA280_FERR  (1<<3)
#define PGA280_FICA  (1<<4)
#define PGA280_FBUF  (1<<5)
#define PGA280_FIAR  (1<<6)
#define PGA280_FCHKE (1<<7)

void PGA_setup();
void PGA_set_gain(uint8_t setting);
uint8_t PGA_status();

#endif
