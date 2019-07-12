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

/** \addtogroup hal_GeneralSerial_tests */
/** @{*/

#ifndef MBED_FPGA_UART_TEST_H
#define MBED_FPGA_UART_TEST_H

#if DEVICE_UART

#ifdef __cplusplus
extern "C" {
#endif

/** Test that the uart can be initialized/de-initialized using all possible
 *  uart pins.
 *
 * Given board provides uart support.
 * When uart is initialized (and then de-initialized) using valid set of uart pins.
 * Then the operation is successfull.
 *
 */
void fpga_uart_init_free_test(PinName tx, PinName rx, PinName cts = NC, PinName rts = NC);

/** Test that the uart transfer can be performed in various configurations.
 *
 * Given board provides uart support.
 * When uart transmission is performed using different settings.
 * Then data is successfully transfered.
 *
 */
void fpga_uart_test_common(PinName tx, PinName rx, PinName cts, PinName rts);



/**@}*/

#ifdef __cplusplus
}
#endif

#endif

#endif

/**@}*/
