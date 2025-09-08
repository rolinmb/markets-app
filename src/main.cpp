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
    if (!file.is_open()) return rows;
    std::string line;
    // Skip header
    std::getline(file, line);
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

HWND hTextDisplay, hPriceLabel, hChangeLabel, hDollarChangeLabel, hDateTimeLabel;
HBITMAP hChartBitmap = nullptr; // global bitmap

std::string g_currentTicker;

HBRUSH hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
COLORREF textColor = RGB(255, 255, 255);
COLORREF changeColor = RGB(255, 255, 255);
COLORREF dollarChangeColor = RGB(255, 255, 255);

void LoadChartBitmap(const std::string& ticker) {
    if (hChartBitmap) {
        DeleteObject(hChartBitmap);
        hChartBitmap = nullptr;
    }
    std::string bmpPath = "img/" + ticker + "_close.bmp"; // Python should save BMP
    hChartBitmap = (HBITMAP)LoadImageA(NULL, bmpPath.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

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
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // Draw chart bitmap if available
        if (hChartBitmap) {
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hChartBitmap);
            // 2:3 ratio display
            int w = 300, h = 200;
            BitBlt(hdc, 50, 333, w, h, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteDC(memDC);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_CREATE: {
        CreateWindowExA(0, "STATIC", "markets-app", WS_CHILD | WS_VISIBLE, 50, 20, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        CreateWindowExA(0, "STATIC", "Enter a ticker (no more than 4 characters).", WS_CHILD | WS_VISIBLE, 50, 45, 135, 50, hwnd, NULL, GetModuleHandle(NULL), NULL);

        HWND hEdit = CreateWindowExA(0, "EDIT", "SPY", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT, 50, 100, 135, 25, hwnd, (HMENU)ID_EDITBOX, GetModuleHandle(NULL), NULL);
        SendMessageA(hEdit, EM_SETLIMITTEXT, 4, 0);

        CreateWindowExA(0, "BUTTON", "Fetch Data", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 50, 130, 100, 30, hwnd, (HMENU)ID_BUTTON, GetModuleHandle(NULL), NULL);

        hTextDisplay = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 225, 20, 250, 400, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hPriceLabel = CreateWindowExA(0, "STATIC", "Price ($): --", WS_CHILD | WS_VISIBLE, 50, 170, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hChangeLabel = CreateWindowExA(0, "STATIC", "% Change: --", WS_CHILD | WS_VISIBLE, 50, 195, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hDollarChangeLabel = CreateWindowExA(0, "STATIC", "$ Change: --", WS_CHILD | WS_VISIBLE, 50, 220, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        hDateTimeLabel = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE, 50, 250, 135, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        SetTimer(hwnd, ID_DATETIME_TIMER, 1000, NULL);
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON) {
            char buffer[5] = {0};
            GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, 5);
            std::string ticker(buffer);
            ticker.erase(ticker.begin(), std::find_if(ticker.begin(), ticker.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            ticker.erase(std::find_if(ticker.rbegin(), ticker.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), ticker.end());

            if (ticker.empty() || !std::all_of(ticker.begin(), ticker.end(), [](unsigned char ch){ return std::isalpha(ch); })) {
                MessageBoxA(hwnd, "Invalid ticker input", "Error", MB_OK | MB_ICONERROR);
                break;
            }

            // Run Python script
            std::string command = "python scripts/main.py " + ticker;
            system(command.c_str());

            std::string csvPath = "data/" + ticker + ".csv";
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

            // Load chart bitmap
            LoadChartBitmap(ticker);
            InvalidateRect(hwnd, NULL, TRUE); // force repaint

            g_currentTicker = ticker;
            SetTimer(hwnd, ID_TIMER, 15000, NULL);
        }
        return 0;
    }
    case WM_TIMER: {
        if (wParam == ID_TIMER && !g_currentTicker.empty()) {
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

            LoadChartBitmap(g_currentTicker);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        else if (wParam == ID_DATETIME_TIMER) {
            SYSTEMTIME st; GetLocalTime(&st);
            char datetimeStr[100];
            snprintf(datetimeStr,sizeof(datetimeStr),"%04d-%02d-%02d %02d:%02d:%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
            SetWindowTextA(hDateTimeLabel, datetimeStr);
        }
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, ID_TIMER);
        KillTimer(hwnd, ID_DATETIME_TIMER);
        if (hChartBitmap) DeleteObject(hChartBitmap);
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

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "markets-app", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 777, 777, NULL, NULL, hInstance, NULL);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return 0;
}
