#ifndef STORAGE_H
#define STORAGE_H

#include "task.h"
#include <stdbool.h>
#include <time.h>

typedef struct {
    int id;
    char text[MAX_TASK_TEXT];
    bool completed;
    time_t created;
    Priority priority;
    time_t due_date;
} TaskData;

bool storage_save(TaskList *list, const char *filename);
bool storage_load(TaskList *list, const char *filename);
void storage_create_backup(const char *filename);

#endif