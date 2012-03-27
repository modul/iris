#include <stdio.h>
#include <assert.h>
#include "controller.h"

/*
 * PID Equations:
 *
 * A:  u = Kp*(w-x) + Ki*T*Sum(w-x) + Kd/T * Dif(w-x)
 * B:  u = Kp*(w-x) + Ki*T*Sum(w-x) + Kd/T * Dif( -x) // More robust: D-term based on x only, i.e. no spike on setpoint change
 * C:  u = Kp*( -x) + Ki*T*Sum(w-x) + Kd/T * Dif( -x) // even more robust: P-term based on x too 
 *
 * with u: output, w: setpoint, x: process value, T = 1/freq: sampling time
 *
 * Type C is not suitable when I-term isn't active (Ki=0).
 *
 * Differentiating both sides gives a cumulative equation.
 * du = -K_dx + Ki*T*(w-x) - Kd/T*ddx          (Type C)
 *
 */
static void pid(struct ctrl *ct)
{
	int pterm, iterm, dterm;
	int u = ct->output;

	// Calculating type B when Ki=0, type C otherwise.
	if (ct->Ki == 0) { 
		iterm = 0;
		pterm = ct->Kp * ct->_de; // otherwise nothing would regard setpoint change
	}
	else {
		iterm = (ct->Ki * ct->_e) / SAMPLING_FREQ;
		pterm = -1 * ct->Kp * ct->_dx;
	}
	dterm = -1 * ct->Kd * ct->_ddx * SAMPLING_FREQ;

	u += (pterm + iterm + dterm)/SCALING_FACTOR;
	ct->output = (short) LIMIT(u, -MAXOUT, MAXOUT);
}

static void update(ctrlio_t pv, struct ctrl *ctrl)
{
	int e, dx;
	dx = pv - ctrl->_x;

	if (ctrl->mode == RAMP)
		e = ctrl->rSlope - (dx * SAMPLING_FREQ);
	else
		e = ctrl->SP - pv;

	ctrl->_ddx = dx - ctrl->_dx;
	ctrl->_de = e - ctrl->_e;
	ctrl->_dx = dx;
	ctrl->_e = e;
	ctrl->_x = pv;

	if (ctrl->tristate != 0 && ctrl->tristate->input != 0) 
		tristate(ctrl->tristate);
}

// React on mode transition
void mode(uint8_t new, struct ctrl *ctrl)
{
	switch (new) {
		case OFF:
			ctrl->mode = new;
			ctrl->SP = 0;
			ctrl->rSP = 0;
			ctrl->rSlope = 0;
			ctrl->output = 0;

			ctrl->_e = 0;
			ctrl->_x = 0;
			ctrl->_de = 0;
			ctrl->_dx = 0;
			ctrl->_ddx = 0;

			if (ctrl->tristate != 0)
				ctrl->tristate->output = 0;
			break;

		case RAMP:
			if (   (ctrl->_x > ctrl->rSP && ctrl->rSlope > 0) \
				|| (ctrl->_x < ctrl->rSP && ctrl->rSlope < 0))
				ctrl->rSlope = -ctrl->rSlope; // adjust direction
		case NORMAL:
			ctrl->mode = new;
			break;

		default:
			ctrl->mode = STOP;
	}
}

// Periodically called 
void control(ctrlio_t *pv, struct ctrl *loop, unsigned loops)
{
	while (loops) {
		if (loop->mode > OFF) {
			update(*pv, loop);

			if (loop->mode == NORMAL)
				pid(loop);
			else if (loop->mode == RAMP) {
				pid(loop);
				if (   (loop->_x >= loop->rSP && loop->rSlope > 0) \
					|| (loop->_x <= loop->rSP && loop->rSlope < 0) \
					|| (loop->rSlope == 0)) {
					loop->SP = loop->rSP;
					mode(NORMAL, loop);
				}
			}
		}
		else if (loop->mode > STOP) {
			mode(STOP, loop);
		}
		loops--;
		loop++;
		pv++;
	}
}

/*
 * Call tris() with pointer to parameters.
 * (Useful in the way that parameter struct can
 * be part of the configuration).
 */
int tristate(struct trip *p)
{
	assert(p != 0);
	assert(p->input != 0);
	return tris(*p->input, &p->output, p->on, p->off);
}

/* Get tristate output for a three-point controller
 * 
 * The result is returned and written to '*output'
 * which must hold the previous value and be initialized
 * with zero.
 */
int tris(int in, int *output, int on, int off)
{
	if (*output < -1 || *output > 1)
		*output == 0;

	if (*output < 0) {
		if (in >= -off) 
			*output = 0;
	}
	else if (*output > 0) {
		if (in <= off) 
			*output = 0;
	}
	else {
		if (in > on) 
			*output = 1;
		else if (in < -on) 
			*output = -1;
	}
	return *output;
}
