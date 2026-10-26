#include "platform.h"
#include "pico/stdlib.h"

#define LED_PIN PICO_DEFAULT_LED_PIN

static const uint BUTTON_PINS[PLATFORM_BUTTON_COUNT] = {15, 14};

void platform_init(void)
{
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    for (platform_button_t button = 0; button < PLATFORM_BUTTON_COUNT; button++)
    {
        gpio_init(BUTTON_PINS[button]);
        gpio_set_dir(BUTTON_PINS[button], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[button]);
    }
}

void platform_led_set(bool on) { gpio_put(LED_PIN, on); }
bool platform_button_read(platform_button_t button) { return !gpio_get(BUTTON_PINS[button]); }
