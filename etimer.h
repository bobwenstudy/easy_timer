#ifndef _ETIMER_H_
#define _ETIMER_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ETIMER32_MAX_VALUE          (~(uint32_t)0)
#define ETIMER32_MAX_VALUE_OVERFLOW (ETIMER32_MAX_VALUE >> 1)

/**
 * @brief  Check two absolute times past with maxvalue/overflow check: time1<time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @param[in]  overflow: Overflow time value.
 * @return resulting 1 means past(time1<time2).
 */
static inline int etimer_past_raw(uint32_t time1, uint32_t time2, uint32_t overflow)
{
    if (time1 <= time2)
    {
        return (time2 - time1) < overflow;
    }

    return (time1 - time2) > overflow;
}

/**
 * @brief  Check two absolute times past: time1<time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @return resulting 1 means past(time1<time2).
 */
static inline int etimer_past(uint32_t time1, uint32_t time2)
{
    return etimer_past_raw(time1, time2, ETIMER32_MAX_VALUE_OVERFLOW);
}

/**
 * @brief This function returns the sum of an absolute time and a signed relative time with
 * maxvalue/overflow check.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  ticks: Signed relative time expressed in internal time units.
 * @param[in]  max_value: Max time value.
 * @return 32bit resulting absolute time expressed in internal time units.
 */
static inline uint32_t etimer_add_raw(uint32_t time1, int32_t ticks, uint32_t max_value)
{
    uint32_t tmp = time1 + ticks;
    while (tmp > max_value)
    {
        tmp -= max_value + 1;
    }

    return tmp;
}

/**
 * @brief This function returns the sum of an absolute time and a signed relative time.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  ticks: Signed relative time expressed in internal time units.
 * @return 32bit resulting absolute time expressed in internal time units.
 */
static inline uint32_t etimer_add(uint32_t time1, int32_t ticks)
{
    return time1 + ticks;
}

/**
 * @brief  Returns the difference between two absolute times with maxvalue/overflow check:
 * time1-time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @param[in]  overflow: Overflow time value.
 * @param[in]  max_value: Max time value.
 * @return resulting signed relative time expressed in internal time units.
 */
static inline int32_t etimer_sub_raw(uint32_t time1, uint32_t time2, uint32_t overflow,
                                     uint32_t max_value)
{
    uint32_t diff;
    if (time1 >= time2)
    {
        diff = time1 - time2;
        if (diff > overflow)
        {
            return diff - max_value - 1;
        }
        return diff;
    }
    diff = time2 - time1;

    if (diff > overflow)
    {
        return max_value + 1 - diff;
    }
    return -diff;
}

/**
 * @brief  Returns the difference between two absolute times: time1-time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @return resulting signed relative time expressed in internal time units.
 */
static inline int32_t etimer_sub(uint32_t time1, uint32_t time2)
{
    return time1 - time2;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ETIMER_H_ */
