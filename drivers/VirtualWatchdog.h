/*
 * Copyright (c) 2019 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MBED_VIRTUAL_WATCHDOG_H
#define MBED_VIRTUAL_WATCHDOG_H

#include "platform/NonCopyable.h"

#if DEVICE_LPTICKER
#include "drivers/LowPowerTimeout.h"
#else
#include "drivers/Timeout.h"
#endif
namespace mbed {

/** \addtogroup drivers */
/**
 * A software watchdog timer that will reset the system if not kicked within a
 * given timeout.
 *
 * Multiple instances of this class may exist independently. Use these software
 * watchdogs for applications where you use multiple services requiring
 * watchdog functionality.
 *
 * For the watchdog peripheral driver, see drivers/Watchdog.h. This driver
 * provides an interface to the watchdog hardware.
 *
 * @see Watchdog
 *
 * @note
 * You may use multiple instances of VirtualWatchdog (for multiple software
 * services) together with one hardware Watchdog (for general system failures).
 *
 * Example:
 * @code
 *
 * VirtualWatchdog virtual_watchdog(300);
 * virtual_watchdog.start();
 *
 * while (true) {
 *    virtual_watchdog.kick();
 *    // Application code
 * }
 * @endcode
 * @ingroup drivers
 */
class VirtualWatchdog : private NonCopyable<VirtualWatchdog> {
public:
    /** Constructor for this software watchdog instance.
     *
     *  @param timeout The timeout of this software watchdog in milliseconds.
     */
    VirtualWatchdog(uint32_t timeout = 1000);

    ~VirtualWatchdog();

    /** Start this software watchdog timer with parameters specified in the constructor.
     *
     * If this software watchdog is already running, this function does nothing.
     */
    void start();

    /** Stop this software watchdog timer.
     *
     * If this software watchdog is not running, this function does nothing.
     */
    void stop();

    /** Refresh this software watchdog timer.
     *
     * Call this function periodically before the watchdog times out.
     * Otherwise, the system resets.
     *
     * If this software watchdog is not running, this function does nothing.
     */
    void kick();

private:
    void _timeout_handler();
    us_timestamp_t _reset_timeout_us;
    bool _is_running;
#if DEVICE_LPTICKER
    LowPowerTimeout _timeout;
#else
    Timeout _timeout;
#endif
};

} // namespace mbed

#endif // MBED_VIRTUAL_WATCHDOG_H
