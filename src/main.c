#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "task.h"
#include "storage.h"
#include "utils.h"

#pragma comment(lib, "comctl32.lib")

// Window controls
#define ID_LISTVIEW 1001
#define ID_BUTTON_ADD 1002
#define ID_BUTTON_DEL 1003
#define ID_BUTTON_EDIT 1004
#define ID_BUTTON_TOGGLE 1005
#define ID_EDIT_TASK 1006

// Global variables
HWND g_hListView;
HWND g_hEditTask;
TaskList *g_taskList;
const char *SAVE_FILE = "tasks.txt";

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitListView(HWND hwnd);
void RefreshListView(void);
void AddTask(void);
void DeleteTask(void);
void EditTask(void);
void ToggleTask(void);
int GetSelectedTaskId(void);

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
        "Todo App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
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
                50,
                rect.right - 20,
                rect.bottom - 60,
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
        10, 50, rect.right - 20, rect.bottom - 60,
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

    if (task_list_add(g_taskList, text))
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