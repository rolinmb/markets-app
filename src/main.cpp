#include <windows.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>

std::string LoadCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "Error: Could not open CSV file.";
    }
    std::ostringstream ss;
    std::string line;
    while (std::getline(file, line)) {
        ss << line << "\r\n";  // Windows-style newline
    }
    return ss.str();
}

// IDs for controls
#define ID_EDITBOX 1
#define ID_BUTTON  2

HWND hTextDisplay;

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

        hTextDisplay = CreateWindowExA(
            WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            200, 20, 250, 400, // x, y, width, height
            hwnd, NULL, GetModuleHandle(NULL), NULL
        );
        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON) {
            HWND hButton = GetDlgItem(hwnd, ID_BUTTON);
            EnableWindow(hButton, FALSE);

            char buffer[5] = {0};
            GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, 5);

            bool valid = true;
            for (int i = 0; buffer[i] != '\0'; i++) {
                if (!isalpha((unsigned char)buffer[i])) {
                    valid = false;
                    break;
                }
            }

            if (!valid) {
                MessageBoxA(hwnd, "Ticker must only contain letters A-Z", "Invalid Input", MB_OK | MB_ICONERROR);
                EnableWindow(hButton, TRUE);
                break;
            }

            // Run Python script
            std::string command = "python scripts/main.py ";
            command += buffer;
            system(command.c_str());

            // Load CSV content
            std::string csvPath = "data/" + std::string(buffer) + ".csv";
            std::string contents = LoadCSV(csvPath);

            // Show in the scrollable text box
            SetWindowTextA(hTextDisplay, contents.c_str());

            EnableWindow(hButton, TRUE);
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
