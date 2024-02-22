#include <stdio.h>
#include <string.h>
#include "etimer.h"

//
// Tests
//
const char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0, suites_empty = 0;
int tests_in_suite = 0, tests_run = 0, tests_failed = 0;

#define QUOTE(str) #str
#define ASSERT(x)                                                                                  \
    {                                                                                              \
        tests_run++;                                                                               \
        tests_in_suite++;                                                                          \
        if (!(x))                                                                                  \
        {                                                                                          \
            printf("failed assert [%s:%i] %s\n", __FILE__, __LINE__, QUOTE(x));                    \
            suite_pass = 0;                                                                        \
            tests_failed++;                                                                        \
        }                                                                                          \
    }

void SUITE_START(const char *name)
{
    suite_pass = 1;
    suite_name = name;
    suites_run++;
    tests_in_suite = 0;
}

void SUITE_END(void)
{
    printf("Testing %s ", suite_name);
    size_t suite_i;
    for (suite_i = strlen(suite_name); suite_i < 80 - 8 - 5; suite_i++)
        printf(".");
    printf("%s\n", suite_pass ? " pass" : " fail");
    if (!suite_pass)
        suites_failed++;
    if (!tests_in_suite)
        suites_empty++;
}

void test_etimer_past(void)
{
    SUITE_START("test_etimer_past");

    // check code.
    uint32_t time0 = 20;
    uint32_t time1 = 10;
    uint8_t past = etimer_past(time0, time1);
    uint8_t expect_past = 0;
    ASSERT(past == expect_past);

    time0 = 10;
    time1 = 20;
    past = etimer_past(time0, time1);
    expect_past = 1;
    ASSERT(past == expect_past);

    time0 = 0xfffffff0;
    time1 = 10;
    past = etimer_past(time0, time1);
    expect_past = 1;
    ASSERT(past == expect_past);

    time0 = 10;
    time1 = 0xfffffff0;
    past = etimer_past(time0, time1);
    expect_past = 0;
    ASSERT(past == expect_past);

    time0 = 10 + ETIMER32_MAX_VALUE_OVERFLOW - 1;
    time1 = 10;
    past = etimer_past(time0, time1);
    expect_past = 0;
    ASSERT(past == expect_past);

    time0 = 10;
    time1 = 10 + ETIMER32_MAX_VALUE_OVERFLOW - 1;
    past = etimer_past(time0, time1);
    expect_past = 1;
    ASSERT(past == expect_past);

    // Becareful here, uint32_t to int32_t will overflow, should work with _time_past
    time0 = 10 + ETIMER32_MAX_VALUE_OVERFLOW + 1;
    time1 = 10;
    past = etimer_past(time0, time1);
    expect_past = 1;
    ASSERT(past == expect_past);

    // Becareful here, uint32_t to int32_t will overflow, should work with _time_past
    time0 = 10;
    time1 = 10 + ETIMER32_MAX_VALUE_OVERFLOW + 1;
    past = etimer_past(time0, time1);
    expect_past = 0;
    ASSERT(past == expect_past);

    SUITE_END();
}

void test_etimer_sub(void)
{
    SUITE_START("test_etimer_sub");

    // check code.
    uint32_t time0 = 20;
    uint32_t time1 = 10;
    int32_t diff = etimer_sub(time0, time1);
    int32_t expect_diff = 10;
    ASSERT(diff == expect_diff);

    time0 = 10;
    time1 = 20;
    diff = etimer_sub(time0, time1);
    expect_diff = -10;
    ASSERT(diff == expect_diff);

    time0 = 0xfffffff0;
    time1 = 10;
    diff = etimer_sub(time0, time1);
    expect_diff = -26;
    ASSERT(diff == expect_diff);

    time0 = 10;
    time1 = 0xfffffff0;
    diff = etimer_sub(time0, time1);
    expect_diff = 26;
    ASSERT(diff == expect_diff);

    time0 = 10 + ETIMER32_MAX_VALUE_OVERFLOW - 1;
    time1 = 10;
    diff = etimer_sub(time0, time1);
    expect_diff = ETIMER32_MAX_VALUE_OVERFLOW - 1;
    ASSERT(diff == expect_diff);

    time0 = 10;
    time1 = 10 + ETIMER32_MAX_VALUE_OVERFLOW - 1;
    diff = etimer_sub(time0, time1);
    expect_diff = -(ETIMER32_MAX_VALUE_OVERFLOW - 1);
    ASSERT(diff == expect_diff);

    // Becareful here, uint32_t to int32_t will overflow, should work with _time_past
    time0 = 10 + ETIMER32_MAX_VALUE_OVERFLOW + 1;
    time1 = 10;
    diff = etimer_sub(time0, time1);
    expect_diff = -(ETIMER32_MAX_VALUE + 1 - (ETIMER32_MAX_VALUE_OVERFLOW + 1));
    ASSERT(diff == expect_diff);

    // Becareful here, uint32_t to int32_t will overflow, should work with _time_past
    time0 = 10;
    time1 = 10 + ETIMER32_MAX_VALUE_OVERFLOW + 1;
    diff = etimer_sub(time0, time1);
    expect_diff = (ETIMER32_MAX_VALUE + 1 - (ETIMER32_MAX_VALUE_OVERFLOW + 1));
    ASSERT(diff == expect_diff);

    SUITE_END();
}

