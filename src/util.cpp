#include "util.h"
#include <windows.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype> 
// ---------------- Globals ----------------
HBITMAP hCurrentBmp = NULL;
HWND hComboBox = NULL;
HBRUSH hBrushBlack = CreateSolidBrush(RGB(0,0,0));
COLORREF textColor = RGB(255,255,255);
COLORREF changeColor = RGB(255,255,255);
COLORREF dollarChangeColor = RGB(255,255,255);
COLORREF g_colors[3] = { RGB(200,220,255), RGB(220,255,200), RGB(255,220,240) };
int g_currentIndex = 0;
int g_nextIndex = 1;
double g_t = 0.0;

// other handles
HWND hTextDisplay, hPriceLabel, hChangeLabel, hDollarChangeLabel, hDateTimeLabel, hInfoLabel, hAppLabel;
HWND hModeButton;
HWND hImageView;

std::string g_currentAsset;

AssetMode g_currentMode = AssetMode::Equities;

AssetMode NextMode(AssetMode current) {
    switch(current) {
        case AssetMode::Equities: return AssetMode::Crypto;
        case AssetMode::Crypto:   return AssetMode::Forex;
        case AssetMode::Forex:    return AssetMode::Commodities;
        case AssetMode::Commodities: return AssetMode::Bonds;
        case AssetMode::Bonds:       return AssetMode::Equities;
    }
    return AssetMode::Equities;
}

std::string ModeName(AssetMode mode) {
    switch(mode) {
        case AssetMode::Equities: return "equities";
        case AssetMode::Crypto:   return "crypto";
        case AssetMode::Forex:    return "forex";
        case AssetMode::Commodities: return "commodities";
        case AssetMode::Bonds:       return "bonds";
    }
    return "unknown";
}
// ---------------- Helpers ----------------
void ClearDirectory(const std::string& dirPath) {
    CreateDirectoryA(dirPath.c_str(), NULL);
    std::string pattern = dirPath + "\\*";
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &ffd);
    if (hFind == INVALID_HANDLE_VALUE) return;
    do {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        std::string filePath = dirPath + "\\" + ffd.cFileName;
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            ClearDirectory(filePath);
            RemoveDirectoryA(filePath.c_str());
        } else {
            DeleteFileA(filePath.c_str());
        }
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);
}

std::vector<std::string> SplitCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::string cell;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') inQuotes = !inQuotes;
        else if (c == ',' && !inQuotes) {
            result.push_back(cell);
            cell.clear();
        } else cell.push_back(c);
    }
    result.push_back(cell);
    return result;
}

std::vector<std::vector<std::string>> LoadCSV(const std::string& path) {
    std::ifstream file(path);
    std::vector<std::vector<std::string>> rows;
    if (!file.is_open()) return rows;
    std::string line;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) rows.push_back(SplitCSVLine(line));
    return rows;
}

void LoadAndShowBMP(HWND hwnd, const std::string& asset) {
    // Build path to BMP
    std::string bmpPath = "img/" + asset + ".bmp";

    // Free old bitmap
    if (hCurrentBmp) {
        DeleteObject(hCurrentBmp);
        hCurrentBmp = NULL;
    }

    // Load new BMP
    hCurrentBmp = (HBITMAP)LoadImageA(
        NULL,
        bmpPath.c_str(),
        IMAGE_BITMAP,
        0, 0,
        LR_LOADFROMFILE
    );

    if (hCurrentBmp) {
        SendMessage(hImageView, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hCurrentBmp);
    } else {
        MessageBoxA(hwnd, ("Failed to load BMP: " + bmpPath).c_str(), "Error", MB_OK | MB_ICONERROR);
    }
}

