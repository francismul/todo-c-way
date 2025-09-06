#include "../include/storage.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

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
        // Try to recover by creating backup and clearing
        storage_create_backup(filename);
        return false;
    }

    // Validate count to prevent buffer overflow
    if (count < 0 || count > MAX_TASKS) {
        fclose(file);
        storage_create_backup(filename);
        return false;
    }

    int max_id = 0;
    int loaded_count = 0;
    
    for (int i = 0; i < count; i++) {
        TaskData data;
        if (fread(&data, sizeof(TaskData), 1, file) != 1) {
            // Partial read - try to recover what we can
            break;
        }

        // Validate data integrity
        if (data.id < 0 || data.id > INT_MAX / 2) {
            continue; // Skip invalid task
        }
        
        // Bounded validation to avoid reading past corrupt buffers
        if (!memchr(data.text, '\0', MAX_TASK_TEXT)) {
            continue; // No terminator within bounds -> invalid/corrupt
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
        loaded_count++;

        if (data.id > max_id)
            max_id = data.id;
    }

    list->next_id = max_id + 1;
    fclose(file);
    
    // If we loaded fewer tasks than expected, create backup
    if (loaded_count < count) {
        storage_create_backup(filename);
    }
    
    return true;
}

// Create backup of corrupted file
void storage_create_backup(const char *filename)
{
    char backup_filename[256];
    time_t now = time(NULL);
    struct tm tm_struct;
    struct tm *tm = NULL;
    
    // Use thread-safe localtime_s on Windows
    errno_t err = localtime_s(&tm_struct, &now);
    if (err == 0) {
        tm = &tm_struct;
    } else {
        // Fallback: use current time components if localtime fails
        tm = gmtime(&now);  // GMT fallback, not ideal but better than crash
        if (!tm) return;    // Complete failure
    }
    
    snprintf(backup_filename, sizeof(backup_filename), "%s.backup_%04d%02d%02d_%02d%02d%02d", 
             filename, tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
             tm->tm_hour, tm->tm_min, tm->tm_sec);
    
    // Try to copy the file
    FILE *src = fopen(filename, "rb");
    if (!src) return;
    
    FILE *dst = fopen(backup_filename, "wb");
    if (!dst) {
        fclose(src);
        return;
    }
    
    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes, dst) != bytes) {
            // Write failed, abort backup
            break;
        }
    }
    
    fclose(src);
    fclose(dst);
}
