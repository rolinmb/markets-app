#include <windows.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// ---------------- CSV Handling ----------------
std::vector<std::string> SplitCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::string cell;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        }
        else if (c == ',' && !inQuotes) {
            result.push_back(cell);
            cell.clear();
        }
        else {
            cell.push_back(c);
        }
    }
    result.push_back(cell);
    return result;
}


std::vector<std::vector<std::string>> LoadCSV(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::vector<std::string>> rows;
    if (!file.is_open()) return rows;
    std::string line;
    std::getline(file, line); // Skip header
    while (std::getline(file, line)) {
        rows.push_back(SplitCSVLine(line));
    }
    return rows;
}

// ---------------- IDs for controls ----------------
#define ID_EDITBOX 1
#define ID_BUTTON 2
#define ID_MODE_BUTTON 3
#define ID_TIMER 100
#define ID_DATETIME_TIMER 101

// ---------------- Globals ----------------
HWND hTextDisplay, hPriceLabel, hChangeLabel, hDollarChangeLabel, hDateTimeLabel, hModeLabel, hInfoLabel, hAppLabel;
std::string g_currentAsset;

HBRUSH hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
COLORREF textColor = RGB(255, 255, 255);
COLORREF changeColor = RGB(255, 255, 255);
COLORREF dollarChangeColor = RGB(255, 255, 255);

bool g_isEquitiesMode = true;
HWND hModeButton;

