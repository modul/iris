#include <string.h>
#include "flashwrite.h"
#include "conf.h"

static struct chan channel[CHANNELS] = {{0}};

void conf_store()
{
	flashwrite((uint32_t *) channel, CHANNELS*sizeof(struct chan));
}

void conf_load()
{
	memcpy((uint8_t *) channel, (uint8_t *) FLASHPAGE, CHANNELS*sizeof(struct chan));
}

struct chan * conf_get(int id)
{
	return (struct chan *) channel+(id%CHANNELS);
}