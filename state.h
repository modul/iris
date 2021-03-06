#ifndef _STATE_
#define _STATE_

#define ERROR_NAME(e) (e == EOK? "ok" : (e == EMIN? "min" : (e == EMAX? "max" : (e == EOVL? "ovl" : "?"))))
#define STATE_NAME(s) (s == IDLE? "IDLE" : (s == READY? "READY" : (s == SET? "SET" : (s == GO? "GO" : (s == STOP? "STOP" : "?")))))

enum events {EV_ERROR, EV_START, EV_CONTACT, EV_BREAK, EV_ABORT, NUMEVENTS};
enum states {IDLE, READY, SET, GO, STOP, NUMSTATES};
enum errors {EOK, EMIN, EMAX, EOVL, NUMERRORS};

void state_send(unsigned ev);
void state_reset();

void state_setError(int id, unsigned err);
unsigned state_getError(int id);
unsigned state_getState();

#endif
