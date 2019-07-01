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
#include "drivers/VirtualWatchdog.h"
#include "platform/mbed_critical.h"

namespace mbed {

VirtualWatchdog::VirtualWatchdog(uint32_t timeout): _reset_timeout_us(timeout * 1000), _is_running(false)
{
}

VirtualWatchdog::~VirtualWatchdog()
{
    core_util_critical_section_enter();
    if (_is_running) {
        _timeout.detach();
    }
    core_util_critical_section_exit();
}

void VirtualWatchdog::start()
{
    core_util_critical_section_enter();
    if (_is_running) {
        core_util_critical_section_exit();
        return;
    }
    _timeout.attach_us(callback(this, &VirtualWatchdog::_timeout_handler), _reset_timeout_us);
    _is_running = true;
    core_util_critical_section_exit();
}

void VirtualWatchdog::kick()
{
    core_util_critical_section_enter();
    if (!_is_running) {
        core_util_critical_section_exit();
        return;
    }
    _timeout.detach();
    _timeout.attach_us(callback(this, &VirtualWatchdog::_timeout_handler), _reset_timeout_us);
    core_util_critical_section_exit();
}

void VirtualWatchdog::stop()
{
    core_util_critical_section_enter();
    if (!_is_running) {
        core_util_critical_section_exit();
        return;
    }
    _is_running = false;
    _timeout.detach();
    core_util_critical_section_exit();
}

void VirtualWatchdog::_timeout_handler()
{
    if (!_is_running) {
        return;
    }
    system_reset();
}

} // namespace mbed
