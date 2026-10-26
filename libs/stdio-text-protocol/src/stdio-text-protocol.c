#include "stdio-text-protocol/stdio-text-protocol.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include "pico/stdlib.h"

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint32_t line_length;
static bool line_truncated;
static command_t command;

void stdio_text_protocol_init(void)
{
    line_length = 0;
    line_truncated = false;
}

static const command_t *finish_line(void)
{
    line[line_length] = '\0';
    command.name = NULL;
    command.argc = 0;
    command.truncated = line_truncated;
    char *cursor = line;
    while (*cursor != '\0')
    {
        while (*cursor == ' ' || *cursor == '\t') *cursor++ = '\0';
        if (*cursor == '\0') break;
        char *word = cursor;
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t') cursor++;
        if (command.name == NULL) command.name = word;
        else if (command.argc < STDIO_TEXT_PROTOCOL_MAX_ARGS) command.argv[command.argc++] = word;
        else command.truncated = true;
    }
    line_length = 0;
    line_truncated = false;
    return command.name == NULL ? NULL : &command;
}

const command_t *stdio_text_protocol_handle(void)
{
    int symbol = getchar_timeout_us(0);
    if (symbol == PICO_ERROR_TIMEOUT) return NULL;
    if (symbol == '\r' || symbol == '\n')
    {
        putchar('\n');
        return finish_line();
    }
    if (symbol == 0x08 || symbol == 0x7f)
    {
        if (line_length > 0)
        {
            line_length--;
            printf("\b \b");
        }
        return NULL;
    }
    if (isprint(symbol))
    {
        putchar(symbol);
        if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE) line[line_length++] = (char)symbol;
        else line_truncated = true;
    }
    return NULL;
}
