#include "../include/task.h"
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

Task *task_create(const char *text, Priority priority, time_t due_date)
{
    if (!text || strlen(text) == 0 || strlen(text) >= MAX_TASK_TEXT)
    {
        return NULL;
    }

    Task *task = malloc(sizeof(Task));
    if (!task)
        return NULL;

    task->id = 0; // Will be set by list
    strncpy(task->text, text, MAX_TASK_TEXT - 1);
    task->text[MAX_TASK_TEXT - 1] = '\0';
    task->completed = false;
    task->created = time(NULL);
    task->priority = priority;
    task->due_date = due_date;
    task->next = NULL;

    return task;
}

bool task_list_add(TaskList *list, const char *text, Priority priority, time_t due_date)
{
    if (!list || !text)
        return false;
    if (list->count >= MAX_TASKS)
        return false;

    // Check for duplicates
    Task *current = list->head;
    while (current) {
        if (strcmp(current->text, text) == 0) {
            return false; // Duplicate found
        }
        current = current->next;
    }

    Task *new_task = task_create(text, priority, due_date);
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

// Filtering functions
TaskList *task_list_filter_pending(TaskList *list)
{
    if (!list)
        return NULL;

    TaskList *filtered = task_list_create();
    if (!filtered)
        return NULL;

    Task *current = list->head;
    while (current)
    {
        if (!current->completed)
        {
            Task *copy = task_create(current->text, current->priority, current->due_date);
            if (copy)
            {
                copy->id = current->id;
                copy->completed = current->completed;
                copy->created = current->created;
                copy->next = filtered->head;
                filtered->head = copy;
                filtered->count++;
            }
        }
        current = current->next;
    }

    return filtered;
}

TaskList *task_list_filter_completed(TaskList *list)
{
    if (!list)
        return NULL;

    TaskList *filtered = task_list_create();
    if (!filtered)
        return NULL;

    Task *current = list->head;
    while (current)
    {
        if (current->completed)
        {
            Task *copy = task_create(current->text, current->priority, current->due_date);
            if (copy)
            {
                copy->id = current->id;
                copy->completed = current->completed;
                copy->created = current->created;
                copy->next = filtered->head;
                filtered->head = copy;
                filtered->count++;
            }
        }
        current = current->next;
    }

    return filtered;
}

// Sorting functions (bubble sort on linked list)
void task_list_sort_by_priority(TaskList *list)
{
    if (!list || list->count < 2)
        return;

    bool swapped;
    do {
        swapped = false;
        Task *current = list->head;
        Task *prev = NULL;

        while (current && current->next)
        {
            if (current->priority < current->next->priority)
            {
                // Swap nodes
                Task *temp = current->next;
                current->next = temp->next;
                temp->next = current;
                if (prev)
                    prev->next = temp;
                else
                    list->head = temp;
                prev = temp;
                swapped = true;
            }
            else
            {
                prev = current;
                current = current->next;
            }
        }
    } while (swapped);
}

void task_list_sort_by_due_date(TaskList *list)
{
    if (!list || list->count < 2)
        return;

    bool swapped;
    do {
        swapped = false;
        Task *current = list->head;
        Task *prev = NULL;

        while (current && current->next)
        {
            if (current->due_date > current->next->due_date)
            {
                // Swap nodes
                Task *temp = current->next;
                current->next = temp->next;
                temp->next = current;
                if (prev)
                    prev->next = temp;
                else
                    list->head = temp;
                prev = temp;
                swapped = true;
            }
            else
            {
                prev = current;
                current = current->next;
            }
        }
    } while (swapped);
}

void task_list_sort_by_title(TaskList *list)
{
    if (!list || list->count < 2)
        return;

    bool swapped;
    do {
        swapped = false;
        Task *current = list->head;
        Task *prev = NULL;

        while (current && current->next)
        {
            if (strcmp(current->text, current->next->text) > 0)
            {
                // Swap nodes
                Task *temp = current->next;
                current->next = temp->next;
                temp->next = current;
                if (prev)
                    prev->next = temp;
                else
                    list->head = temp;
                prev = temp;
                swapped = true;
            }
            else
            {
                prev = current;
                current = current->next;
            }
        }
    } while (swapped);
}
