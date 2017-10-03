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
#include "rtos.h"
#include "unity.h"

#define NUM_TIMEOUTS 16
const float TEST_DELAY_S = 0.01;
const uint32_t TEST_DELAY_MS = 1000.0F * TEST_DELAY_S;
const us_timestamp_t TEST_DELAY_US = 1000000.0F * TEST_DELAY_S;

/* Timeouts are quite arbitrary due to large number of boards with varying level of accuracy */
#define LONG_DELTA_US (100000)
#define SHORT_DELTA_US (600)

void sem_callback(Semaphore *sem)
{
    sem->release();
}

void cnt_callback(volatile uint32_t *cnt)
{
    (*cnt)++;
}

template<typename TimeoutType>
class AttachTester: public TimeoutType {
public:
    void attach_callback(Callback<void()> func, us_timestamp_t delay_us)
    {
        TimeoutType::attach(func, (float) delay_us / 1000000.0f);
    }
};

template<typename TimeoutType>
class AttachUSTester: public TimeoutType {
public:
    void attach_callback(Callback<void()> func, us_timestamp_t delay_us)
    {
        TimeoutType::attach_us(func, delay_us);
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
void test_single_call(void)
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

/** Template for tests: accuracy of timeout delay
 *
 * Test delay accuracy
 * Given a Timeout object with a callback attached with @a attach()
 * When the callback is called
 * Then elapsed time matches given delay
 *
 * Test delay accuracy
 * Given a Timeout object with a callback attached with @a attach_us()
 * When the callback is called
 * Then elapsed time matches given delay
 */
template<typename T, us_timestamp_t delay_us, us_timestamp_t delta_us>
void test_delay_accuracy(void)
{
    Semaphore sem(0, 1);
    T timeout;
    Timer timer;

    timer.start();
    timeout.attach_callback(mbed::callback(sem_callback, &sem), delay_us);

    int32_t sem_slots = sem.wait(osWaitForever);
    timer.stop();
    TEST_ASSERT_EQUAL(1, sem_slots);
    TEST_ASSERT_UINT64_WITHIN(delta_us, delay_us, timer.read_high_resolution_us());

    timeout.detach();
}

#if DEVICE_SLEEP
/** Template for tests: timeout during sleep
 *
 * Test timeout during sleep
 * Given a Timeout object with a callback attached with @a attach()
 *     and the uC in a sleep mode
 * When given delay elapses
 * Then the callback is called
 *     and elapsed time matches given delay
 *
 * Test timeout during sleep
 * Given a Timeout object with a callback attached with @a attach_us()
 *     and the uC in a sleep mode
 * When given delay elapses
 * Then the callback is called
 *     and elapsed time matches given delay
 */
template<typename T, us_timestamp_t delay_us, us_timestamp_t delta_us>
void test_sleep(void)
{
    Semaphore sem(0, 1);
    T timeout;
    Timer timer;

    timer.start();
    sleep_manager_lock_deep_sleep();
    timeout.attach_callback(mbed::callback(sem_callback, &sem), delay_us);

    sleep();
    int32_t sem_slots = sem.wait(osWaitForever);
    timer.stop();
    sleep_manager_unlock_deep_sleep();
    TEST_ASSERT_EQUAL(1, sem_slots);
    TEST_ASSERT_UINT64_WITHIN(delta_us, delay_us, timer.read_high_resolution_us());

    timeout.detach();
}

/** Template for tests: timeout during deepsleep
 *
 * Test timeout during deepsleep
 * Given a LowPowerTimeout object with a callback attached with @a attach()
 *     and the uC in a deepsleep mode
 * When given delay elapses
 * Then the callback is called
 *     and elapsed time matches given delay
 *
 * Test timeout during deepsleep
 * Given a LowPowerTimeout object with a callback attached with @a attach_us()
 *     and the uC in a deepsleep mode
 * When given delay elapses
 * Then the callback is called
 *     and elapsed time matches given delay
 */
template<typename T, us_timestamp_t delay_us, us_timestamp_t delta_us>
void test_deepsleep(void)
{
    Semaphore sem(0, 1);
    T timeout;
    LowPowerTimer timer;

    /*
     * Since deepsleep() may shut down the UART peripheral, we wait for 10ms
     * to allow for hardware serial buffers to completely flush.

     * This should be replaced with a better function that checks if the
     * hardware buffers are empty. However, such an API does not exist now,
     * so we'll use the wait_ms() function for now.
     */
    wait_ms(10);

    /*
     * We use here the low power timer instead of microsecond timer for start and
     * end because the microseconds timer might be disabled during deepsleep.
     */
    timer.start();
    timeout.attach_callback(mbed::callback(sem_callback, &sem), delay_us);

    deepsleep();
    int32_t sem_slots = sem.wait(osWaitForever);
    timer.stop();
    TEST_ASSERT_EQUAL(1, sem_slots);
    TEST_ASSERT_UINT64_WITHIN(delta_us, delay_us, timer.read_high_resolution_us());

    timeout.detach();
}
#endif
