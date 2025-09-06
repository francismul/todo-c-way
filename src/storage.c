#include "../include/storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool storage_save(TaskList *list, const char *filename)
{
    if (!list || !filename)
        return false;

    FILE *file = fopen(filename, "wb");
    if (!file)
        return false;

    // Write number of tasks
    int count = list->count;
    if (fwrite(&count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return false;
    }

    Task *current = list->head;
    while (current)
    {
        TaskData data;
        data.id = current->id;
        strncpy(data.text, current->text, MAX_TASK_TEXT - 1);
        data.text[MAX_TASK_TEXT - 1] = '\0';
        data.completed = current->completed;
        data.created = current->created;
        data.priority = current->priority;
        data.due_date = current->due_date;

        if (fwrite(&data, sizeof(TaskData), 1, file) != 1) {
            fclose(file);
            return false;
        }

        current = current->next;
    }

    fclose(file);
    return true;
}

bool storage_load(TaskList *list, const char *filename)
{
    if (!list || !filename)
        return false;

    FILE *file = fopen(filename, "rb");
    if (!file)
        return true;  // File doesn't exist, not an error

    task_list_clear(list);

    int count;
    if (fread(&count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return false;
    }

    int max_id = 0;
    for (int i = 0; i < count; i++) {
        TaskData data;
        if (fread(&data, sizeof(TaskData), 1, file) != 1) {
            fclose(file);
            return false;
        }

        Task *task = malloc(sizeof(Task));
        if (!task) {
            fclose(file);
            return false;
        }

        task->id = data.id;
        strncpy(task->text, data.text, MAX_TASK_TEXT - 1);
        task->text[MAX_TASK_TEXT - 1] = '\0';
        task->completed = data.completed;
        task->created = data.created;
        task->priority = data.priority;
        task->due_date = data.due_date;
        task->next = list->head;
        list->head = task;
        list->count++;

        if (data.id > max_id)
            max_id = data.id;
    }

    list->next_id = max_id + 1;
    fclose(file);
    return true;
}
