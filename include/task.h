#ifndef TASK_H
#define TASK_H

#include <stdbool.h>
#include <time.h>

#define MAX_TASK_TEXT 256
#define MAX_TASKS 1000

typedef enum {
    PRIORITY_LOW = 1,
    PRIORITY_MEDIUM = 2,
    PRIORITY_HIGH = 3
} Priority;

typedef struct Task
{
    int id;
    char text[MAX_TASK_TEXT];
    bool completed;
    time_t created;
    Priority priority;
    time_t due_date;
    struct Task *next;
} Task;

typedef struct TaskList
{
    Task *head;
    int count;
    int next_id;
} TaskList;

TaskList *task_list_create(void);
void task_list_destroy(TaskList *list);
Task *task_create(const char *text, Priority priority, time_t due_date);
bool task_list_add(TaskList *list, const char *text, Priority priority, time_t due_date);
bool task_list_remove(TaskList *list, int id);
Task *task_list_find(TaskList *list, int id);
bool task_list_toggle_complete(TaskList *list, int id);
bool task_list_edit(TaskList *list, int id, const char *new_text);
void task_list_clear(TaskList *list);

// Filtering functions
TaskList *task_list_filter_pending(TaskList *list);
TaskList *task_list_filter_completed(TaskList *list);

// Sorting functions
void task_list_sort_by_priority(TaskList *list);
void task_list_sort_by_due_date(TaskList *list);
void task_list_sort_by_title(TaskList *list);

#endif