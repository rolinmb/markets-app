#include <windows.h>

// IDs for controls
#define ID_EDITBOX 1
#define ID_BUTTON  2

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Create a static text label
        CreateWindowExA(
            0, "STATIC", "markets-app", WS_CHILD | WS_VISIBLE,
            50, 20, 100, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        CreateWindowExA(
            0, "STATIC", "Enter a ticker (no more than 4 characters).", WS_CHILD | WS_VISIBLE,
            50, 45, 100, 50, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Create an Edit control (text box)
        HWND hEdit = CreateWindowExA(
            0, "EDIT", "SPY", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            50, 100, 100, 25, hwnd, (HMENU)ID_EDITBOX, GetModuleHandle(NULL), NULL
        );

        // Limit to 4 characters
        SendMessageA(hEdit, EM_SETLIMITTEXT, 4, 0);

        // Create a Button
        CreateWindowExA(
            0, "BUTTON", "Fetch Data", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            50, 130, 100, 30, hwnd, (HMENU)ID_BUTTON, GetModuleHandle(NULL), NULL
        );
        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON) {
            char buffer[5] = {0};
            GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, 5);
            MessageBoxA(hwnd, buffer, "You typed:", MB_OK);
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "markets-app";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "markets-app",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}
