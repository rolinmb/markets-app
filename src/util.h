#pragma once
#include <windows.h>
#include <vector>
#include <string>

#define ID_EDITBOX        1
#define ID_BUTTON         2
#define ID_MODE_BUTTON    3
#define ID_TIMER          100
#define ID_DATETIME_TIMER 101
#define ID_BG_TIMER       102

// Control handles
extern HWND hTextDisplay, hPriceLabel, hChangeLabel, hDollarChangeLabel, hDateTimeLabel, hInfoLabel, hAppLabel;
extern HWND hModeButton;
extern HWND hImageView;
extern HWND hComboBox;

// GDI objects
extern HBITMAP hCurrentBmp;
extern HBRUSH hBrushBlack;

// State
extern std::string g_currentAsset;
extern COLORREF textColor;
extern COLORREF changeColor;
extern COLORREF dollarChangeColor;
extern COLORREF g_colors[3];
extern int g_currentIndex;
extern int g_nextIndex;
extern double g_t;

// Mode enum (shared between files)
enum class AssetMode { Equities, Crypto, Forex, Commodities };
extern AssetMode g_currentMode;

// Utility functions
AssetMode NextMode(AssetMode current);
std::string ModeName(AssetMode mode);
void ClearDirectory(const std::string& dirPath);
std::vector<std::vector<std::string>> LoadCSV(const std::string& path);
std::vector<std::string> SplitCSVLine(const std::string& line);
std::string GetSelectedAsset(HWND hwnd);
void FetchAndDisplay(HWND hwnd, const std::string& asset);
void LoadAndShowBMP(HWND hwnd, const std::string& asset);
COLORREF LerpColor(COLORREF c1, COLORREF c2, double t);