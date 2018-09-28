#include "mbed.h"
#include "mbed_lp_ticker_wrapper.h"

DigitalOut d0(D0);
extern "C" void set_D0(int val)
{
    d0.write(val);
}
DigitalOut d1(D1);
extern "C" void set_D1(int val)
{
    d1.write(val);
}
DigitalOut d2(D2);
extern "C" void set_D2(int val)
{
    d2.write(val);
}
DigitalOut d3(D3);
extern "C" void set_D3(int val)
{
    d3.write(val);
}

int main()
{
    set_D2(0);
    set_D2(1);

    // Suspend the RTOS kernel scheduler to prevent interference from SysTick.
    osKernelSuspend();
    // Suspend the low power ticker wrapper.
    lp_ticker_wrapper_suspend();
    // Init the lp ticker.
    ticker_read(get_lp_ticker_data());

    while(1);
}
