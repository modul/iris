#ifndef _INPUT_
#define _INPUT_

void stop_sampling();
void start_sampling();

uint16_t get_latest_volt(unsigned index);
uint16_t get_previous_volt(unsigned index);

#endif
