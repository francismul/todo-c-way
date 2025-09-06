#include "../include/utils.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

void utils_trim_string(char *str)
{
    if (!str)
        return;

    char *start = str;
    while (isspace(*start))
        start++;

    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }

    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        *end = '\0';
        end--;
    }
}

bool utils_is_valid_string(const char *str)
{
    if (!str)
        return false;

    while (*str)
    {
        if (!isspace(*str))
            return true;
        str++;
    }

    return false;
}