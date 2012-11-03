#include <stdio.h>
#include <string.h>
#include "conf.h"
#include "state.h"
#include "input.h"
#include "version.h"

#define OK()  puts("ok")
#define NOK() puts("nok")

#define limit(x, min, max) (x < min? min : (x > max? max : x))

static void do_start();
static void do_log();
static void do_abort();
static void do_info();
static void do_conf();
static void do_store();
static void do_load();
static void do_version();

typedef void(*functionPointer)(void);

struct command {
	char c;
	functionPointer exec;
	char *help;
};

static const struct command commands[] = {
	{'s', do_start, "start"},
	{'l', do_log,   "log"},
	{'a', do_abort, "abort/acknowledge"},
	{'i', do_info,  "info"},
	{'c', do_conf,  "conf"},
	{'S', do_store, "store configuration"},
	{'L', do_load,  "load configuration"},
	{'v', do_version, "show version"},
	{0,  0, ""}
};

unsigned command_invoke(char c)
{
	int i=0; 
	struct command cmd = commands[0];

	if (c == 'h' || c == '?' || c == 'H') {
		while (cmd.exec) {
			printf("%c -- %s\n", cmd.c, cmd.help);
			cmd = commands[++i];
		}
		printf("H, h, ? -- display this help text\n");
		return 1;
	}

	while (cmd.exec) {
		if (cmd.c == c) {
			cmd.exec();
			return 1;
		}
		cmd = commands[++i];
	}
	return 0;
}

static void do_start()
{
	state_send(EV_START);
	OK();
}

static void do_log()
{
	printf("%u %u %u %u %i %i %i\n", 
			state_getState(), state_getError(F), state_getError(p), state_getError(s), 
			input_latest(F), input_latest(p), input_latest(s));
}

static void do_abort()
{
	if (state_getState() == STOP) { // acknowledge error
		const Pin stop = PIN_STOP;
		if (PIO_Get(&stop)) {
			state_reset();
			OK();
		}
		else NOK();
	}
	else {
		state_reset();
		OK();
	}
}

static void do_info()
{
	if (!state_isSafe())
		NOK();
	else {
		char c;
		int i, tmp;
		struct chan *channel;
	
		input_stop();
		if ((c = getchar()) == 'V') {
			tmp = AD7793_voltmon();
			printf("%u.%uV\n", tmp/1000000, tmp%1000000);
		}
		else if (c == 'T') {
			tmp = AD7793_temperature();
			printf("%u.%uC\n", tmp/10000, tmp%10000);
		}
		else {
			if ((i = CHANNEL_ID(c)) < CHANNELS)
				tmp = i+1;
			else {
				i = 0;
				tmp = CHANNELS;
			}
			
			while (i < tmp) {
				channel = conf_get(i);
				printf("%c %s ch%u %ux %i ... %i %iuV\n", 
					CHANNEL_NAME(i), ERROR_NAME(state_getError(i)), 
					channel->num, 1<<channel->gain,
					channel->min, channel->max,
					input_latest(i));
				i++;
			}
		}
		input_start();
	}
}

static void do_conf()
{
	if (!state_isSafe())
		NOK();
	else {
		char c = 0;
		char line[64];
		int args, id, num, gain, min, max;
		struct chan *channel;

		gets(line);
		args = sscanf(line, "%c %u %u %i %i", &c, &num, &gain, &min, &max);
		if (args > 0) {
			id = CHANNEL_ID(c);
			if (id >= CHANNELS)
				NOK();
			else if (args == 1) {
				channel = conf_get(id);
				printf("%c %u %u %i %i\n", c, channel->num, channel->gain, channel->min, channel->max);
			}
			else {
				int tmp;
				input_stop();
				channel = conf_get(id);
				tmp = channel->gain;

				channel->num = limit(num, 0, AD_CHANNELS);

				if (args >= 3) {
					channel->gain = limit(gain, AD_GAIN_MIN, AD_GAIN_MAX);
					if (tmp != channel->gain)
						input_calibrate(id);
				}
				if (args == 4) {
					max = limit(min, 0, AD_VMAX);
					min = limit(-min, AD_VMIN, 0);
					channel->min = min;
					channel->max = max;
				}
				else if (args >= 5) {
					channel->min = limit(min, AD_VMIN, AD_VMAX);
					channel->max = limit(max, channel->min, AD_VMAX);
				}

				printf("ok %c %u %u %i %i\n", c, channel->num, channel->gain, channel->min, channel->max);
				input_start();
			}
		}
		else NOK();
	}
}

static void do_store()
{
	if (state_isSafe()) {
		input_stop();
		conf_store();
		input_start();
		OK();
	}
	else NOK();
}

static void do_load()
{
	if (state_isSafe()) {
		input_stop();
		conf_load();
		OK();
		input_calibrate(CHANNELS);
		input_start();
	}
	else NOK();
}

static void do_version()
{
	printf("%s %s %s\n", BOARD_NAME, VERSION, BUILD_DATE);
}
