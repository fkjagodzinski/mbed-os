/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
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

/** \addtogroup hal_gpio_tests */
/** @{*/

#ifndef MBED_FPGA_GPIO_TEST_H
#define MBED_FPGA_GPIO_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/** Test that the gpio works correctly.
 *
 * Given board provides gpio.
 * When gpio port is set as input port and logic 0/1 is provided on this port.
 * Then gpio_read returns 0/1.
 * When gpio port is set as output port and the output value is set to logic 0/1 by means of gpio_write.
 * Then we have logic 0/1 on output.
 *
 */
void fpga_gpio_inout_test();

/**@}*/

#ifdef __cplusplus
}
#endif

#endif


/**@}*/
