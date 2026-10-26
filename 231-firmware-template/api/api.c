#include "api.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "device.h"
#include "profiling.h"

typedef void (*api_callback_t)(const command_t *command);
typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;

static volatile double pi_result;

static bool parse_u32(const char *text, uint32_t *value)
{
    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || number > UINT32_MAX) return false;
    *value = (uint32_t)number;
    return true;
}

static double calc_pi(uint32_t terms)
{
    double sum = 0.0;
    double sign = 1.0;
    for (uint32_t k = 0; k < terms; k++)
    {
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }
    return sum * 4.0;
}

static void cmd_info(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage info\n"); return; }
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage uptime\n"); return; }
    printf("uptime: %llu ms\n", time_us_64() / 1000u);
}

static void cmd_calc_pi(const command_t *command)
{
    uint32_t terms = 1000000;
    if (command->argc > 1 || (command->argc == 1 && !parse_u32(command->argv[0], &terms)))
    {
        printf("error: usage calc_pi [terms]\n");
        return;
    }
    if (terms == 0) { printf("error: terms must be greater than zero\n"); return; }
    uint64_t start_us = time_us_64();
    pi_result = calc_pi(terms);
    printf("pi: %.9f\n", pi_result);
    printf("time: %llu ms\n", (time_us_64() - start_us) / 1000u);
}

static void cmd_main_time_exec(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage main_time_exec\n"); return; }
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

static void cmd_main_time_reset(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage main_time_reset\n"); return; }
    profiling_reset_max();
    printf("max reset\n");
}

static const api_command_t commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi},
    {"main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec},
    {"main_time_reset", "reset maximum iteration", cmd_main_time_reset},
};

static void print_help(void)
{
    printf("%-16s %s\n", "help", "list of commands");
    for (uint32_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
        printf("%-16s %s\n", commands[i].name, commands[i].help);
}

void api_handle(const command_t *command)
{
    if (command->truncated)
    {
        printf("error: command longer than 63 characters or 4 arguments\n");
        return;
    }
    if (strcmp(command->name, "help") == 0)
    {
        print_help();
        return;
    }
    for (uint32_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (strcmp(command->name, commands[i].name) == 0)
        {
            commands[i].callback(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}
