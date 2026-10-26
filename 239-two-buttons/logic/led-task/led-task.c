#include "led-task/led-task.h"
#include "platform/platform.h"
#include "systime/systime.h"

static led_state_t led_state;
static uint32_t led_period_ms;
static uint64_t last_toggle_us;
static bool led_on;

void led_task_init(void)
{
    led_period_ms = 1000;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (state)
    {
    case LED_STATE_OFF: led_on = false; platform_led_set(led_on); break;
    case LED_STATE_ON: led_on = true; platform_led_set(led_on); break;
    case LED_STATE_BLINK:
        last_toggle_us = systime_us() - (uint64_t)led_period_ms * 500u;
        break;
    }
}

void led_task_handle(void)
{
    switch (led_state)
    {
    case LED_STATE_OFF:
    case LED_STATE_ON:
        break;
    case LED_STATE_BLINK:
        if (systime_period_elapsed(&last_toggle_us, (uint64_t)led_period_ms * 500u))
        {
            led_on = !led_on;
            platform_led_set(led_on);
        }
        break;
    }
}

led_state_t led_task_get_state(void) { return led_state; }
bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms == 0) return false;
    led_period_ms = period_ms;
    return true;
}
uint32_t led_task_get_period_ms(void) { return led_period_ms; }

void led_task_next_state(void)
{
    switch (led_state)
    {
    case LED_STATE_OFF: led_task_set_state(LED_STATE_ON); break;
    case LED_STATE_ON: led_task_set_state(LED_STATE_BLINK); break;
    case LED_STATE_BLINK: led_task_set_state(LED_STATE_OFF); break;
    }
}

void led_task_next_period(void)
{
    switch (led_period_ms)
    {
    case 250: led_period_ms = 500; break;
    case 500: led_period_ms = 1000; break;
    default: led_period_ms = 250; break;
    }
}
