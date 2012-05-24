#include <string.h>
#include "conf.h"
#include "pga280.h"

#define LIMIT(x, min, max) (x > max? max-1 : (x < min? min : x))

static conf_t config = {CONF_INIT};

void get_config(conf_t *dest)
{
	memcpy(dest, &config, sizeof(config));
}

void set_config(conf_t src)
{
	config.fmax   = LIMIT(src.fmax, 0, VREF-1);
	config.pmax   = LIMIT(src.pmax, 0, VREF-1);
	config.smax   = LIMIT(src.smax, 0, VREF-1);
	config.gainid = (uint8_t) LIMIT(src.gainid, PGA280_GAIN_SETMIN, PGA280_GAIN_SETMAX);
}

