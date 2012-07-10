#ifndef _INPUT_
#define _INPUT_

void setup_channel(int id, int num, int gain, int max);
void get_channel(int id, int *num, int *gain, int *max);

void stop_sampling();
void start_sampling();

int get_latest_volt(unsigned index);
int get_previous_volt(unsigned index);

#endif
