#include <windows.h>
#include <shlwapi.h>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include "util.h"
// ---------------- Window Procedure ----------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        if ((HWND)lParam == hChangeLabel) SetTextColor(hdc, changeColor);
        else if ((HWND)lParam == hDollarChangeLabel) SetTextColor(hdc, dollarChangeColor);
        else SetTextColor(hdc, textColor);
        SetBkColor(hdc, RGB(0,0,0));
        return (INT_PTR)hBrushBlack;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, textColor);
        SetBkColor(hdc, RGB(0,0,0));
        return (INT_PTR)hBrushBlack;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        COLORREF bg = LerpColor(g_colors[g_currentIndex], g_colors[g_nextIndex], g_t);
        HBRUSH hBrush = CreateSolidBrush(bg);
        FillRect(hdc, &rc, hBrush);
        DeleteObject(hBrush);
        return 1;
    }
    case WM_CREATE: {
        hAppLabel = CreateWindowExA(0,"STATIC","Equities",WS_CHILD|WS_VISIBLE,
            50,20,200,40,hwnd,NULL,GetModuleHandle(NULL),NULL);

        hInfoLabel = CreateWindowExA(0,"STATIC","Enter an equity ticker symbol. (1-4 chars)",WS_CHILD|WS_VISIBLE,
            50,65,250,40,hwnd,NULL,GetModuleHandle(NULL),NULL);

        HWND hEdit = CreateWindowExA(0,"EDIT","SPY",
            WS_CHILD|WS_VISIBLE|WS_BORDER|ES_LEFT,
            50,110,135,25,hwnd,(HMENU)ID_EDITBOX,GetModuleHandle(NULL),NULL);
        SendMessageA(hEdit, EM_SETLIMITTEXT, 15, 0);

        hComboBox = CreateWindowExA(0, "COMBOBOX", NULL,
            WS_CHILD | CBS_DROPDOWNLIST,
            50, 110, 200, 300,
            hwnd, NULL, GetModuleHandle(NULL), NULL);

        const char* commodities[] = {"wti","brent","natural_gas","copper","aluminum","wheat","corn","cotton","sugar","coffee","all_commodities"};
        for (auto c : commodities) SendMessageA(hComboBox, CB_ADDSTRING, 0, (LPARAM)c);
        ShowWindow(hComboBox, SW_HIDE);

        CreateWindowExA(0,"BUTTON","Fetch Data",
            WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
            50,135,100,30,hwnd,(HMENU)ID_BUTTON,GetModuleHandle(NULL),NULL);

        hTextDisplay = CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT","",
            WS_CHILD|WS_VISIBLE|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_READONLY,
            300,20,300,200,hwnd,NULL,GetModuleHandle(NULL),NULL);

        hPriceLabel = CreateWindowExA(0,"STATIC","Price: --",WS_CHILD|WS_VISIBLE,50,170,200,30,hwnd,NULL,GetModuleHandle(NULL),NULL);
        hChangeLabel = CreateWindowExA(0,"STATIC","% Change: --",WS_CHILD|WS_VISIBLE,50,205,200,30,hwnd,NULL,GetModuleHandle(NULL),NULL);
        hDollarChangeLabel = CreateWindowExA(0,"STATIC","$ Change: --",WS_CHILD|WS_VISIBLE,50,240,200,30,hwnd,NULL,GetModuleHandle(NULL),NULL);
        hDateTimeLabel = CreateWindowExA(0,"STATIC","",WS_CHILD|WS_VISIBLE,50,300,200,20,hwnd,NULL,GetModuleHandle(NULL),NULL);

        hModeButton = CreateWindowExA(0,"BUTTON","switch to crypto mode",
            WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
            50,400,200,30,hwnd,(HMENU)ID_MODE_BUTTON,GetModuleHandle(NULL),NULL);
        
        hImageView = CreateWindowExA(0, "STATIC", NULL,
            WS_CHILD | WS_VISIBLE | SS_BITMAP,
            300, 240, 350, 350,hwnd, NULL, GetModuleHandle(NULL), NULL);

        SetTimer(hwnd, ID_DATETIME_TIMER, 1000, NULL);
        SetTimer(hwnd, ID_BG_TIMER, 50, NULL);
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_MODE_BUTTON) {
            g_currentMode = NextMode(g_currentMode);
            g_currentAsset.clear();

            int limit = (g_currentMode == AssetMode::Equities || g_currentMode == AssetMode::Options) ? 4 :
                        (g_currentMode == AssetMode::Crypto)   ? 15 :
                        (g_currentMode == AssetMode::Forex)    ? 6 : 10;
            SendMessageA(GetDlgItem(hwnd, ID_EDITBOX), EM_SETLIMITTEXT, limit, 0);

            switch(g_currentMode) {
            case AssetMode::Equities:
                SetWindowTextA(hModeButton,"switch to crypto mode");
                SetWindowTextA(hAppLabel,"Equities");
                SetWindowTextA(hInfoLabel,"Enter an equity ticker symbol. (1-4 chars)");
                ShowWindow(GetDlgItem(hwnd, ID_EDITBOX), SW_SHOW);
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(hPriceLabel, SW_SHOW);
                ShowWindow(hChangeLabel, SW_SHOW);
                ShowWindow(hDollarChangeLabel, SW_SHOW);
                break;
            case AssetMode::Crypto:
                SetWindowTextA(hModeButton,"switch to forex mode");
                SetWindowTextA(hAppLabel,"Cryptocurrencies");
                SetWindowTextA(hInfoLabel,"Enter a cryptocurrency name. (1-15 chars)");
                ShowWindow(GetDlgItem(hwnd, ID_EDITBOX), SW_SHOW);
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(hPriceLabel, SW_SHOW);
                ShowWindow(hChangeLabel, SW_SHOW);
                ShowWindow(hDollarChangeLabel, SW_SHOW);
                break;
            case AssetMode::Forex:
                SetWindowTextA(hModeButton,"switch to commodities mode");
                SetWindowTextA(hAppLabel,"Foreign Exchange");
                SetWindowTextA(hInfoLabel,"Enter a currency pair (e.g., eurusd, max 6 chars)");
                ShowWindow(GetDlgItem(hwnd, ID_EDITBOX), SW_SHOW);
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(hPriceLabel, SW_SHOW);
                ShowWindow(hChangeLabel, SW_SHOW);
                ShowWindow(hDollarChangeLabel, SW_SHOW);
                break;
            case AssetMode::Commodities:
                SetWindowTextA(hModeButton,"switch to bonds mode");
                SetWindowTextA(hAppLabel,"Commodities");
                SetWindowTextA(hInfoLabel,"Select a commodity from the list:");
                ShowWindow(GetDlgItem(hwnd, ID_EDITBOX), SW_HIDE);
                ShowWindow(hComboBox, SW_SHOW);
                ShowWindow(hPriceLabel, SW_SHOW);
                ShowWindow(hChangeLabel, SW_SHOW);
                ShowWindow(hDollarChangeLabel, SW_SHOW);
                break;
            case AssetMode::Bonds:
                SetWindowTextA(hModeButton,"switch to options mode");
                SetWindowTextA(hAppLabel,"Bonds");
                SetWindowTextA(hInfoLabel,"US Treasury Yield Curve");
                ShowWindow(GetDlgItem(hwnd, ID_EDITBOX), SW_HIDE);
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(hPriceLabel, SW_HIDE); // Hide price/change/dollar labels for this mode
                ShowWindow(hChangeLabel, SW_HIDE);
                ShowWindow(hDollarChangeLabel, SW_HIDE);
                break;
            case AssetMode::Options:
                SetWindowTextA(hModeButton,"switch to equities mode");
                SetWindowTextA(hAppLabel,"Options");
                SetWindowTextA(hInfoLabel,"Enter an underlying ticker (1-4 chars)");
                ShowWindow(GetDlgItem(hwnd, ID_EDITBOX), SW_SHOW);
                ShowWindow(hComboBox, SW_HIDE);
                ShowWindow(hPriceLabel, SW_HIDE); // Hide price/change/dollar labels for this mode
                ShowWindow(hChangeLabel, SW_HIDE);
                ShowWindow(hDollarChangeLabel, SW_HIDE);
                break;
            }
            SetWindowTextA(hTextDisplay, "");
            SetWindowTextA(hPriceLabel, "Price: --");
            SetWindowTextA(hChangeLabel, "% Change: --");
            SetWindowTextA(hDollarChangeLabel, "$ Change: --");
        }
        else if (LOWORD(wParam) == ID_BUTTON) {
            std::string asset = GetSelectedAsset(hwnd);
            FetchAndDisplay(hwnd, asset);
        }
        return 0;
    }
    case WM_TIMER: {
        if(wParam == ID_TIMER && !g_currentAsset.empty()) {
            FetchAndDisplay(hwnd, g_currentAsset);
        } else if(wParam == ID_DATETIME_TIMER) {
            SYSTEMTIME st; GetLocalTime(&st);
            char datetimeStr[100];
            snprintf(datetimeStr,sizeof(datetimeStr),"%04d-%02d-%02d %02d:%02d:%02d",
                st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
            SetWindowTextA(hDateTimeLabel,datetimeStr);
        } else if (wParam == ID_BG_TIMER) {
            g_t += 0.01;
            if (g_t >= 1.0) {
                g_t = 0.0;
                g_currentIndex = g_nextIndex;
                g_nextIndex = (g_nextIndex + 1) % 3;
            }
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd,ID_TIMER);
        KillTimer(hwnd,ID_DATETIME_TIMER);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd,uMsg,wParam,lParam);
}
// ---------------- Entry ----------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ClearDirectory("data");
    ClearDirectory("img");
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
