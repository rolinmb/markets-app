#pragma once
#include <windows.h>
#include <vector>
#include <string>

void ClearDirectory(const std::string& dirPath);

std::vector<std::string> SplitCSVLine(const std::string& line);
std::vector<std::vector<std::string>> LoadCSV(const std::string& path);

void LoadAndShowBMP(HWND hwnd, const std::string& asset);

COLORREF LerpColor(COLORREF c1, COLORREF c2, double t);