#ifndef INPUT_H
#define INPUT_H

extern uint16_t current[NUM_AIN];   // current ADC input in mV
extern uint16_t previous[NUM_AIN];  // previous ADC input in mV

void stop_sampling();
void start_sampling();

#endif
