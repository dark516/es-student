#include "button-task/button-task.h"
#include <stddef.h>
#include "platform/platform.h"
#include "systime/systime.h"

#define BUTTON_DEBOUNCE_US 20000

typedef enum
{
    BUTTON_STATE_RELEASED,
    BUTTON_STATE_PRESSING,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_RELEASING,
} button_state_t;

static button_state_t button_states[PLATFORM_BUTTON_COUNT];
static uint64_t transition_started_us[PLATFORM_BUTTON_COUNT];
static uint32_t press_counts[PLATFORM_BUTTON_COUNT];
static button_task_callback_t press_callbacks[PLATFORM_BUTTON_COUNT];

void button_task_init(platform_button_t button, button_task_callback_t on_press)
{
    press_callbacks[button] = on_press;
    button_states[button] = platform_button_read(button) ? BUTTON_STATE_PRESSED : BUTTON_STATE_RELEASED;
    transition_started_us[button] = systime_us();
    press_counts[button] = 0;
}

static void button_handle(platform_button_t button, uint64_t now_us)
{
    bool pressed = platform_button_read(button);

    switch (button_states[button])
    {
    case BUTTON_STATE_RELEASED:
        if (pressed)
        {
            transition_started_us[button] = now_us;
            button_states[button] = BUTTON_STATE_PRESSING;
        }
        break;
    case BUTTON_STATE_PRESSING:
        if (!pressed)
            button_states[button] = BUTTON_STATE_RELEASED;
        else if (now_us - transition_started_us[button] >= BUTTON_DEBOUNCE_US)
        {
            button_states[button] = BUTTON_STATE_PRESSED;
            press_counts[button]++;
            if (press_callbacks[button] != NULL) press_callbacks[button]();
        }
        break;
    case BUTTON_STATE_PRESSED:
        if (!pressed)
        {
            transition_started_us[button] = now_us;
            button_states[button] = BUTTON_STATE_RELEASING;
        }
        break;
    case BUTTON_STATE_RELEASING:
        if (pressed)
            button_states[button] = BUTTON_STATE_PRESSED;
        else if (now_us - transition_started_us[button] >= BUTTON_DEBOUNCE_US)
            button_states[button] = BUTTON_STATE_RELEASED;
        break;
    }
}

void button_task_handle(void)
{
    uint64_t now_us = systime_us();
    for (platform_button_t button = 0; button < PLATFORM_BUTTON_COUNT; button++)
        button_handle(button, now_us);
}

bool button_task_is_pressed(platform_button_t button)
{
    return button_states[button] == BUTTON_STATE_PRESSED ||
           button_states[button] == BUTTON_STATE_RELEASING;
}

uint32_t button_task_get_press_count(platform_button_t button) { return press_counts[button]; }
