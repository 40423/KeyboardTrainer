#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include "Exercise.h"
#include "DatabaseManager.h"

enum Screen {
    SCREEN_PROFILES,
    SCREEN_CREATE,
    SCREEN_AUTH,
    SCREEN_MENU,
    SCREEN_MODE,
    SCREEN_EXERCISE,
    SCREEN_RESULTS,
    SCREEN_HISTORY,
    SCREEN_SETTINGS,
    SCREEN_HEATMAP
};

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool create(HINSTANCE hInstance, int nCmdShow);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    void onCreate(HWND hwnd);
    void onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void onTimer(HWND hwnd, WPARAM wParam);

    void showScreen(Screen screen);
    void loadHistory();
    void loadSettingsIntoUI();

    void startExercise(HWND hwnd);
    void finishExercise(HWND hwnd);
    void updateStats(HWND hwnd);

    void layout();

    void drawHeatmap(HDC hdc);
    COLORREF heatmapColor(int errorCount, int maxErrors);

    HINSTANCE m_hInstance;
    HWND m_hwnd;

    int m_winW;
    int m_winH;

    std::vector<HWND> m_profileScreenElements;
    std::vector<HWND> m_createScreenElements;
    std::vector<HWND> m_authScreenElements;
    std::vector<HWND> m_menuScreenElements;
    std::vector<HWND> m_modeScreenElements;
    std::vector<HWND> m_exerciseScreenElements;
    std::vector<HWND> m_resultsScreenElements;
    std::vector<HWND> m_historyScreenElements;
    std::vector<HWND> m_settingsScreenElements;
    std::vector<HWND> m_heatmapScreenElements;

    Exercise m_exercise;
    Screen   m_currentScreen;

    int m_selectedMode;
    int m_selectedDuration;
    int m_selectedRow;
    int m_selectedProfileId;
    int m_elapsedSeconds;
    int m_remainingSeconds;
    bool m_exerciseActive;
    int m_lastExerciseId;

    int m_lastWpm;
    int m_lastErrorRate;
    int m_lastElapsedSeconds;
    int m_lastTotalChars;

    static MainWindow* s_instance;
};