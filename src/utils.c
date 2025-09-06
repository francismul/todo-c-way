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

bool utils_strings_equal_case_insensitive(const char *str1, const char *str2)
{
    if (!str1 || !str2)
        return str1 == str2;

    while (*str1 && *str2) {
        if (tolower(*str1) != tolower(*str2))
            return false;
        str1++;
        str2++;
    }
    
    return *str1 == *str2;
}

bool utils_is_duplicate_task(TaskList *list, const char *text)
{
    if (!list || !text)
        return false;

    // Create a trimmed copy of the input text for comparison
    char trimmed_text[MAX_TASK_TEXT];
    strncpy(trimmed_text, text, MAX_TASK_TEXT - 1);
    trimmed_text[MAX_TASK_TEXT - 1] = '\0';
    utils_trim_string(trimmed_text);

    Task *current = list->head;
    while (current) {
        // Create a trimmed copy of existing task text
        char trimmed_existing[MAX_TASK_TEXT];
        strncpy(trimmed_existing, current->text, MAX_TASK_TEXT - 1);
        trimmed_existing[MAX_TASK_TEXT - 1] = '\0';
        utils_trim_string(trimmed_existing);

        if (utils_strings_equal_case_insensitive(trimmed_text, trimmed_existing)) {
            return true; // Duplicate found
        }
        current = current->next;
    }

    return false;
}