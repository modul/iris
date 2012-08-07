#include "board.h"

static volatile unsigned count = 0;

void SysTick_Handler()
{
    count++;
	blinkhandler(count);
}

int TimeTick_Configure(unsigned current_clk)
{
    count = 0;
    return SysTick_Config(current_clk/1000);
}

unsigned timetick()
{
    return count ;
}

void wait(unsigned ms)
{
    unsigned start = count;
    while (timetick() - start < ms);
}