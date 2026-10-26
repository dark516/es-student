#include "pi-task.h"
#include "pico/stdlib.h"

// Один член ряда занимает около 4,7 мкс; 100 членов укладываются в 0,5 мс.
#define PI_TERMS_PER_CALL 100

static pi_state_t pi_state = PI_STATE_NOT_STARTED;
static uint32_t pi_k;
static uint32_t pi_terms;
static double pi_sum;
static double pi_sign;
static double pi_result;
static uint64_t pi_started_us;
static uint64_t pi_elapsed_ms;

bool pi_task_start(uint32_t terms)
{
    if (pi_state == PI_STATE_RUNNING) return false;
    pi_k = 0;
    pi_terms = terms;
    pi_sum = 0.0;
    pi_sign = 1.0;
    pi_started_us = time_us_64();
    pi_state = PI_STATE_RUNNING;
    return true;
}

void pi_task_handle(void)
{
    if (pi_state != PI_STATE_RUNNING) return;

    uint64_t end64 = (uint64_t)pi_k + PI_TERMS_PER_CALL;
    uint32_t end = end64 > pi_terms ? pi_terms : (uint32_t)end64;
    for (; pi_k < end; pi_k++)
    {
        pi_sum += pi_sign / (2.0 * pi_k + 1.0);
        pi_sign = -pi_sign;
    }
    if (pi_k == pi_terms)
    {
        pi_result = 4.0 * pi_sum;
        pi_elapsed_ms = (time_us_64() - pi_started_us) / 1000u;
        pi_state = PI_STATE_READY;
    }
}

bool pi_task_get_result(double *result, uint64_t *elapsed_ms)
{
    if (pi_state != PI_STATE_READY) return false;
    *result = pi_result;
    *elapsed_ms = pi_elapsed_ms;
    return true;
}

pi_state_t pi_task_get_state(void) { return pi_state; }
