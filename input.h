#ifndef _INPUT_
#define _INPUT_

void setup_channel(int id, int num, int gain, int max);
void get_channel(int id, int *num, int *gain, int *max);

void stop_sampling();
void start_sampling();

int latest(int id);
int previous(int id);

#endif
