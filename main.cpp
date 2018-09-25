#include "mbed.h"

#define NO_OS_INTERFERENCE 1

#if NO_OS_INTERFERENCE
#include "mbed_lp_ticker_wrapper.h"
#endif

#define SLEEP_DURATION_US 100000ULL
#define SERIAL_FLUSH_TIME_MS 20

void wakeup_callback(volatile int *wakeup_flag)
{
    (*wakeup_flag)++;
}

int main()
{
#if NO_OS_INTERFERENCE
    // Suspend the RTOS kernel scheduler to prevent interference with duration of sleep.
    osKernelSuspend();
#if DEVICE_LPTICKER && (LPTICKER_DELAY_TICKS > 0)
    // Suspend the low power ticker wrapper to prevent interference with deep sleep lock.
    lp_ticker_wrapper_suspend();
#endif
#endif

    volatile int wakeup_flag;
    LowPowerTimeout lp_timeout;

    bool deepsleep_status1 = sleep_manager_can_deep_sleep();
    sleep_manager_lock_deep_sleep();
    bool deepsleep_status2 = sleep_manager_can_deep_sleep();

    wakeup_flag = 0;
    lp_timeout.attach_us(mbed::callback(wakeup_callback, &wakeup_flag), SLEEP_DURATION_US);

    while (wakeup_flag == 0) {
        sleep_manager_sleep_auto();
    }

    bool deepsleep_status3 = sleep_manager_can_deep_sleep();
    sleep_manager_unlock_deep_sleep();
    bool deepsleep_status4 = sleep_manager_can_deep_sleep();

    printf("------------------------------------------------------------\r\n");
    printf("deepsleep_status1 = %i\r\n", deepsleep_status1);
    printf("deepsleep_status2 = %i\r\n", deepsleep_status2);
    printf("deepsleep_status3 = %i\r\n", deepsleep_status3);
    printf("deepsleep_status4 = %i\r\n", deepsleep_status4);
    printf("\r\n");
    wait_ms(SERIAL_FLUSH_TIME_MS);
}