COLORREF LerpColor(COLORREF c1, COLORREF c2, double t) {
    int r1 = GetRValue(c1), g1 = GetGValue(c1), b1 = GetBValue(c1);
    int r2 = GetRValue(c2), g2 = GetGValue(c2), b2 = GetBValue(c2);
    int r = (int)(r1 + (r2 - r1) * t);
    int g = (int)(g1 + (g2 - g1) * t);
    int b = (int)(b1 + (b2 - b1) * t);
    return RGB(r, g, b);
}
// ---------------- Fetch Helper ----------------
std::string GetSelectedAsset(HWND hwnd) {
    std::string asset;
    if (g_currentMode == AssetMode::Commodities) {
        int sel = (int)SendMessageA(hComboBox, CB_GETCURSEL, 0, 0);
        if (sel != CB_ERR) {
            char buffer[64];
            SendMessageA(hComboBox, CB_GETLBTEXT, sel, (LPARAM)buffer);
            asset = buffer;
        }
    } else {
        int maxLen = (g_currentMode == AssetMode::Equities) ? 5 :
                     (g_currentMode == AssetMode::Crypto) ? 16 : 7;
        char buffer[16] = {0};
        GetWindowTextA(GetDlgItem(hwnd, ID_EDITBOX), buffer, maxLen);
        asset = buffer;
    }

    // trim whitespace
    asset.erase(
        asset.begin(),
        std::find_if(asset.begin(), asset.end(),
                    [](unsigned char c){ return !std::isspace(c); })
    );

        // trim trailing whitespace
    asset.erase(
        std::find_if(asset.rbegin(), asset.rend(),
                    [](unsigned char c){ return !std::isspace(c); }).base(),
        asset.end()
    );
        return asset;
}

void FetchAndDisplay(HWND hwnd, const std::string& asset) {
    if (asset.empty()) {
        MessageBoxA(hwnd, "Invalid input", "Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::string command;
    switch(g_currentMode) {
        case AssetMode::Equities: command = "python scripts/equities.py " + asset; break;
        case AssetMode::Crypto:   command = "python scripts/cryptos.py " + asset; break;
        case AssetMode::Forex:    command = "python scripts/forex.py " + asset; break;
        case AssetMode::Commodities: command = "python scripts/commodities.py " + asset; break;
        case AssetMode::Bonds:    command = "python scripts/bonds.py"; break;
    }
    system(command.c_str());

    SetWindowTextA(hTextDisplay,"");
    SetWindowTextA(hPriceLabel,"Price: --");
    SetWindowTextA(hChangeLabel,"% Change: --");
    SetWindowTextA(hDollarChangeLabel,"$ Change: --");

    LoadAndShowBMP(hwnd, asset);

    std::string csvPath = "data/" + asset + ".csv";
    auto rows = LoadCSV(csvPath);

    if(rows.empty() && g_currentMode != AssetMode::Commodities) {
        MessageBoxA(hwnd,"Failed to fetch data.","Error",MB_OK|MB_ICONERROR);
        g_currentAsset.clear();
        return;
    }

    g_currentAsset = asset;

    std::ostringstream out;
    for(const auto& row: rows) {
        for(size_t i=0;i<row.size();i++) {
            out << row[i];
            if(i<row.size()-1) out << " :: ";
        }
        out << "\r\n";
    }
    SetWindowTextA(hTextDisplay,out.str().c_str());

    std::string price="--", change="--", dollarChange="--";
    for(const auto& row: rows) {
        if(row.size()>=2) {
            if(row[0]=="Price") price=row[1];
            else if(row[0]=="Change" || row[0]=="% Change (24hrs)") change=row[1];
            else if(row[0]=="$ Change" || row[0]=="$ Change (24hrs)") dollarChange=row[1];
        }
    }
    changeColor = (change[0]=='-')?RGB(255,0,0):RGB(0,255,0);
    dollarChangeColor = (dollarChange[0]=='-')?RGB(255,0,0):RGB(0,255,0);

    SetWindowTextA(hPriceLabel,("Price: "+price).c_str());
    SetWindowTextA(hChangeLabel,("% Change: "+change).c_str());
    SetWindowTextA(hDollarChangeLabel,("$ Change: "+dollarChange).c_str());

    SetTimer(hwnd, ID_TIMER, 15000, NULL);
}