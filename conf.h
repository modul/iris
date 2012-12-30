#ifndef _CONF_
#define _CONF_

#include <board.h>

#define TIMER_FREQ 10

/* Configuration Parameters */
#define PAR_PSET   16700 // 1bar with 10x attenuation
#define PAR_PEAK   2

/* Input Channels */
#define F 0
#define p 1
#define s 2

#define CHANNELS 3
#define CHANNEL_ID(c) (c == 'F'? F : (c == 'p'? p: (c == 's'? s : 0xFF)))
#define CHANNEL_NAME(i) (i == F? 'F' : (i == p? 'p' : (i == s? 's' : 'x')))

struct chan {
	int min;
	int max;
	unsigned num:3;
	unsigned gain:3;
};

void conf_store();
void conf_load();
struct chan * conf_get(int id);

#endif
