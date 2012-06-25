#ifndef _INPUT_
#define _INPUT_

typedef uint32_t input_t;

void stop_sampling();
void start_sampling();

input_t get_latest_volt(unsigned index);
input_t get_previous_volt(unsigned index);

#endif
