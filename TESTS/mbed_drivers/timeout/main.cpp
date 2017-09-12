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
#include "rtos.h"

using namespace utest::v1;

#define NUM_TIMEOUTS 64
const float TEST_DELAY_S = 0.01;
const uint32_t TEST_DELAY_MS = 1000.0F * TEST_DELAY_S;
const us_timestamp_t TEST_DELAY_US = 1000000.0F * TEST_DELAY_S;

void sem_callback(Semaphore *sem)
{
    sem->release();
}

void cnt_callback(volatile uint32_t *cnt)
{
    (*cnt)++;
}

class TimeoutAttachUSTester: public Timeout {
public:
    void attach_callback(Callback<void()> func, us_timestamp_t delay_us)
    {
        attach_us(func, delay_us);
    }
};

class TimeoutAttachTester: public Timeout {
public:
    void attach_callback(Callback<void()> func, us_timestamp_t delay_us)
    {
        attach(func, (float) delay_us / 1000000.0f);
    }
};

/** Template for tests: callback called once
 *
 * Test callback called once
 * Given a Timeout object with a callback attached with @a attach()
 * When given time elapses
 * Then the callback is called exactly one time
 *
 * Test callback called once
 * Given a Timeout object with a callback attached with @a attach_us()
 * When given time elapses
 * Then the callback is called exactly one time
 */
template<typename T>
void test_callback_fires_once(void)
{
    Semaphore sem(0, 1);
    T timeout;

    timeout.attach_callback(mbed::callback(sem_callback, &sem), TEST_DELAY_US);

    int32_t sem_slots = sem.wait(0);
    TEST_ASSERT_EQUAL(0, sem_slots);

    sem_slots = sem.wait(TEST_DELAY_MS + 1);
    TEST_ASSERT_EQUAL(1, sem_slots);

    sem_slots = sem.wait(TEST_DELAY_MS + 1);
    TEST_ASSERT_EQUAL(0, sem_slots);

    timeout.detach();
}

/** Template for tests: callback not called when cancelled
 *
 * Test callback not called when cancelled
 * Given a Timeout object with a callback attached with @a attach()
 * When the callback is detached before being called
 * Then the callback is never called
 *
 * Test callback not called when cancelled
 * Given a Timeout object with a callback attached with @a attach_us()
 * When the callback is detached before being called
 * Then the callback is never called
 */
template<typename T>
void test_cancel(void)
{
    Semaphore sem(0, 1);
    T timeout;

    timeout.attach_callback(mbed::callback(sem_callback, &sem), 2.0f * TEST_DELAY_US);

    int32_t sem_slots = sem.wait(TEST_DELAY_MS);
    TEST_ASSERT_EQUAL(0, sem_slots);
    timeout.detach();

    sem_slots = sem.wait(TEST_DELAY_MS + 1);
    TEST_ASSERT_EQUAL(0, sem_slots);
}

/** Template for tests: callback override
 *
 * Test callback override
 * Given a Timeout object with a callback attached with @a attach()
 * When another callback is attached before first one is called
 *     and second callback's delay elapses
 * Then the second callback is called
 *     and the first callback is never called
 *
 * Test callback override
 * Given a Timeout object with a callback attached with @a attach_us()
 * When another callback is attached before first one is called
 *     and second callback's delay elapses
 * Then the second callback is called
 *     and the first callback is never called
 */
template<typename T>
void test_override(void)
{
    Semaphore sem1(0, 1);
    Semaphore sem2(0, 1);
    T timeout;

    timeout.attach_callback(mbed::callback(sem_callback, &sem1), 2.0f * TEST_DELAY_US);

    int32_t sem_slots = sem1.wait(TEST_DELAY_MS);
    TEST_ASSERT_EQUAL(0, sem_slots);
    timeout.attach_callback(mbed::callback(sem_callback, &sem2), 2.0f * TEST_DELAY_US);

    sem_slots = sem2.wait(2 * TEST_DELAY_MS + 1);
    TEST_ASSERT_EQUAL(1, sem_slots);
    sem_slots = sem1.wait(0);
    TEST_ASSERT_EQUAL(0, sem_slots);

    timeout.detach();
}

/** Template for tests: multiple Timeouts
 *
 * Test multiple Timeouts
 * Given multiple separate Timeout objects
 * When a callback is attached to all of these Timeout objects with @a attach()
 *     and delay for every Timeout elapses
 * Then all callbacks are called
 *
 * Test multiple Timeouts
 * Given multiple separate Timeout objects
 * When a callback is attached to all of these Timeout objects with @a attach_us()
 *     and delay for every Timeout elapses
 * Then all callbacks are called
 */
template<typename T>
void test_multiple(void)
{
    volatile uint32_t callback_count = 0;
    T timeouts[NUM_TIMEOUTS];
    for (size_t i = 0; i < NUM_TIMEOUTS; i++) {
        timeouts[i].attach_callback(mbed::callback(cnt_callback, &callback_count), TEST_DELAY_US);
    }
    Thread::wait(TEST_DELAY_MS + 1);
    TEST_ASSERT_EQUAL(NUM_TIMEOUTS, callback_count);
}

/** Template for tests: zero delay
 *
 * Test zero delay
 * Given a Timeout object
 * When a callback is attached with 0.0 s delay, with @a attach()
 * Then the callback is called instantly
 *
 * Test zero delay
 * Given a Timeout object
 * When a callback is attached with 0.0 s delay, with @a attach_us()
 * Then the callback is called instantly
 */
template<typename T>
void test_no_wait(void)
{
    Semaphore sem(0, 1);
    T timeout;
    timeout.attach_callback(mbed::callback(sem_callback, &sem), 0ULL);

    int32_t sem_slots = sem.wait(0);
    TEST_ASSERT_EQUAL(1, sem_slots);
    timeout.detach();
}

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
    Case("Test callback called once (with attach)", test_callback_fires_once<TimeoutAttachTester>),
    Case("Test callback called once (with attach_us)", test_callback_fires_once<TimeoutAttachUSTester>),

    Case("Test callback not called when cancelled (with attach)", test_cancel<TimeoutAttachTester>),
    Case("Test callback not called when cancelled (with attach_us)", test_cancel<TimeoutAttachUSTester>),

    Case("Test callback override (with attach)", test_override<TimeoutAttachTester>),
    Case("Test callback override (with attach_us)", test_override<TimeoutAttachUSTester>),

    Case("Test multiple timeouts running in parallel (with attach)", test_multiple<TimeoutAttachTester>),
    Case("Test multiple timeouts running in parallel (with attach_us)", test_multiple<TimeoutAttachUSTester>),

    Case("Test zero delay (with attach)", test_no_wait<TimeoutAttachTester>),
    Case("Test zero delay (with attach_us)", test_no_wait<TimeoutAttachUSTester>),

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
