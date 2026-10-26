#include "pico/stdlib.h"
#include "led-task.h"
#include "profiling.h"
#include "stdio-text-protocol.h"
#include "api.h"

int main(void)
{
    stdio_init_all();
    led_task_init();
    stdio_text_protocol_init();
    profiling_init();

    while (1)
    {
        profiling_iteration();
        led_task_handle();
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
            api_handle(command);
    }
}
