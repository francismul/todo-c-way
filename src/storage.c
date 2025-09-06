#include "storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

bool storage_save(TaskList *list, const char *filename)
{
    if (!list || !filename)
        return false;

    FILE *file = fopen(filename, "w");
    if (!file)
        return false;

    Task *current = list->head;
    while (current)
    {
        fprintf(file, "%d|%s|%d|%ld\n",
                current->id,
                current->text,
                current->completed ? 1 : 0,
                (long)current->created);
        current = current->next;
    }

    fclose(file);
    return true;
}

bool storage_load(TaskList *list, const char *filename)
{
    if (!list || !filename)
        return false;

    FILE *file = fopen(filename, "r");
    if (!file)
        return true;

    task_list_clear(list);

    char line[512];
    int max_id = 0;

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;

        char *token = strtok(line, "|");
        if (!token)
            continue;
        int id = atoi(token);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        char *text = token;

        token = strtok(NULL, "|");
        if (!token)
            continue;
        bool completed = (atoi(token) == 1);

        token = strtok(NULL, "|");
        time_t created = token ? atol(token) : time(NULL);

        Task *task = malloc(sizeof(Task));
        if (task)
        {
            task->id = id;
            strncpy(task->text, text, MAX_TASK_TEXT - 1);
            task->text[MAX_TASK_TEXT - 1] = '\0';
            task->completed = completed;
            task->created = created;
            task->next = list->head;
            list->head = task;
            list->count++;

            if (id > max_id)
                max_id = id;
        }
    }

    list->next_id = max_id + 1;
    fclose(file);
    return true;
}
