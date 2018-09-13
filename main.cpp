#include "mbed.h"

#define NO_OS_INTERFERENCE 0

#if NO_OS_INTERFERENCE
#include "mbed_lp_ticker_wrapper.h"
#endif

#define SLEEP_DURATION_US 100000ULL
#define SERIAL_FLUSH_TIME_MS 20

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
    const ticker_data_t *const us_ticker = get_us_ticker_data();
    const ticker_data_t *const lp_ticker = get_lp_ticker_data();

    uint32_t us_ticks1, us_ticks2, lp_ticks1, lp_ticks2;
    us_timestamp_t us_ts1, lp_ts1, us_ts2, lp_ts2;

    MBED_ASSERT(sleep_manager_can_deep_sleep());

    // Wait for hardware serial buffers to flush.
    wait_ms(SERIAL_FLUSH_TIME_MS);

    wakeup_flag = 0;
    lp_timeout.attach_us(mbed::callback(wakeup_callback, &wakeup_flag), SLEEP_DURATION_US);

    us_ticks1 = us_ticker_read();
    lp_ticks1 = lp_ticker_read();
    us_ts1 = ticker_read_us(us_ticker);
    lp_ts1 = ticker_read_us(lp_ticker);

    // Deep sleep unlocked -- deep sleep mode used:
    // * us_ticker powered OFF,
    // * lp_ticker powered ON.
    while (wakeup_flag == 0) {
        sleep_manager_sleep_auto();
    }

    us_ticks2 = us_ticker_read();
    lp_ticks2 = lp_ticker_read();
    us_ts2 = ticker_read_us(us_ticker);
    lp_ts2 = ticker_read_us(lp_ticker);

    printf("------------------------------------------------------------\r\n");
    printf("%24s%12s%12s%12s\r\n", "us_ticks", "us_ts", "lp_ticks", "lp_ts");
    printf("%12s%12lu%12llu%12lu%12llu\r\n", "before", us_ticks1, us_ts1, lp_ticks1, lp_ts1);
    printf("%12s%12lu%12llu%12lu%12llu\r\n", "after", us_ticks2, us_ts2, lp_ticks2, lp_ts2);
    printf("%12s%12lu%12llu%12lu%12llu\r\n", "diff", us_ticks2 - us_ticks1, us_ts2 - us_ts1, lp_ticks2 - lp_ticks1,
           lp_ts2 - lp_ts1);
    printf("\r\n");
    wait_ms(SERIAL_FLUSH_TIME_MS);
}
