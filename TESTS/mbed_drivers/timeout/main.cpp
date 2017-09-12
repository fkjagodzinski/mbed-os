/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
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
#include "mbed.h"
#include "greentea-client/test_env.h"
#include "unity.h"
#include "utest.h"
#include "timeout_tests.h"

using namespace utest::v1;

#define PERIOD_US 10000
volatile uint32_t callback_trigger_count = 0;
static const int test_timeout = 240;
Timeout timeout;

void set_increment_count()
{
    timeout.attach_us(set_increment_count, PERIOD_US);
    ++callback_trigger_count;
}

/*
 * Tests is to measure the accuracy of Timeout over a period of time
 *
 *
 * 1) DUT would start to update callback_trigger_count every milli sec
 * 2) Host would query what is current count base_time, Device responds by the callback_trigger_count
 * 3) Host after waiting for measurement stretch. It will query for device time again final_time.
 * 4) Host computes the drift considering base_time, final_time, transport delay and measurement stretch
 * 5) Finally host send the results back to device pass/fail based on tolerance.
 * 6) More details on tests can be found in timing_drift_auto.py
 *
 */
void test_case_timeout()
{
    char _key[11] = { };
    char _value[128] = { };
    int expected_key = 1;

    greentea_send_kv("timing_drift_check_start", 0);
    timeout.attach_us(set_increment_count, PERIOD_US);

    // wait for 1st signal from host
    do {
        greentea_parse_kv(_key, _value, sizeof(_key), sizeof(_value));
        expected_key = strcmp(_key, "base_time");
    } while (expected_key);

    greentea_send_kv(_key, callback_trigger_count * PERIOD_US);

    // wait for 2nd signal from host
    greentea_parse_kv(_key, _value, sizeof(_key), sizeof(_value));
    greentea_send_kv(_key, callback_trigger_count * PERIOD_US);

    //get the results from host
    greentea_parse_kv(_key, _value, sizeof(_key), sizeof(_value));

    TEST_ASSERT_EQUAL_STRING_MESSAGE("pass", _key, "Host side script reported a fail...");
}

Case cases[] = {
    Case("Callback called once (with attach)", test_single_call<AttachTester<Timeout> >),
    Case("Callback called once (with attach_us)", test_single_call<AttachUSTester<Timeout> >),

    Case("Callback not called when cancelled (with attach)", test_cancel<AttachUSTester<Timeout> >),
    Case("Callback not called when cancelled (with attach_us)", test_cancel<AttachUSTester<Timeout> >),

    Case("Callback override (with attach)", test_override<AttachTester<Timeout> >),
    Case("Callback override (with attach_us)", test_override<AttachUSTester<Timeout> >),

    Case("Multiple timeouts running in parallel (with attach)", test_multiple<AttachTester<Timeout> >),
    Case("Multiple timeouts running in parallel (with attach_us)", test_multiple<AttachUSTester<Timeout> >),

    Case("Zero delay (with attach)", test_no_wait<AttachTester<Timeout> >),
    Case("Zero delay (with attach_us)", test_no_wait<AttachUSTester<Timeout> >),

    Case("Timers: toggle on/off", test_case_timeout)
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases)
{
    GREENTEA_SETUP(test_timeout, "timing_drift_auto");
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main()
{
    Harness::run(specification);
}
