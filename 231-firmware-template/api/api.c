#include "api.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "device.h"
#include "profiling.h"
#include "led-task.h"
#include "button-task.h"

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

static const char *led_state_name(void)
{
    switch (led_task_get_state())
    {
    case LED_STATE_OFF: return "off";
    case LED_STATE_ON: return "on";
    case LED_STATE_BLINK: return "blink";
    }
    return "unknown";
}

static void print_led(void)
{
    printf("led: %s, period %u ms\n", led_state_name(), (unsigned)led_task_get_period_ms());
}

static void cmd_led_enable(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage led_enable\n"); return; }
    led_task_set_state(LED_STATE_ON); print_led();
}
static void cmd_led_disable(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage led_disable\n"); return; }
    led_task_set_state(LED_STATE_OFF); print_led();
}
static void cmd_led_blink(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage led_blink\n"); return; }
    led_task_set_state(LED_STATE_BLINK); print_led();
}
static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;
    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    { printf("error: usage led_period <period_ms>\n"); return; }
    if (!led_task_set_period_ms(period_ms))
    { printf("error: period_ms must be greater than 0\n"); return; }
    print_led();
}

static void cmd_button(const command_t *command)
{
    if (command->argc != 0) { printf("error: usage button\n"); return; }
    printf("button: %s, presses %u\n",
           button_task_is_pressed() ? "pressed" : "released",
           (unsigned)button_task_get_press_count());
}

static const api_command_t commands[] = {
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi},
    {"main_time_exec", "superloop iteration: average and maximum", cmd_main_time_exec},
    {"main_time_reset", "reset maximum iteration", cmd_main_time_reset},
    {"led_enable", "turn LED on", cmd_led_enable},
    {"led_disable", "turn LED off", cmd_led_disable},
    {"led_blink", "blink LED", cmd_led_blink},
    {"led_period", "set period: led_period <period_ms>", cmd_led_period},
    {"button", "button state and press count", cmd_button},
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
