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
#ifdef DEVICE_WATCHDOG

#include "VirtualWatchdog.h"
#include "Watchdog.h"

namespace mbed {

VirtualWatchdog *VirtualWatchdog::_first = NULL;

bool VirtualWatchdog::_hw_initialized = false;

VirtualWatchdog::VirtualWatchdog(uint32_t timeout, const char *const str): _name(str)
{
    _current_count = 0;
    _is_initialized = false;
    _next = NULL;
    _max_timeout = timeout;
    // start watchdog
    Watchdog &watchdog = Watchdog::get_instance();
    // core_util_critical_section_enter();
    if (!_hw_initialized) {
        // FIXME throw an error instad?
        MBED_ASSERT(watchdog.is_running() == false);
        // FIXME need to sort out the relation of timeout and Watchdog::start(timeout)
        MBED_ASSERT(watchdog.start(&VirtualWatchdog::process, MBED_CONF_TARGET_WATCHDOG_TIMEOUT));
        _hw_initialized = true;
    }
    // core_util_critical_section_exit();
}

VirtualWatchdog::~VirtualWatchdog()
{
    if (_is_initialized) {
        stop();
        // we do not need to stop hw watchdog, it's ticking by itself
    }
}

void VirtualWatchdog::start()
{
    MBED_ASSERT(!_is_initialized);
    core_util_critical_section_enter();
    add_to_list();
    core_util_critical_section_exit();
}


void VirtualWatchdog::kick()
{
    MBED_ASSERT(_is_initialized);
    core_util_critical_section_enter();
    _current_count = 0;
    core_util_critical_section_exit();
}

void VirtualWatchdog::stop()
{
    MBED_ASSERT(_is_initialized);
    core_util_critical_section_enter();
    remove_from_list();
    core_util_critical_section_exit();
}

void VirtualWatchdog::add_to_list()
{
    this->_next = _first;
    _first = this;
    _is_initialized =  true;
}

void VirtualWatchdog::remove_from_list()
{
    VirtualWatchdog *cur_ptr = _first,
                     *prev_ptr = NULL;
    while (cur_ptr != NULL) {
        if (cur_ptr == this) {
            if (cur_ptr == _first) {
                prev_ptr = _first;
                _first = cur_ptr->_next;
                prev_ptr->_next = NULL;
            } else {
                prev_ptr->_next = cur_ptr->_next;
                cur_ptr->_next = NULL;
            }
            _is_initialized = false;
            break;
        } else {
            prev_ptr = cur_ptr;
            cur_ptr = cur_ptr->_next;
        }
    }
}

void VirtualWatchdog::process(uint32_t elapsed_ms)
{
    VirtualWatchdog *cur_ptr =  _first;
    while (cur_ptr != NULL) {
        if (cur_ptr->_is_initialized) {
            if (cur_ptr->_current_count > cur_ptr->_max_timeout) {
                system_reset();
            } else {
                cur_ptr->_current_count += elapsed_ms;
            }
        }
        cur_ptr = cur_ptr->_next;
    }
}


} // namespace mbed

#endif // DEVICE_WATCHDOG
