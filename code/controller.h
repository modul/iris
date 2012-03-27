#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>

#define SAMPLING_FREQ 4
#define SCALING_FACTOR 128

#define RESOLUTION 12
#define MAX (1<<RESOLUTION) - 1

#define MAXOUT MAX

#define CTRL_INIT {OFF, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

typedef uint16_t ctrlio_t;

typedef enum {OFF, NORMAL, RAMP, STOP} ctrlmode_t;

/* Controller parameter structure */
struct ctrl {
	ctrlmode_t mode;// Current mode
	ctrlio_t SP;    // Setpoint
	ctrlio_t rSP;   // endpoint for ramp
	int rSlope;     // slope for ramp mode

	ctrlio_t Kp;    // P-term coefficient
	ctrlio_t Ki;    // I-term coefficient
	ctrlio_t Kd;    // D-term coefficient

	int _e;         // Error (difference)
	int _dx;        // Change in PV 
	int _de;        // Change in e 
	int _ddx;       // Change in dx 
	ctrlio_t _x;    // Process value

	short output;   // Output

	struct trip *tristate; // Tristate parameters for three-point control
};

/* Tristate parameter structure */
struct trip {
	int *input;
	int output;
	int on;        // total deadband = 2*on
	int off;       // hysteresis = on-off
};

#define LIMIT(a, amin, amax) (a < amin ? amin : (a > amax ? amax : a))
#define SCALE(x) ((ctrlio_t) (x * SCALING_FACTOR))

void mode(uint8_t new, struct ctrl *ctrl);
void control(ctrlio_t *pv, struct ctrl *loop, unsigned loops);

int tris(int in, int *output, int on, int off);
int tristate(struct trip *p);

#endif
