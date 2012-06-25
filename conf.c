#include <string.h>
#include "conf.h"

#define LIMIT(x, min, max) (x > max? max-1 : (x < min? min : x))

static conf_t config = {CONF_INIT};

void get_config(conf_t *dest)
{
	memcpy(dest, &config, sizeof(config));
}

void set_config(conf_t src)
{
	config.fmax   = LIMIT(src.fmax, 0, VREF);
	config.pmax   = LIMIT(src.pmax, 0, VREF);
	config.smax   = LIMIT(src.smax, 0, VREF);
	config.gainid = (uint8_t) LIMIT(src.gainid, 1, 128);
}

