#ifndef _TIMETICK_
#define _TIMETICK_

int TimeTick_Configure(unsigned current_clock);

unsigned timetick();
void wait(unsigned ms);

#endif