/**
 * @brief  Check two absolute times past: timer1<timer2.
 * @param[in]  timer1: Absolute time expressed in internal time units.
 * @param[in]  timer2: Absolute time expressed in internal time units.
 * @return resulting 1 means past(timer1<timer2).
 */
int timer_past(uint32_t timer1, uint32_t timer2)
{
    return timer1 < timer2;
}
/**
 * @brief  Returns the difference between two absolute times: timer1-timer2.
 * @param[in]  timer1: Absolute time expressed in internal time units.
 * @param[in]  timer2: Absolute time expressed in internal time units.
 * @return resulting signed relative time expressed in internal time units.
 */
int32_t timer_sub(uint32_t timer1, uint32_t timer2)
{
    return timer1 - timer2;
}

/**
 * @brief This function returns the sum of an absolute time and a signed relative time.
 * @param[in]  timer1: Absolute time expressed in internal time units.
 * @param[in]  ticks: Signed relative time expressed in internal time units.
 * @return 32bit resulting absolute time expressed in internal time units.
 */
uint32_t timer_add(uint32_t timer1, int32_t ticks)
{
    return timer1 + ticks;
}

void test_work(void)
{
    SUITE_START("test_work");

    uint32_t A = 0xFFFFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = timer_past(A, B); // ERROR, Get res=0, Expect res=1;
    ASSERT(res == 1);
    res = timer_past(B, C); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = timer_add(A, 0x20); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = timer_add(B, 0x10); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = timer_add(C, -0x10); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = timer_sub(B, A); // SUCCESS, Get diff=0x20, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = timer_sub(C, B); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}

void test_work_insuff(void)
{
    SUITE_START("test_work_insuff");

    uint32_t A = 0x00FFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = timer_past(A, B); // ERROR, Get res=0, Expect res=1;
    ASSERT(res == 1);
    res = timer_past(B, C); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = timer_add(A, 0x20); // ERROR, Get tmp=0x01000010, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = timer_add(B, 0x10); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = timer_add(C, -0x10); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = timer_sub(B, A); // ERROR, Get diff=0xFF000020, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = timer_sub(C, B); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}

void test_work_etimer(void)
{
    SUITE_START("test_work_etimer");

    uint32_t A = 0xFFFFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = etimer_past(A, B); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);
    res = etimer_past(B, C); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = etimer_add(A, 0x20); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = etimer_add(B, 0x10); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = etimer_add(C, -0x10); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = etimer_sub(B, A); // SUCCESS, Get diff=0x20, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = etimer_sub(C, B); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}

void test_work_etimer_insuff(void)
{
    SUITE_START("test_work_etimer_insuff");

    uint32_t A = 0x00FFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    uint32_t max_value = 0x00FFFFFF;
    uint32_t overflow = max_value / 2;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = etimer_past_raw(A, B, overflow); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);
    res = etimer_past_raw(B, C, overflow); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = etimer_add_raw(A, 0x20, max_value); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = etimer_add_raw(B, 0x10, max_value); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = etimer_add_raw(C, -0x10, max_value); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = etimer_sub_raw(B, A, overflow, max_value); // SUCCESS, Get diff=0x20, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = etimer_sub_raw(C, B, overflow, max_value); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}

int main(void)
{
    // normal process test
    test_work();
    test_work_insuff();

    // normal process test - etimer
    test_work_etimer();
    test_work_etimer_insuff();

    // special sense process test - etimer
    test_etimer_past();
    test_etimer_sub();

    return 0;
}