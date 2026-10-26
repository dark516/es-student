#pragma once
#include <stdbool.h>
#include <stdint.h>

#define PROFILING_STOPWATCH_COUNT 5

// 32-битная сумма длительностей переполняется примерно через 71 минуту.
typedef struct {
    const char *name;
    uint32_t count;
    float mean_us;
    uint32_t max_us;
} profiling_result_t;

void profiling_stopwatch_init(uint32_t id, const char *name);
void profiling_start(uint32_t id);
void profiling_stop(uint32_t id);
bool profiling_get(uint32_t id, profiling_result_t *result);
void profiling_reset(void);
