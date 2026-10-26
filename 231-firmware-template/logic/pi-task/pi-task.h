#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PI_STATE_NOT_STARTED,
    PI_STATE_RUNNING,
    PI_STATE_READY,
} pi_state_t;

bool pi_task_start(uint32_t terms);
void pi_task_handle(void);
bool pi_task_get_result(double *result, uint64_t *elapsed_ms);
pi_state_t pi_task_get_state(void);
