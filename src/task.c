#include "task.h"
#include <stdlib.h>
#include <string.h>

TaskList *task_list_create(void)
{
    TaskList *list = malloc(sizeof(TaskList));
    if (!list)
        return NULL;

    list->head = NULL;
    list->count = 0;
    list->next_id = 1;
    return list;
}

void task_list_destroy(TaskList *list)
{
    if (!list)
        return;

    task_list_clear(list);
    free(list);
}

Task *task_create(const char *text)
{
    if (!text || strlen(text) == 0 || strlen(text) >= MAX_TASK_TEXT)
    {
        return NULL;
    }

    Task *task = malloc(sizeof(Task));
    if (!task)
        return NULL;

    strncpy(task->text, text, MAX_TASK_TEXT - 1);
    task->text[MAX_TASK_TEXT - 1] = '\0';
    task->completed = false;
    task->created = time(NULL);
    task->next = NULL;

    return task;
}

bool task_list_add(TaskList *list, const char *text)
{
    if (!list || !text)
        return false;
    if (list->count >= MAX_TASKS)
        return false;

    Task *new_task = task_create(text);
    if (!new_task)
        return false;

    new_task->id = list->next_id++;

    new_task->next = list->head;
    list->head = new_task;
    list->count++;

    return true;
}

bool task_list_remove(TaskList *list, int id)
{
    if (!list || !list->head)
        return false;

    Task *current = list->head;
    Task *prev = NULL;

    while (current)
    {
        if (current->id == id)
        {
            if (prev)
            {
                prev->next = current->next;
            }
            else
            {
                list->head = current->next;
            }
            free(current);
            list->count--;
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

Task *task_list_find(TaskList *list, int id)
{
    if (!list)
        return NULL;

    Task *current = list->head;
    while (current)
    {
        if (current->id == id)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

bool task_list_toggle_complete(TaskList *list, int id)
{
    Task *task = task_list_find(list, id);
    if (!task)
        return false;

    task->completed = !task->completed;
    return true;
}

bool task_list_edit(TaskList *list, int id, const char *new_text)
{
    if (!new_text || strlen(new_text) == 0 || strlen(new_text) >= MAX_TASK_TEXT)
    {
        return false;
    }

    Task *task = task_list_find(list, id);
    if (!task)
        return false;

    strncpy(task->text, new_text, MAX_TASK_TEXT - 1);
    task->text[MAX_TASK_TEXT - 1] = '\0';
    return true;
}

void task_list_clear(TaskList *list)
{
    if (!list)
        return;

    Task *current = list->head;
    while (current)
    {
        Task *next = current->next;
        free(current);
        current = next;
    }

    list->head = NULL;
    list->count = 0;
}
