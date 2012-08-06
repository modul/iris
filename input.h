#ifndef _INPUT_
#define _INPUT_

#define F 0
#define p 1
#define s 2

#define CHANNELS 3
#define CHANNEL_ID(c) (c == 'F'? F : (c == 'p'? p: (c == 's'? s : 0xFF)))
#define CHANNEL_NAME(i) (i == F? 'F' : (i == p? 'p' : (i == s? 's' : 'x')))

void setup_channel(int id, int num, int gain, int max);
void get_channel(int id, int *num, int *gain, int *max);

void stop_sampling();
void start_sampling();

int latest(int id);
int previous(int id);

#endif
