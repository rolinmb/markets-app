#include <windows.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

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

HWND hTextDisplay;
HWND hPriceLabel;
HWND hChangeLabel;
HWND hDollarChangeLabel;

std::string g_currentTicker;

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

        // Data Panel
        hTextDisplay = CreateWindowExA(
            WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            200, 20, 250, 400, // x, y, width, height
            hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Price panel
        hPriceLabel = CreateWindowExA(
            0, "STATIC", "Price ($): --", WS_CHILD | WS_VISIBLE,
            50, 170, 130, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Change % panel
        hChangeLabel = CreateWindowExA(
            0, "STATIC", "% Change: --", WS_CHILD | WS_VISIBLE,
            50, 195, 130, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
        );

        // Dollar Change Panel
        hDollarChangeLabel = CreateWindowExA(
            0, "STATIC", "$ Change: --", WS_CHILD | WS_VISIBLE,
            50, 220, 130, 20, hwnd, NULL, GetModuleHandle(NULL), NULL
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

            std::string csvPath = "data/" + std::string(buffer) + ".csv";
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
                    if (row[0] == "Price") {
                        price = row[1];
                    } else if (row[0] == "Change") {
                        change = row[1];
                    } else if (row[0] == "Dollar Change") {
                        dollarChange = row[1];
                    }
                }
            }

            SetWindowTextA(hPriceLabel, ("Price ($): " + price).c_str());
            SetWindowTextA(hChangeLabel, ("% Change: " + change).c_str());
            SetWindowTextA(hDollarChangeLabel, ("$ Change: " + dollarChange).c_str());

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
        }
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, ID_TIMER);
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
