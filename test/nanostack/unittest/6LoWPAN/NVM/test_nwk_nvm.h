/*
 * Copyright (c) 2017, Arm Limited and affiliates.
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
#ifndef TEST_NWK_NVM_H
#define TEST_NWK_NVM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

bool test_net_nvm_wpan_params_storage_enable(void);
bool test_net_nvm_wpan_params_storage_disable(void);
bool test_net_nvm_wpan_params_storage_reset(void);
bool test_net_nvm_usage(void);

#ifdef __cplusplus
}
#endif

#endif
