#ifndef STORAGE_H
#define STORAGE_H

#include "task.h"

bool storage_save(TaskList *list, const char *filename);
bool storage_load(TaskList *list, const char *filename);

#endif