// ---------------- Window Procedure ----------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        if ((HWND)lParam == hChangeLabel) SetTextColor(hdc, changeColor);
        else if ((HWND)lParam == hDollarChangeLabel) SetTextColor(hdc, dollarChangeColor);
        else SetTextColor(hdc, textColor);
        SetBkColor(hdc, RGB(0, 0, 0));
        return (INT_PTR)hBrushBlack;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, textColor);
        SetBkColor(hdc, RGB(0, 0, 0));
        return (INT_PTR)hBrushBlack;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBrushBlack);
        return 1;
    }
    case WM_CREATE: {
        hAppLabel = CreateWindowExA(0, "STATIC", "equities", WS_CHILD | WS_VISIBLE,
            50, 20, 200, 40, hwnd, NULL, GetModuleHandle(NULL), NULL);

        hInfoLabel = CreateWindowExA(0, "STATIC", "Enter an equity ticker symbol. (1-4 characters)", WS_CHILD | WS_VISIBLE,
            50, 65, 200, 40, hwnd, NULL, GetModuleHandle(NULL), NULL);

        HWND hEdit = CreateWindowExA(0, "EDIT", "SPY",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
            50, 110, 135, 25, hwnd, (HMENU)ID_EDITBOX, GetModuleHandle(NULL), NULL);
        SendMessageA(hEdit, EM_SETLIMITTEXT, 15, 0);

        CreateWindowExA(0, "BUTTON", "Fetch Data",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            50, 140, 100, 30, hwnd, (HMENU)ID_BUTTON, GetModuleHandle(NULL), NULL);

        hTextDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            300, 20, 250, 200, hwnd, NULL, GetModuleHandle(NULL), NULL);

        hPriceLabel = CreateWindowExA(0, "STATIC", "Price: --",
            WS_CHILD | WS_VISIBLE, 50, 170, 200, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hChangeLabel = CreateWindowExA(0, "STATIC", "% Change: --",
            WS_CHILD | WS_VISIBLE, 50, 205, 200, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hDollarChangeLabel = CreateWindowExA(0, "STATIC", "$ Change: --",
            WS_CHILD | WS_VISIBLE, 50, 240, 200, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hDateTimeLabel = CreateWindowExA(0, "STATIC", "",
            WS_CHILD | WS_VISIBLE, 50, 300, 200, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hModeButton = CreateWindowExA(0, "BUTTON", "switch to crypto mode",
            WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            500, 300, 150, 30, hwnd, (HMENU)ID_MODE_BUTTON, GetModuleHandle(NULL), NULL);
        hModeLabel = CreateWindowExA(0, "STATIC", "mode: equities",
            WS_CHILD | WS_VISIBLE,
            500, 260, 200, 30, hwnd, NULL, GetModuleHandle(NULL), NULL);

        SetTimer(hwnd, ID_DATETIME_TIMER, 1000, NULL);
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_MODE_BUTTON) {
            g_isEquitiesMode = !g_isEquitiesMode;
            if (g_isEquitiesMode) {
                SetWindowTextA(hModeButton, "switch to crypto mode");
                SetWindowTextA(hModeLabel, "mode: equities");
                SetWindowTextA(hAppLabel, "equities");
                SetWindowTextA(hInfoLabel, "Enter an equity ticker symbol. (1-4 characters)");
            } else {
                SetWindowTextA(hModeButton, "Switch to Stock Mode");
                SetWindowTextA(hModeLabel, "mode: crypto");
                SetWindowTextA(hAppLabel, "cryptocurrencies");
                SetWindowTextA(hInfoLabel, "Enter a cryptocurrency name. (1-15 characters)");
            }
        }
        else if (LOWORD(wParam) == ID_BUTTON) {
            char buffer[16] = {0};  // big enough for either case

            if (g_isEquitiesMode) {
                GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, 5); // limit stock tickers
            } else {
                GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, sizeof(buffer));
            }

            std::string asset(buffer);

            // trim whitespace
            asset.erase(asset.begin(), std::find_if(asset.begin(), asset.end(),
                [](unsigned char ch){ return !std::isspace(ch); }));
            asset.erase(std::find_if(asset.rbegin(), asset.rend(),
                [](unsigned char ch){ return !std::isspace(ch); }).base(), asset.end());

            if (asset.empty()) {
                if (g_isEquitiesMode) {
                    MessageBoxA(hwnd, "Invalid equity ticker input", "Error", MB_OK | MB_ICONERROR);
                } else {
                    MessageBoxA(hwnd, "Invalid cryptocurrency name input", "Error", MB_OK | MB_ICONERROR);
                }
                
                break;
            }

            // Run correct Python script
            std::string command;
            if (g_isEquitiesMode)
                command = "python scripts/equities.py " + asset;
            else
                command = "python scripts/cryptos.py " + asset;

            system(command.c_str());

            // Load CSV as before
            std::string csvPath = "data/" + asset + ".csv";
            auto rows = LoadCSV(csvPath);
            std::ostringstream out;
            for (const auto& row : rows) {
                for (size_t i = 0; i < row.size(); i++) {
                    out << row[i];
                    if (i < row.size() - 1) out << " :: ";
                }
                out << "\r\n";
            }
            SetWindowTextA(hTextDisplay, out.str().c_str());

            std::string price="--", change="--", dollarChange="--";
            for (const auto& row : rows) {
                if (row.size()>=2) {
                    if (row[0]=="Price") price=row[1];
                    else if (row[0]=="% Change (24hrs)" || row[0]=="% Change") change=row[1];
                    else if (row[0]=="Dollar Change (24hrs)") dollarChange=row[1];
                }
            }
            changeColor = (change[0]=='-')?RGB(255,0,0):RGB(0,255,0);
            dollarChangeColor = (dollarChange[0]=='-')?RGB(255,0,0):RGB(0,255,0);
            SetWindowTextA(hPriceLabel, ("Price ($): "+price).c_str());
            if (g_isEquitiesMode) {
                SetWindowTextA(hChangeLabel, ("% Change: "+change).c_str());
                SetWindowTextA(hDollarChangeLabel, ("$ Change: "+dollarChange).c_str());
            } else {
                SetWindowTextA(hChangeLabel, ("% Change (24hrs): "+change).c_str());
                SetWindowTextA(hDollarChangeLabel, ("$ Change (24hrs): "+dollarChange).c_str());
            }            

            g_currentAsset = asset;
            SetTimer(hwnd, ID_TIMER, 15000, NULL);
        }
        return 0;
    }
    case WM_TIMER: {
        if (wParam == ID_TIMER && !g_currentAsset.empty()) {
            std::string command;
            if (g_isEquitiesMode)
                command = "python scripts/equities.py " + g_currentAsset;
            else
                command = "python scripts/cryptos.py " + g_currentAsset;

            system(command.c_str());

            std::string csvPath = "data/" + g_currentAsset + ".csv";
            auto rows = LoadCSV(csvPath);
            std::ostringstream out;
            for (const auto& row : rows) {
                for (size_t i = 0; i < row.size(); i++) {
                    out << row[i];
                    if (i < row.size() - 1) out << " :: ";
                }
                out << "\r\n";
            }
            SetWindowTextA(hTextDisplay, out.str().c_str());

            std::string price="--", change="--", dollarChange="--";
            for (const auto& row : rows) {
                if (row.size()>=2) {
                    if (row[0]=="Price") price=row[1];
                    else if (row[0]=="Change") change=row[1];
                    else if (row[0]=="Dollar Change") dollarChange=row[1];
                }
            }
            changeColor = (change[0]=='-')?RGB(255,0,0):RGB(0,255,0);
            dollarChangeColor = (dollarChange[0]=='-')?RGB(255,0,0):RGB(0,255,0);
            SetWindowTextA(hPriceLabel, ("Price ($): "+price).c_str());
            SetWindowTextA(hChangeLabel, ("% Change: "+change).c_str());
            SetWindowTextA(hDollarChangeLabel, ("$ Change: "+dollarChange).c_str());
        }
        else if (wParam == ID_DATETIME_TIMER) {
            SYSTEMTIME st; GetLocalTime(&st);
            char datetimeStr[100];
            snprintf(datetimeStr,sizeof(datetimeStr),
                "%04d-%02d-%02d %02d:%02d:%02d",
                st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
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

// ---------------- Entry ----------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const char CLASS_NAME[] = "markets-app";
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "markets-app",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 700, 700,
        NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
