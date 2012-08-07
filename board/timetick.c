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

unsigned GetTick()
{
    return count ;
}

void Wait(unsigned ms)
{
    unsigned start = count;
    while (GetTick() - start < ms);
}