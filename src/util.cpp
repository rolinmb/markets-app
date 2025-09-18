#include "util.h"
#include <windows.h>
#include <cstring>
#include <fstream>
#include <sstream>

extern HWND hImageView;
extern HBITMAP hCurrentBmp;

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