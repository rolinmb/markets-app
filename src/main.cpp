#include <windows.h>
#include <string>
#include <cstdlib>

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
            HWND hButton = GetDlgItem(hwnd, ID_BUTTON);  // get button handle
            EnableWindow(hButton, FALSE);                // disable button

            char buffer[5] = {0};
            GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, 5);

            // Check if all characters are alphabetical
            bool valid = true;
            for (int i = 0; buffer[i] != '\0'; i++) {
                if (!((buffer[i] >= 'A' && buffer[i] <= 'Z') || (buffer[i] >= 'a' && buffer[i] <= 'z'))) {
                    valid = false;
                    break;
                }
            }

            if (!valid) {
                MessageBoxA(hwnd, "Ticker must only contain letters A-Z", "Invalid Input", MB_OK | MB_ICONERROR);
                EnableWindow(hButton, TRUE); // re-enable button
                break;
            }

            MessageBoxA(hwnd, buffer, "Executing python script to fetch data for:", MB_OK | MB_ICONEXCLAMATION);

            // Run Python script (this is blocking because system() waits)
            std::string command = "python scripts/main.py ";
            command += buffer;
            system(command.c_str());

            MessageBoxA(hwnd, buffer, "Successfully executed python script and fetched data for:", MB_OK | MB_ICONINFORMATION);

            EnableWindow(hButton, TRUE); // re-enable button after script finishes
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
    wc.lpfnWndProc = WindowProc; // Register our window proceedure function
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
