#include "mbed.h"
#include <string>

// Set to 0 to test only the (software) VirtualWatchdog class.
// Set to 1 to test also the (hardware) Watchdog class.
#define USE_HW_WATCHDOG 1

const uint32_t TIMEOUT_MS = 5000;
InterruptIn button(BUTTON1);
VirtualWatchdog vw(TIMEOUT_MS);
volatile int countdown = 9;

void trigger()
{
    vw.kick();
    countdown = 9;
}

#if USE_HW_WATCHDOG
InterruptIn button2(BUTTON2);
void trigger2()
{
    Watchdog &hw_watchdog = Watchdog::get_instance();
    hw_watchdog.kick();
}
#endif

std::string reset_reason_to_string(const reset_reason_t reason)
{
    switch (reason) {
        case RESET_REASON_POWER_ON:
            return "Power On";
        case RESET_REASON_PIN_RESET:
            return "Hardware Pin";
        case RESET_REASON_SOFTWARE:
            return "Software Reset";
        case RESET_REASON_WATCHDOG:
            return "Watchdog";
        default:
            return "Other Reason";
    }
}

int main()
{
    const reset_reason_t reason = ResetReason::get();
    printf("\r\nTarget started. The reason of the last system reset: %s\r\n",
           reset_reason_to_string(reason).c_str());
    printf("VirtualWatchdog initialized to %lu ms.\r\n", TIMEOUT_MS);
    printf("Press button1 at least once every %lu ms to kick the software "
           "watchdog and prevent system reset.\r\n", TIMEOUT_MS);
#if USE_HW_WATCHDOG
    Watchdog &hw_watchdog = Watchdog::get_instance();
    hw_watchdog.start(2 * TIMEOUT_MS);
    button2.rise(&trigger2);
    uint32_t hw_watchdog_timeout = hw_watchdog.get_timeout();
    printf("Hardware Watchdog initialized to %lu ms.\r\n", hw_watchdog_timeout);
    printf("Press button2 at least once every %lu ms to kick the hardtware "
           "watchdog and prevent system reset.\r\n", hw_watchdog_timeout);
    button2.rise(&trigger2);
#endif
    vw.start();
    button.rise(&trigger);

    while (1) {
        printf("\r%1i", countdown--);
        fflush(stdout);
        wait(TIMEOUT_MS / 1000.0 / 10);
    }
}
