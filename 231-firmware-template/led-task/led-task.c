#include "led-task.h"
#include "pico/stdlib.h"

static led_state_t led_state;
static uint32_t led_period_ms;
static uint64_t last_toggle_us;

static bool period_elapsed(uint64_t *last_us, uint64_t period_us)
{
    uint64_t now_us = time_us_64();
    if (now_us - *last_us < period_us) return false;
    *last_us += period_us;
    if (now_us - *last_us >= period_us) *last_us = now_us;
    return true;
}

void led_task_init(void)
{
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    led_period_ms = 1000;
    led_task_set_state(LED_STATE_BLINK);
}

void led_task_set_state(led_state_t state)
{
    led_state = state;
    switch (state)
    {
    case LED_STATE_OFF: gpio_put(PICO_DEFAULT_LED_PIN, false); break;
    case LED_STATE_ON: gpio_put(PICO_DEFAULT_LED_PIN, true); break;
    case LED_STATE_BLINK:
        last_toggle_us = time_us_64() - (uint64_t)led_period_ms * 500u;
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
        if (period_elapsed(&last_toggle_us, (uint64_t)led_period_ms * 500u)) gpio_xor_mask(1u << PICO_DEFAULT_LED_PIN);
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
