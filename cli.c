#include <stdio.h>
#include <string.h>
#include "conf.h"
#include "state.h"
#include "input.h"
#include "output.h"
#include "version.h"

#define OK()  puts("ok")
#define NOK() puts("nok")

#define limit(x, min, max) (x < min? min : (x > max? max : x))

static void do_start();
static void do_abort();
static void do_log();
static void do_info();
static void do_up();
static void do_down();
static void do_conf();
static void do_store();
static void do_load();
static void do_version();
static void do_help();

typedef void(*functionPointer)(void);

struct command {
	char c;
	functionPointer action;
	const char *help;
};

static const struct command commands[] = { // available commands and their actions
	{'s', do_start,   "start/continue"},
	{'a', do_abort,   "abort/acknowledge"},
	{'g', do_log,     "get data"},
	{'i', do_info,    "get info"},
	{'u', do_up,      "piston up"},
	{'d', do_down,    "piston down"},
	{'c', do_conf,    "configure"},
	{'S', do_store,   "store configuration"},
	{'L', do_load,    "load configuration"},
	{'v', do_version, "show version"},
	{'?', do_help,    "show help"},
	{0,  0, ""}
};

static  char *cmask[NUMSTATES] = { // allowed commands per state
	"?vgsicudSL", // IDLE
	"?vga",       // READY
	"?vgas",      // SET
	"?vga",       // GO
	"?vgasicudSL" // STOP
};

void command_invoke(char c)
{
	if (strchr(cmask[state_getState()], c) == 0) {
		TRACE_DEBUG("CLI: command %c not accepted\n", c);
		NOK();
	}
	else {
		int i = 0; 
		struct command cmd = commands[0];
		while (cmd.action) {
			if (cmd.c == c)  {
				TRACE_DEBUG("CLI: command %c invoked\n", c);
				cmd.action();
				break;
			}
			cmd = commands[++i];
		}
	}
}

static void do_help()
{
	int i=0; 
	struct command cmd = commands[0];

	while (cmd.action) {
		printf("%c -- %s\n", cmd.c, cmd.help);
		cmd = commands[++i];
	}
}

static void do_start()
{
	state_send(EV_START);
	OK();
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

static void do_log()
{
	printf("%u %u %u %u %i %i %i\n", 
			state_getState(), state_getError(F), state_getError(p), state_getError(s), 
			input_latest(F), input_latest(p), input_latest(s));
}

static void do_info()
{
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
		if ((i = CHANNEL_ID(c)) < CHANNELS) // print only one channel
			tmp = i+1;
		else { // print all channels and state information
			i = 0;
			tmp = CHANNELS;
			printf("%s\n", STATE_NAME(state_getState()));
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

static void do_up()
{
	output_vent();
	wait(500);
	output_stop();
}

static void do_down()
{
	output_press();
	wait(500);
	output_stop();
}

static void do_conf()
{
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

static void do_store()
{
	input_stop();
	conf_store();
	input_start();
	OK();
}

static void do_load()
{
	input_stop();
	conf_load();
	OK();
	input_calibrate(CHANNELS);
	input_start();
}

static void do_version()
{
	printf("%s %s %s\n", BOARD_NAME, VERSION, BUILD_DATE);
}
