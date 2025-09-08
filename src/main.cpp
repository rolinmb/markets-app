#include <windows.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

std::vector<std::string> SplitCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string cell;

    while (std::getline(ss, cell, ',')) {
        result.push_back(cell);
    }
    return result;
}

std::vector<std::vector<std::string>> LoadCSV(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::vector<std::string>> rows;

    if (!file.is_open()) {
        return rows; // empty
    }

    std::string line;

    // Skip header line
    if (std::getline(file, line)) {
        // ignore
    }

    while (std::getline(file, line)) {
        rows.push_back(SplitCSVLine(line));
    }

    return rows;
}

// IDs for controls
#define ID_EDITBOX 1
#define ID_BUTTON 2
#define ID_TIMER 100
#define ID_DATETIME_TIMER 101

HWND hTextDisplay;
HWND hPriceLabel;
HWND hChangeLabel;
HWND hDollarChangeLabel;
HWND hDateTimeLabel;

std::string g_currentTicker;

HBRUSH hBrushBlack = CreateSolidBrush(RGB(0, 0, 0)); // Black
COLORREF textColor = RGB(255, 255, 255); // White
COLORREF changeColor = RGB(255, 255, 255); // default white
COLORREF dollarChangeColor = RGB(255, 255, 255); // default white

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CTLCOLORSTATIC: { // For static text labels
        HDC hdc = (HDC)wParam;

        if ((HWND)lParam == hChangeLabel) {
            SetTextColor(hdc, changeColor);
        } else if ((HWND)lParam == hDollarChangeLabel) {
            SetTextColor(hdc, dollarChangeColor);
        } else {
            SetTextColor(hdc, textColor); // other static text
        }

        SetBkColor(hdc, RGB(0, 0, 0)); // black background
        return (INT_PTR)hBrushBlack;
    }
    case WM_CTLCOLOREDIT: {  // For edit controls
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, textColor);     // White text
        SetBkColor(hdc, RGB(0,0,0)); // black background
        return (INT_PTR)hBrushBlack;
    }
    
    case WM_ERASEBKGND: { // For the main window background
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBrushBlack);
        return 1; // We've erased background
    }
    case WM_CREATE: {
        // Create a static text label
        CreateWindowExA(
            0, "STATIC", "markets-app", WS_CHILD | WS_VISIBLE,
            50, 20, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        CreateWindowExA(
            0, "STATIC", "Enter a ticker (no more than 4 characters).", WS_CHILD | WS_VISIBLE,
            50, 45, 135, 50, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Create an Edit control (text box)
        HWND hEdit = CreateWindowExA(
            0, "EDIT", "SPY", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            50, 100, 135, 25, hwnd, (HMENU)ID_EDITBOX, GetModuleHandle(NULL), NULL
        );

        // Limit to 4 characters
        SendMessageA(hEdit, EM_SETLIMITTEXT, 4, 0);

        // Create a Button
        CreateWindowExA(
            0, "BUTTON", "Fetch Data", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            50, 130, 100, 30, hwnd, (HMENU)ID_BUTTON, GetModuleHandle(NULL), NULL
        );

        // Data Panel
        hTextDisplay = CreateWindowExA(
            WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            225, 20, 250, 400,
            hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Price panel
        hPriceLabel = CreateWindowExA(
            0, "STATIC", "Price ($): --", WS_CHILD | WS_VISIBLE,
            50, 170, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Change % panel
        hChangeLabel = CreateWindowExA(
            0, "STATIC", "% Change: --", WS_CHILD | WS_VISIBLE,
            50, 195, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Dollar Change Panel
        hDollarChangeLabel = CreateWindowExA(
            0, "STATIC", "$ Change: --", WS_CHILD | WS_VISIBLE,
            50, 220, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        hDateTimeLabel = CreateWindowExA(
            0, "STATIC", "", WS_CHILD | WS_VISIBLE,
            50, 250, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        SetTimer(hwnd, ID_DATETIME_TIMER, 1000, NULL);

        return 0;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON) {
            // Check if Python script exists
            std::ifstream scriptFile("scripts/main.py");
            if (!scriptFile.is_open()) {
                MessageBoxA(hwnd, "Python script not found. Exiting.", "Error", MB_OK | MB_ICONERROR);
                PostQuitMessage(0);
                return 0;
            }

            HWND hButton = GetDlgItem(hwnd, ID_BUTTON);
            EnableWindow(hButton, FALSE);

            char buffer[5] = {0};
            GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, 5);

            std::string ticker(buffer);

            // Trim leading whitespace
            ticker.erase(ticker.begin(), std::find_if(ticker.begin(), ticker.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

            // Trim trailing whitespace
            ticker.erase(std::find_if(ticker.rbegin(), ticker.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(), ticker.end());

            // Check for empty ticker
            if (ticker.empty()) {
                MessageBoxA(hwnd, "You must enter at least one character", "Invalid Input", MB_OK | MB_ICONERROR);
                EnableWindow(hButton, TRUE);
                break;
            }

            // Check that all characters are letters
            bool valid = std::all_of(ticker.begin(), ticker.end(), [](unsigned char ch) {
                return std::isalpha(ch);
            });

            if (!valid) {
                MessageBoxA(hwnd, "Ticker must only contain letters A-Z", "Invalid Input", MB_OK | MB_ICONERROR);
                EnableWindow(hButton, TRUE);
                break;
            }

            // Run Python script
            std::string command = "python scripts/main.py ";
            command += ticker;
            system(command.c_str());

            std::string csvPath = "data/" + std::string(ticker) + ".csv";
            auto rows = LoadCSV(csvPath);

            std::ostringstream out;
            for (const auto& row : rows) {
                for (size_t i = 0; i < row.size(); i++) {
                    out << row[i];
                    if (i < row.size() - 1) {
                        out << " | "; // separator
                    }
                }
                out << "\r\n"; // newline for EDIT control
            }

            SetWindowTextA(hTextDisplay, out.str().c_str());

            std::string price = "--", change = "--", dollarChange = "--";
            for (const auto& row : rows) {
                if (row.size() >= 2) {
                    if (row[0] == "Price") price = row[1];
                    else if (row[0] == "Change") change = row[1];
                    else if (row[0] == "Dollar Change") dollarChange = row[1];
                }
            }

            changeColor = (change[0] == '-') ? RGB(255, 0, 0) : RGB(0, 255, 0); // red if negative, green if positive
            dollarChangeColor = (dollarChange[0] == '-') ? RGB(255, 0, 0) : RGB(0, 255, 0);

            SetWindowTextA(hPriceLabel, ("Price ($): " + price).c_str());
            SetWindowTextA(hChangeLabel, ("% Change: " + change).c_str());
            SetWindowTextA(hDollarChangeLabel, ("$ Change: " + dollarChange).c_str());

            InvalidateRect(hChangeLabel, NULL, TRUE);
            InvalidateRect(hDollarChangeLabel, NULL, TRUE);

            EnableWindow(hButton, TRUE);

            g_currentTicker = buffer;
            SetTimer(hwnd, ID_TIMER, 15000, NULL);
        }
        return 0;
    }

    case WM_TIMER: {
        if (wParam == ID_TIMER && !g_currentTicker.empty()) {
            // Re-run Python script for g_currentTicker
            std::string command = "python scripts/main.py " + g_currentTicker;
            system(command.c_str());

            std::string csvPath = "data/" + g_currentTicker + ".csv";
            auto rows = LoadCSV(csvPath);

            std::ostringstream out;
            for (const auto& row : rows) {
                for (size_t i = 0; i < row.size(); i++) {
                    out << row[i];
                    if (i < row.size() - 1) out << " | ";
                }
                out << "\r\n";
            }
            SetWindowTextA(hTextDisplay, out.str().c_str());

            // Update Price/Change labels
            std::string price = "--", change = "--", dollarChange = "--";
            for (const auto& row : rows) {
                if (row.size() >= 2) {
                    if (row[0] == "Price") price = row[1];
                    else if (row[0] == "Change") change = row[1];
                    else if (row[0] == "Dollar Change") dollarChange = row[1];
                }
            }
            
            SetWindowTextA(hPriceLabel, ("Price ($): " + price).c_str());
            SetWindowTextA(hChangeLabel, ("% Change: " + change).c_str());
            SetWindowTextA(hDollarChangeLabel, ("$ Change: " + dollarChange).c_str());
        } else if (wParam == ID_DATETIME_TIMER) {
            // Update date and time
            SYSTEMTIME st;
            GetLocalTime(&st);

            char datetimeStr[100];
            snprintf(datetimeStr, sizeof(datetimeStr), "%04d-%02d-%02d %02d:%02d:%02d",
                st.wYear, st.wMonth, st.wDay,
                st.wHour, st.wMinute, st.wSecond);

            SetWindowTextA(hDateTimeLabel, datetimeStr);
        }
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, ID_TIMER);
        KillTimer(hwnd, ID_DATETIME_TIMER);
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
        CW_USEDEFAULT, CW_USEDEFAULT, 777, 777,
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
