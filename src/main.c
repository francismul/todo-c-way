#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "../include/task.h"
#include "../include/storage.h"
#include "../include/utils.h"

#pragma comment(lib, "comctl32.lib")

// Window controls
#define ID_LISTVIEW 1001
#define ID_BUTTON_ADD 1002
#define ID_BUTTON_DEL 1003
#define ID_BUTTON_EDIT 1004
#define ID_BUTTON_TOGGLE 1005
#define ID_EDIT_TASK 1006
#define ID_COMBO_PRIORITY 1007
#define ID_EDIT_DUE_DATE 1008
#define ID_BUTTON_SHOW_ALL 1009
#define ID_BUTTON_SHOW_PENDING 1010
#define ID_BUTTON_SHOW_COMPLETED 1011
#define ID_BUTTON_SORT_PRIORITY 1012
#define ID_BUTTON_SORT_DUE_DATE 1013
#define ID_BUTTON_SORT_TITLE 1014

// Global variables
HWND g_hListView;
HWND g_hEditTask;
HWND g_hComboPriority;
HWND g_hEditDueDate;
TaskList *g_taskList;
TaskList *g_filteredList; // For filtered views
const char *SAVE_FILE = "tasks.dat";

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitListView(HWND hwnd);
void RefreshListView(void);
void AddTask(void);
void DeleteTask(void);
void EditTask(void);
void ToggleTask(void);
int GetSelectedTaskId(void);
void ShowAllTasks(void);
void ShowPendingTasks(void);
void ShowCompletedTasks(void);
void SortByPriority(void);
void SortByDueDate(void);
void SortByTitle(void);
void RefreshFilteredListView(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    g_taskList = task_list_create();
    if (!g_taskList)
    {
        MessageBox(NULL, "Failed to initialize task list", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    g_filteredList = NULL;
    storage_load(g_taskList, SAVE_FILE);

    const char *CLASS_NAME = "TodoAppWindow";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Todo App - Enhanced Task Management",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 600,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
    {
        MessageBox(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    storage_save(g_taskList, SAVE_FILE);
    task_list_destroy(g_taskList);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        g_hEditTask = CreateWindow(
            "EDIT",
            "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            10, 10, 400, 25,
            hwnd,
            (HMENU)ID_EDIT_TASK,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Add Task",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            420, 10, 80, 25,
            hwnd,
            (HMENU)ID_BUTTON_ADD,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Delete",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            510, 10, 70, 25,
            hwnd,
            (HMENU)ID_BUTTON_DEL,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Edit",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            590, 10, 60, 25,
            hwnd,
            (HMENU)ID_BUTTON_EDIT,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Toggle",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            660, 10, 70, 25,
            hwnd,
            (HMENU)ID_BUTTON_TOGGLE,
            GetModuleHandle(NULL),
            NULL);

        // Priority combo box
        CreateWindow(
            "STATIC",
            "Priority:",
            WS_CHILD | WS_VISIBLE,
            10, 75, 60, 20,
            hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL);

        g_hComboPriority = CreateWindow(
            "COMBOBOX",
            "",
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            70, 70, 100, 200,
            hwnd,
            (HMENU)ID_COMBO_PRIORITY,
            GetModuleHandle(NULL),
            NULL);

        SendMessage(g_hComboPriority, CB_ADDSTRING, 0, (LPARAM)"Low");
        SendMessage(g_hComboPriority, CB_ADDSTRING, 0, (LPARAM)"Medium");
        SendMessage(g_hComboPriority, CB_ADDSTRING, 0, (LPARAM)"High");
        SendMessage(g_hComboPriority, CB_SETCURSEL, 1, 0); // Default to Medium

        // Due date input
        CreateWindow(
            "STATIC",
            "Due Date (YYYY-MM-DD HH:MM):",
            WS_CHILD | WS_VISIBLE,
            180, 75, 180, 20,
            hwnd,
            NULL,
            GetModuleHandle(NULL),
            NULL);

        g_hEditDueDate = CreateWindow(
            "EDIT",
            "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            360, 70, 150, 25,
            hwnd,
            (HMENU)ID_EDIT_DUE_DATE,
            GetModuleHandle(NULL),
            NULL);

        // Set default due date (1 day from now)
        time_t default_due = time(NULL) + 86400;
        struct tm *tm = localtime(&default_due);
        char default_due_str[20];
        strftime(default_due_str, sizeof(default_due_str), "%Y-%m-%d %H:%M", tm);
        SetWindowText(g_hEditDueDate, default_due_str);

        // Filter buttons
        CreateWindow(
            "BUTTON",
            "Show All",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            520, 70, 80, 25,
            hwnd,
            (HMENU)ID_BUTTON_SHOW_ALL,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Show Pending",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            610, 70, 100, 25,
            hwnd,
            (HMENU)ID_BUTTON_SHOW_PENDING,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Show Completed",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            720, 70, 120, 25,
            hwnd,
            (HMENU)ID_BUTTON_SHOW_COMPLETED,
            GetModuleHandle(NULL),
            NULL);

        // Sort buttons
        CreateWindow(
            "BUTTON",
            "Sort Priority",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            850, 70, 100, 25,
            hwnd,
            (HMENU)ID_BUTTON_SORT_PRIORITY,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Sort Due Date",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            960, 70, 110, 25,
            hwnd,
            (HMENU)ID_BUTTON_SORT_DUE_DATE,
            GetModuleHandle(NULL),
            NULL);

        CreateWindow(
            "BUTTON",
            "Sort Title",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            1080, 70, 80, 25,
            hwnd,
            (HMENU)ID_BUTTON_SORT_TITLE,
            GetModuleHandle(NULL),
            NULL);

        InitListView(hwnd);
        RefreshListView();
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_BUTTON_ADD:
            AddTask();
            break;
        case ID_BUTTON_DEL:
            DeleteTask();
            break;
        case ID_BUTTON_EDIT:
            EditTask();
            break;
        case ID_BUTTON_TOGGLE:
            ToggleTask();
            break;
        case ID_BUTTON_SHOW_ALL:
            ShowAllTasks();
            break;
        case ID_BUTTON_SHOW_PENDING:
            ShowPendingTasks();
            break;
        case ID_BUTTON_SHOW_COMPLETED:
            ShowCompletedTasks();
            break;
        case ID_BUTTON_SORT_PRIORITY:
            SortByPriority();
            break;
        case ID_BUTTON_SORT_DUE_DATE:
            SortByDueDate();
            break;
        case ID_BUTTON_SORT_TITLE:
            SortByTitle();
            break;
        }
        break;

    case WM_SIZE:
        if (g_hListView)
        {
            RECT rect;
            GetClientRect(hwnd, &rect);
            SetWindowPos(
                g_hListView,
                NULL,
                10,
                100,
                rect.right - 20,
                rect.bottom - 110,
                SWP_NOZORDER);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void InitListView(HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);

    g_hListView = CreateWindow(
        WC_LISTVIEW, "",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 100, rect.right - 20, rect.bottom - 110,
        hwnd,
        (HMENU)ID_LISTVIEW,
        GetModuleHandle(NULL),
        NULL);

    // Set up columns
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    lvc.iSubItem = 0;
    lvc.pszText = "ID";
    lvc.cx = 50;
    ListView_InsertColumn(g_hListView, 0, &lvc);

    lvc.iSubItem = 1;
    lvc.pszText = "Task";
    lvc.cx = 400;
    ListView_InsertColumn(g_hListView, 1, &lvc);

    lvc.iSubItem = 2;
    lvc.pszText = "Status";
    lvc.cx = 100;
    ListView_InsertColumn(g_hListView, 2, &lvc);

    lvc.iSubItem = 3;
    lvc.pszText = "Priority";
    lvc.cx = 80;
    ListView_InsertColumn(g_hListView, 3, &lvc);

    lvc.iSubItem = 4;
    lvc.pszText = "Due Date";
    lvc.cx = 120;
    ListView_InsertColumn(g_hListView, 4, &lvc);
}

void RefreshListView(void)
{
    if (!g_hListView || !g_taskList)
        return;

    ListView_DeleteAllItems(g_hListView);

    Task *current = g_taskList->head;
    int index = 0;

    while (current)
    {
        LVITEM lvi = {0};
        char id_str[16];
        sprintf(id_str, "%d", current->id);

        lvi.mask = LVIF_TEXT;
        lvi.iItem = index;
        lvi.iSubItem = 0;
        lvi.pszText = id_str;
        ListView_InsertItem(g_hListView, &lvi);

        ListView_SetItemText(g_hListView, index, 1, current->text);
        ListView_SetItemText(g_hListView, index, 2, current->completed ? "Done" : "Pending");

        // Priority
        const char *priority_str;
        switch (current->priority) {
            case PRIORITY_LOW: priority_str = "Low"; break;
            case PRIORITY_MEDIUM: priority_str = "Medium"; break;
            case PRIORITY_HIGH: priority_str = "High"; break;
            default: priority_str = "Unknown"; break;
        }
        ListView_SetItemText(g_hListView, index, 3, (char*)priority_str);

        // Due date
        char due_str[32];
        if (current->due_date > 0) {
            struct tm *due_tm = localtime(&current->due_date);
            strftime(due_str, sizeof(due_str), "%Y-%m-%d %H:%M", due_tm);
        } else {
            strcpy(due_str, "No due date");
        }
        ListView_SetItemText(g_hListView, index, 4, due_str);

        current = current->next;
        index++;
    }
}

void AddTask(void)
{
    char text[MAX_TASK_TEXT];
    GetWindowText(g_hEditTask, text, sizeof(text));

    utils_trim_string(text);

    if (!utils_is_valid_string(text))
    {
        MessageBox(NULL, "Please enter a valid task", "Error", MB_OK | MB_ICONWARNING);
        return;
    }

    // Get priority from combo box
    int priority_index = SendMessage(g_hComboPriority, CB_GETCURSEL, 0, 0);
    Priority priority = PRIORITY_MEDIUM; // default
    switch (priority_index) {
        case 0: priority = PRIORITY_LOW; break;
        case 1: priority = PRIORITY_MEDIUM; break;
        case 2: priority = PRIORITY_HIGH; break;
    }

    // Get due date from edit control
    char due_date_str[32];
    GetWindowText(g_hEditDueDate, due_date_str, sizeof(due_date_str));
    
    time_t due_date = time(NULL) + 86400; // default: 1 day from now
    if (strlen(due_date_str) > 0) {
        struct tm tm = {0};
        if (sscanf(due_date_str, "%d-%d-%d %d:%d", 
                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday, 
                   &tm.tm_hour, &tm.tm_min) == 5) {
            tm.tm_year -= 1900; // tm_year is years since 1900
            tm.tm_mon -= 1;     // tm_mon is 0-based
            due_date = mktime(&tm);
        }
    }

    if (task_list_add(g_taskList, text, priority, due_date))
    {
        SetWindowText(g_hEditTask, "");
        RefreshListView();
        storage_save(g_taskList, SAVE_FILE);
    }
    else
    {
        MessageBox(NULL, "Failed to add task", "Error", MB_OK | MB_ICONERROR);
    }
}

void DeleteTask(void)
{
    int task_id = GetSelectedTaskId();
    if (task_id == -1)
    {
        MessageBox(NULL, "Please select a task to delete", "Error", MB_OK | MB_ICONWARNING);
        return;
    }

    if (task_list_remove(g_taskList, task_id))
    {
        RefreshListView();
        storage_save(g_taskList, SAVE_FILE);
    }
    else
    {
        MessageBox(NULL, "Failed to delete task", "Error", MB_OK | MB_ICONERROR);
    }
}

void EditTask(void)
{
    int task_id = GetSelectedTaskId();
    if (task_id == -1)
    {
        MessageBox(NULL, "Please select a task to edit", "Error", MB_OK | MB_ICONWARNING);
        return;
    }

    Task *task = task_list_find(g_taskList, task_id);
    if (!task)
        return;

    SetWindowText(g_hEditTask, task->text);
    MessageBox(NULL, "Task loaded into text field. Modify and click 'Add Task' to update.",
               "Edit Mode", MB_OK | MB_ICONINFORMATION);

    task_list_remove(g_taskList, task_id);
    RefreshListView();
}

void ToggleTask(void)
{
    int task_id = GetSelectedTaskId();
    if (task_id == -1)
    {
        MessageBox(NULL, "Please select a task to toggle", "Error", MB_OK | MB_ICONWARNING);
        return;
    }

    if (task_list_toggle_complete(g_taskList, task_id))
    {
        RefreshListView();
        storage_save(g_taskList, SAVE_FILE);
    }
    else
    {
        MessageBox(NULL, "Failed to toggle task", "Error", MB_OK | MB_ICONERROR);
    }
}

int GetSelectedTaskId(void)
{
    int selected = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
    if (selected == -1)
        return -1;

    char id_str[16];
    ListView_GetItemText(g_hListView, selected, 0, id_str, sizeof(id_str));
    return atoi(id_str);
}

void ShowAllTasks(void)
{
    if (g_filteredList) {
        task_list_destroy(g_filteredList);
        g_filteredList = NULL;
    }
    RefreshListView();
}

void ShowPendingTasks(void)
{
    if (g_filteredList) {
        task_list_destroy(g_filteredList);
    }
    g_filteredList = task_list_filter_pending(g_taskList);
    RefreshFilteredListView();
}

void ShowCompletedTasks(void)
{
    if (g_filteredList) {
        task_list_destroy(g_filteredList);
    }
    g_filteredList = task_list_filter_completed(g_taskList);
    RefreshFilteredListView();
}

void SortByPriority(void)
{
    task_list_sort_by_priority(g_taskList);
    RefreshListView();
    storage_save(g_taskList, SAVE_FILE);
}

void SortByDueDate(void)
{
    task_list_sort_by_due_date(g_taskList);
    RefreshListView();
    storage_save(g_taskList, SAVE_FILE);
}

void SortByTitle(void)
{
    task_list_sort_by_title(g_taskList);
    RefreshListView();
    storage_save(g_taskList, SAVE_FILE);
}

void RefreshFilteredListView(void)
{
    if (!g_hListView || !g_filteredList)
        return;

    ListView_DeleteAllItems(g_hListView);

    Task *current = g_filteredList->head;
    int index = 0;

    while (current)
    {
        LVITEM lvi = {0};
        char id_str[16];
        sprintf(id_str, "%d", current->id);

        lvi.mask = LVIF_TEXT;
        lvi.iItem = index;
        lvi.iSubItem = 0;
        lvi.pszText = id_str;
        ListView_InsertItem(g_hListView, &lvi);

        ListView_SetItemText(g_hListView, index, 1, current->text);
        ListView_SetItemText(g_hListView, index, 2, current->completed ? "Done" : "Pending");

        // Priority
        const char *priority_str;
        switch (current->priority) {
            case PRIORITY_LOW: priority_str = "Low"; break;
            case PRIORITY_MEDIUM: priority_str = "Medium"; break;
            case PRIORITY_HIGH: priority_str = "High"; break;
            default: priority_str = "Unknown"; break;
        }
        ListView_SetItemText(g_hListView, index, 3, (char*)priority_str);

        // Due date
        char due_str[32];
        if (current->due_date > 0) {
            struct tm *due_tm = localtime(&current->due_date);
            strftime(due_str, sizeof(due_str), "%Y-%m-%d %H:%M", due_tm);
        } else {
            strcpy(due_str, "No due date");
        }
        ListView_SetItemText(g_hListView, index, 4, due_str);

        current = current->next;
        index++;
    }
}