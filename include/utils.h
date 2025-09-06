#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include "../include/task.h"

void utils_trim_string(char *str);
bool utils_is_valid_string(const char *str);
bool utils_strings_equal_case_insensitive(const char *str1, const char *str2);
bool utils_is_duplicate_task(TaskList *list, const char *text);

#endif
