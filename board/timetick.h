#ifndef _TIMETICK_
#define _TIMETICK_

int TimeTick_Configure(unsigned current_clock);
unsigned GetTick();
void Wait(unsigned ms);

#endif
