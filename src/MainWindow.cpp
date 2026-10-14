#include "MainWindow.h"
#include "ExerciseFactory.h"
#include "Settings.h"
#include "FlatButton.h"
#include <commctrl.h>
#include <cstdio>
#include <mmsystem.h>

#define ID_LIST_PROFILES 201
#define ID_BTN_CREATE    101
#define ID_BTN_DELETE    102
#define ID_BTN_LOGIN     103

#define ID_EDIT_LOGIN    301
#define ID_EDIT_PASSWORD 302
#define ID_BTN_SAVE      401
#define ID_BTN_BACK      402

#define ID_EDIT_AUTH_PASSWORD 501
#define ID_BTN_AUTH_LOGIN     502
#define ID_BTN_AUTH_BACK      503
#define ID_LABEL_AUTH_PROFILE 504

#define ID_BTN_MENU_START     601
#define ID_BTN_MENU_HISTORY   602
#define ID_BTN_MENU_HEATMAP   603
#define ID_BTN_MENU_SETTINGS  604
#define ID_BTN_MENU_EXIT      605

#define ID_RADIO_LETTERS   701
#define ID_RADIO_SYLLABLES 702
#define ID_RADIO_WORDS     703
#define ID_RADIO_TEXT      704
#define ID_COMBO_DURATION  705
#define ID_COMBO_ROW       706
#define ID_BTN_MODE_START  707
#define ID_BTN_MODE_BACK   708

#define ID_LABEL_WPM           801
#define ID_LABEL_ERRORS        802
#define ID_LABEL_TIME          803
#define ID_LABEL_TEXT          804
#define ID_EDIT_TYPING         805
#define ID_BTN_EXERCISE_FINISH 806
#define ID_BTN_EXERCISE_BACK   807

#define ID_LABEL_RESULT_WPM      901
#define ID_LABEL_RESULT_ERRORS   902
#define ID_LABEL_RESULT_TIME     903
#define ID_LABEL_RESULT_CHARS    904
#define ID_BTN_RESULT_HEATMAP    905
#define ID_BTN_RESULT_MENU       906

#define ID_LIST_HISTORY     1001
#define ID_BTN_HISTORY_BACK 1002
#define ID_BTN_HISTORY_CLEAR 1003

#define ID_CHK_SOUND           1101
#define ID_TRACK_VOLUME        1102
#define ID_RADIO_THEME_LIGHT   1103
#define ID_RADIO_THEME_DARK    1104
#define ID_EDIT_FONT           1105
#define ID_BTN_SETTINGS_SAVE   1106
#define ID_BTN_SETTINGS_BACK   1107
#define ID_BTN_HEATMAP_BACK 1201
#define ID_HEATMAP_AREA     1202
#define TIMER_EXERCISE_ID 1

MainWindow* MainWindow::s_instance = nullptr;

static void roundWindowCorners(HWND hwnd, int radius)
{
    if (!hwnd) return;
    RECT rc;
    GetWindowRect(hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    HRGN hRgn = CreateRoundRectRgn(0, 0, w + 1, h + 1, radius, radius);
    SetWindowRgn(hwnd, hRgn, TRUE);
}

static bool isPrimaryButton(int id)
{
    return id == ID_BTN_SAVE         ||
           id == ID_BTN_LOGIN        ||
           id == ID_BTN_CREATE       ||
           id == ID_BTN_AUTH_LOGIN   ||
           id == ID_BTN_MODE_START   ||
           id == ID_BTN_MENU_START   ||
           id == ID_BTN_EXERCISE_FINISH ||
           id == ID_BTN_RESULT_HEATMAP  ||
           id == ID_BTN_SETTINGS_SAVE;
}

MainWindow::MainWindow()
    : m_hInstance(nullptr), m_hwnd(nullptr),
      m_selectedMode(3), m_selectedDuration(60), m_selectedRow(0),
      m_selectedProfileId(-1),
      m_elapsedSeconds(0), m_remainingSeconds(60),
      m_exerciseActive(false),
      m_lastExerciseId(-1),
      m_currentScreen(SCREEN_PROFILES),
      m_lastWpm(0), m_lastErrorRate(0),
      m_lastElapsedSeconds(0), m_lastTotalChars(0),
      m_winW(1280), m_winH(720)
{
    s_instance = this;
}

MainWindow::~MainWindow()
{
    s_instance = nullptr;
}

bool MainWindow::create(HINSTANCE hInstance, int nCmdShow)
{
    m_hInstance = hInstance;

    const wchar_t CLASS_NAME[] = L"KeyboardTrainerClass";

    RECT workArea;
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &workArea, 0);
    m_winW = workArea.right - workArea.left;
    m_winH = workArea.bottom - workArea.top;

    WNDCLASSW wc = {};
    wc.lpfnWndProc   = MainWindow::WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(Settings::getInstance().getBackgroundColor());
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&wc)) return false;

    m_hwnd = CreateWindowExW(
        0, CLASS_NAME, L"KeyboardTrainer",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        workArea.left, workArea.top,
        m_winW, m_winH,
        NULL, NULL, hInstance, this
    );

    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
}

void MainWindow::showScreen(Screen screen)
{
    m_currentScreen = screen;
    for (HWND h : m_profileScreenElements)  ShowWindow(h, SW_HIDE);
    for (HWND h : m_createScreenElements)   ShowWindow(h, SW_HIDE);
    for (HWND h : m_authScreenElements)     ShowWindow(h, SW_HIDE);
    for (HWND h : m_menuScreenElements)     ShowWindow(h, SW_HIDE);
    for (HWND h : m_modeScreenElements)     ShowWindow(h, SW_HIDE);
    for (HWND h : m_exerciseScreenElements) ShowWindow(h, SW_HIDE);
    for (HWND h : m_resultsScreenElements)  ShowWindow(h, SW_HIDE);
    for (HWND h : m_historyScreenElements)  ShowWindow(h, SW_HIDE);
    for (HWND h : m_settingsScreenElements) ShowWindow(h, SW_HIDE);
    for (HWND h : m_heatmapScreenElements)  ShowWindow(h, SW_HIDE);

    auto show = [](std::vector<HWND>& v) {
        for (HWND h : v) ShowWindow(h, SW_SHOW);
    };

    switch (screen) {
    case SCREEN_PROFILES: show(m_profileScreenElements); break;
    case SCREEN_CREATE:   show(m_createScreenElements);  break;
    case SCREEN_AUTH:     show(m_authScreenElements);    break;
    case SCREEN_MENU:     show(m_menuScreenElements);    break;
    case SCREEN_MODE:     show(m_modeScreenElements);    break;
    case SCREEN_EXERCISE: show(m_exerciseScreenElements); break;
    case SCREEN_RESULTS:  show(m_resultsScreenElements); break;
    case SCREEN_HISTORY:  show(m_historyScreenElements); break;
    case SCREEN_SETTINGS: show(m_settingsScreenElements); break;
    case SCREEN_HEATMAP:  show(m_heatmapScreenElements);  break;
    }

    HWND hWpm    = GetDlgItem(m_hwnd, ID_LABEL_WPM);
    HWND hErr    = GetDlgItem(m_hwnd, ID_LABEL_ERRORS);
    HWND hTime   = GetDlgItem(m_hwnd, ID_LABEL_TIME);
    HWND hText   = GetDlgItem(m_hwnd, ID_LABEL_TEXT);
    HWND hTyping = GetDlgItem(m_hwnd, ID_EDIT_TYPING);

    if (screen == SCREEN_EXERCISE) {
        if (hWpm)    ShowWindow(hWpm,    SW_SHOW);
        if (hErr)    ShowWindow(hErr,    SW_SHOW);
        if (hTime)   ShowWindow(hTime,   SW_SHOW);
        if (hText)   ShowWindow(hText,   SW_SHOW);
        if (hTyping) ShowWindow(hTyping, SW_SHOW);
    } else {
        if (hWpm)    ShowWindow(hWpm,    SW_HIDE);
        if (hErr)    ShowWindow(hErr,    SW_HIDE);
        if (hTime)   ShowWindow(hTime,   SW_HIDE);
        if (hText)   ShowWindow(hText,   SW_HIDE);
        if (hTyping) ShowWindow(hTyping, SW_HIDE);
    }

    InvalidateRect(m_hwnd, NULL, TRUE);
    UpdateWindow(m_hwnd);
}

void MainWindow::layout() {}

void MainWindow::loadHistory()
{
    HWND hList = GetDlgItem(m_hwnd, ID_LIST_HISTORY);
    if (!hList) return;

    // Очищаем список
    ListView_DeleteAllItems(hList);

    if (m_selectedProfileId < 0) return;

    auto history = DatabaseManager::getInstance().getHistory(m_selectedProfileId);

    const wchar_t* modeNames[] = { L"Буквы", L"Слоги", L"Слова", L"Текст" };

    int rowIndex = 0;
    for (const auto& e : history) {
        const wchar_t* mode = (e.mode >= 0 && e.mode <= 3) ? modeNames[e.mode] : L"?";
        int min = e.duration / 60;
        int sec = e.duration % 60;

        std::wstring dateRu;
        if (e.date.length() >= 16) {
            std::wstring year  = e.date.substr(0, 4);
            std::wstring month = e.date.substr(5, 2);
            std::wstring day   = e.date.substr(8, 2);
            std::wstring time  = e.date.substr(11, 5);
            dateRu = day + L"." + month + L"." + year + L" " + time;
        } else {
            dateRu = e.date;
        }

        wchar_t wpmBuf[32];
        swprintf(wpmBuf, 32, L"%d", e.wpm);

        wchar_t errBuf[32];
        swprintf(errBuf, 32, L"%d%%", e.errorRate);

        wchar_t timeBuf[32];
        swprintf(timeBuf, 32, L"%02d:%02d", min, sec);

        // Вставляем строку (Unicode)
        LVITEMW item = {};
        item.mask = LVIF_TEXT;
        item.iItem = rowIndex;
        item.iSubItem = 0;
        item.pszText = (LPWSTR)dateRu.c_str();
        SendMessageW(hList, LVM_INSERTITEMW, 0, (LPARAM)&item);

        // Заполняем остальные колонки (Unicode)
        LVITEMW subItem = {};
        subItem.mask = LVIF_TEXT;
        subItem.iItem = rowIndex;
        subItem.pszText = (LPWSTR)mode;
        subItem.iSubItem = 1;
        SendMessageW(hList, LVM_SETITEMTEXTW, rowIndex, (LPARAM)&subItem);

        subItem.iSubItem = 2;
        subItem.pszText = wpmBuf;
        SendMessageW(hList, LVM_SETITEMTEXTW, rowIndex, (LPARAM)&subItem);

        subItem.iSubItem = 3;
        subItem.pszText = errBuf;
        SendMessageW(hList, LVM_SETITEMTEXTW, rowIndex, (LPARAM)&subItem);

        subItem.iSubItem = 4;
        subItem.pszText = timeBuf;
        SendMessageW(hList, LVM_SETITEMTEXTW, rowIndex, (LPARAM)&subItem);

        rowIndex++;
    }
}

void MainWindow::loadSettingsIntoUI()
{
    Settings& s = Settings::getInstance();

    CheckDlgButton(m_hwnd, ID_CHK_SOUND, s.getSoundEnabled() ? BST_CHECKED : BST_UNCHECKED);
    SendDlgItemMessageW(m_hwnd, ID_TRACK_VOLUME, TBM_SETPOS, TRUE, s.getVolume());
    EnableWindow(GetDlgItem(m_hwnd, ID_TRACK_VOLUME), s.getSoundEnabled());

    if (s.getTheme() == 0) {
        CheckRadioButton(m_hwnd, ID_RADIO_THEME_LIGHT, ID_RADIO_THEME_DARK, ID_RADIO_THEME_LIGHT);
    } else {
        CheckRadioButton(m_hwnd, ID_RADIO_THEME_LIGHT, ID_RADIO_THEME_DARK, ID_RADIO_THEME_DARK);
    }

    wchar_t fontBuf[16];
    swprintf(fontBuf, 16, L"%d", s.getFontSize());
    SetDlgItemTextW(m_hwnd, ID_EDIT_FONT, fontBuf);
}

void MainWindow::onCreate(HWND hwnd)
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int W = clientRect.right - clientRect.left;
    int H = clientRect.bottom - clientRect.top;

    int cx = W / 2;
    int cy = H / 2;

    HFONT hFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HFONT hTitleFont = CreateFontW(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HFONT hHeadingFont = CreateFontW(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    // Times New Roman для истории
    HFONT hHistFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Times New Roman");

    HFONT hHistHeaderFont = CreateFontW(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Times New Roman");

    const int TITLE_W = 800;
    const int TITLE_H = 60;
    const int TITLE_Y = 40;

    // ===== ЭКРАН 1: ВЫБОР ПРОФИЛЯ =====
    HWND hTitle1 = CreateWindowW(L"STATIC", L"KeyboardTrainer",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H, hwnd, NULL, NULL, NULL);
    SendMessageW(hTitle1, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
    m_profileScreenElements.push_back(hTitle1);

    int listW = 400, listH = 180;
    int listX = cx - listW/2;
    int listY = cy - 100;

    HWND hProfileLabel = CreateWindowW(L"STATIC", L"Выберите профиль:",
        WS_VISIBLE | WS_CHILD | SS_LEFT,
        listX, listY - 32, listW, 26,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hProfileLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_profileScreenElements.push_back(hProfileLabel);

    HWND hList = CreateWindowW(L"LISTBOX", L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | LBS_NOTIFY | WS_VSCROLL,
        listX, listY, listW, listH, hwnd, (HMENU)ID_LIST_PROFILES, NULL, NULL);
    SendMessageW(hList, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hList, 12);
    m_profileScreenElements.push_back(hList);

    int btnW = 125, btnH = 44;
    int totalBtnW = btnW * 3 + 13 * 2;
    int btnStartX = cx - totalBtnW/2;
    int btnY = listY + listH + 30;

    HWND hCreate = CreateWindowW(L"BUTTON", L"Создать",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, btnStartX, btnY, btnW, btnH,
        hwnd, (HMENU)ID_BTN_CREATE, NULL, NULL);
    SendMessageW(hCreate, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hCreate, 12);
    m_profileScreenElements.push_back(hCreate);

    HWND hDelete = CreateWindowW(L"BUTTON", L"Удалить",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, btnStartX + btnW + 13, btnY, btnW, btnH,
        hwnd, (HMENU)ID_BTN_DELETE, NULL, NULL);
    SendMessageW(hDelete, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hDelete, 12);
    m_profileScreenElements.push_back(hDelete);

    HWND hLogin = CreateWindowW(L"BUTTON", L"Войти",
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, btnStartX + (btnW + 13) * 2, btnY, btnW, btnH,
        hwnd, (HMENU)ID_BTN_LOGIN, NULL, NULL);
    SendMessageW(hLogin, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hLogin, 12);
    m_profileScreenElements.push_back(hLogin);

    // ===== ЭКРАН 2: СОЗДАНИЕ ПРОФИЛЯ =====
    HWND hBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_BACK, NULL, NULL);
    SendMessageW(hBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hBack, 12);
    m_createScreenElements.push_back(hBack);

    HWND hTitle2 = CreateWindowW(L"STATIC", L"Создание профиля",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hTitle2, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_createScreenElements.push_back(hTitle2);

    int fieldW = 400, fieldH = 44;
    int fieldX = cx - fieldW/2;
    int fieldY = cy - 110;

    HWND hLoginLabel = CreateWindowW(L"STATIC", L"Логин:",
        WS_CHILD | SS_LEFT, fieldX, fieldY - 30, fieldW, 24,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hLoginLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_createScreenElements.push_back(hLoginLabel);

    HWND hEditLogin = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, fieldX, fieldY, fieldW, fieldH,
        hwnd, (HMENU)ID_EDIT_LOGIN, NULL, NULL);
    SendMessageW(hEditLogin, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEditLogin, EM_SETCUEBANNER, TRUE, (LPARAM)L"Введите логин");
    m_createScreenElements.push_back(hEditLogin);

    HWND hPassLabel = CreateWindowW(L"STATIC", L"Пароль:",
        WS_CHILD | SS_LEFT, fieldX, fieldY + 80, fieldW, 24,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hPassLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_createScreenElements.push_back(hPassLabel);

    HWND hEditPass = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
        fieldX, fieldY + 110, fieldW, fieldH,
        hwnd, (HMENU)ID_EDIT_PASSWORD, NULL, NULL);
    SendMessageW(hEditPass, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hEditPass, EM_SETCUEBANNER, TRUE, (LPARAM)L"Введите пароль");
    m_createScreenElements.push_back(hEditPass);

    HWND hSave = CreateWindowW(L"BUTTON", L"Сохранить",
        WS_CHILD | BS_OWNERDRAW, fieldX, fieldY + 200, fieldW, 50,
        hwnd, (HMENU)ID_BTN_SAVE, NULL, NULL);
    SendMessageW(hSave, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hSave, 12);
    m_createScreenElements.push_back(hSave);

    // ===== ЭКРАН 3: АВТОРИЗАЦИЯ =====
    HWND hAuthBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_AUTH_BACK, NULL, NULL);
    SendMessageW(hAuthBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hAuthBack, 12);
    m_authScreenElements.push_back(hAuthBack);

    HWND hAuthTitle = CreateWindowW(L"STATIC", L"Авторизация",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hAuthTitle, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_authScreenElements.push_back(hAuthTitle);

    int authFieldW = 400;
    int authFieldX = cx - authFieldW/2;
    int authFieldY = cy - 120;

    HWND hAuthProfileLabel = CreateWindowW(L"STATIC", L"Профиль:",
        WS_CHILD | SS_LEFT, authFieldX, authFieldY - 30, authFieldW, 24,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hAuthProfileLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_authScreenElements.push_back(hAuthProfileLabel);

    HWND hAuthProfileName = CreateWindowW(L"STATIC", L"—",
        WS_CHILD | SS_LEFT, authFieldX, authFieldY, authFieldW, 30,
        hwnd, (HMENU)ID_LABEL_AUTH_PROFILE, NULL, NULL);
    SendMessageW(hAuthProfileName, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_authScreenElements.push_back(hAuthProfileName);

    HWND hAuthPassLabel = CreateWindowW(L"STATIC", L"Пароль:",
        WS_CHILD | SS_LEFT, authFieldX, authFieldY + 60, authFieldW, 24,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hAuthPassLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_authScreenElements.push_back(hAuthPassLabel);

    HWND hAuthPass = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_BORDER | ES_PASSWORD | ES_AUTOHSCROLL,
        authFieldX, authFieldY + 90, authFieldW, 44,
        hwnd, (HMENU)ID_EDIT_AUTH_PASSWORD, NULL, NULL);
    SendMessageW(hAuthPass, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hAuthPass, EM_SETCUEBANNER, TRUE, (LPARAM)L"Введите пароль");
    m_authScreenElements.push_back(hAuthPass);

    HWND hAuthBtn = CreateWindowW(L"BUTTON", L"Войти",
        WS_CHILD | BS_OWNERDRAW, authFieldX, authFieldY + 170, authFieldW, 50,
        hwnd, (HMENU)ID_BTN_AUTH_LOGIN, NULL, NULL);
    SendMessageW(hAuthBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hAuthBtn, 12);
    m_authScreenElements.push_back(hAuthBtn);

    // ===== ЭКРАН 4: ГЛАВНОЕ МЕНЮ =====
    HWND hMenuTitle = CreateWindowW(L"STATIC", L"Главное меню",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hMenuTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
    m_menuScreenElements.push_back(hMenuTitle);

    int mBtnW = 380, mBtnH = 100, mGap = 60;
    int mTotalW = mBtnW * 2 + mGap;
    int mTotalH = mBtnH * 2 + mGap;
    int mStartX = cx - mTotalW/2;
    int mStartY = cy - mTotalH/2 + 40;

    HWND hMenuStart = CreateWindowW(L"BUTTON", L"Начать упражнение",
        WS_CHILD | BS_OWNERDRAW, mStartX, mStartY, mBtnW, mBtnH,
        hwnd, (HMENU)ID_BTN_MENU_START, NULL, NULL);
    SendMessageW(hMenuStart, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hMenuStart, 16);
    m_menuScreenElements.push_back(hMenuStart);

    HWND hMenuHeatmap = CreateWindowW(L"BUTTON", L"Тепловая карта",
        WS_CHILD | BS_OWNERDRAW, mStartX + mBtnW + mGap, mStartY, mBtnW, mBtnH,
        hwnd, (HMENU)ID_BTN_MENU_HEATMAP, NULL, NULL);
    SendMessageW(hMenuHeatmap, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hMenuHeatmap, 16);
    m_menuScreenElements.push_back(hMenuHeatmap);

    HWND hMenuHistory = CreateWindowW(L"BUTTON", L"История",
        WS_CHILD | BS_OWNERDRAW, mStartX, mStartY + mBtnH + mGap, mBtnW, mBtnH,
        hwnd, (HMENU)ID_BTN_MENU_HISTORY, NULL, NULL);
    SendMessageW(hMenuHistory, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hMenuHistory, 16);
    m_menuScreenElements.push_back(hMenuHistory);

    HWND hMenuSettings = CreateWindowW(L"BUTTON", L"Настройки",
        WS_CHILD | BS_OWNERDRAW, mStartX + mBtnW + mGap, mStartY + mBtnH + mGap, mBtnW, mBtnH,
        hwnd, (HMENU)ID_BTN_MENU_SETTINGS, NULL, NULL);
    SendMessageW(hMenuSettings, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hMenuSettings, 16);
    m_menuScreenElements.push_back(hMenuSettings);

    HWND hMenuExit = CreateWindowW(L"BUTTON", L"Выход",
        WS_CHILD | BS_OWNERDRAW, W - 130 - 30, H - 40 - 30, 130, 44,
        hwnd, (HMENU)ID_BTN_MENU_EXIT, NULL, NULL);
    SendMessageW(hMenuExit, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hMenuExit, 12);
    m_menuScreenElements.push_back(hMenuExit);

    // ===== ЭКРАН 5: ВЫБОР РЕЖИМА =====
    HWND hModeBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_MODE_BACK, NULL, NULL);
    SendMessageW(hModeBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hModeBack, 12);
    m_modeScreenElements.push_back(hModeBack);

    HWND hModeTitle = CreateWindowW(L"STATIC", L"Выбор режима",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hModeTitle, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_modeScreenElements.push_back(hModeTitle);

    int modeCX = cx - 200;
    int modeY = cy - 150;

    HWND hModeLabel = CreateWindowW(L"STATIC", L"Режим",
        WS_CHILD | SS_LEFT, modeCX, modeY, 400, 30, hwnd, NULL, NULL, NULL);
    SendMessageW(hModeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hModeLabel);

    HWND hRL = CreateWindowW(L"BUTTON", L"Буквы",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP,
        modeCX, modeY + 40, 400, 30, hwnd, (HMENU)ID_RADIO_LETTERS, NULL, NULL);
    SendMessageW(hRL, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hRL);

    HWND hRS = CreateWindowW(L"BUTTON", L"Слоги",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP,
        modeCX, modeY + 75, 400, 30, hwnd, (HMENU)ID_RADIO_SYLLABLES, NULL, NULL);
    SendMessageW(hRS, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hRS);

    HWND hRW = CreateWindowW(L"BUTTON", L"Слова",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP,
        modeCX, modeY + 110, 400, 30, hwnd, (HMENU)ID_RADIO_WORDS, NULL, NULL);
    SendMessageW(hRW, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hRW);

    HWND hRT = CreateWindowW(L"BUTTON", L"Текст",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_TABSTOP,
        modeCX, modeY + 145, 400, 30, hwnd, (HMENU)ID_RADIO_TEXT, NULL, NULL);
    SendMessageW(hRT, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hRT);
    CheckRadioButton(hwnd, ID_RADIO_LETTERS, ID_RADIO_TEXT, ID_RADIO_TEXT);

    HWND hDL = CreateWindowW(L"STATIC", L"Длительность",
        WS_CHILD | SS_LEFT, modeCX, modeY + 200, 200, 30, hwnd, NULL, NULL, NULL);
    SendMessageW(hDL, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hDL);

    HWND hCD = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        modeCX + 200, modeY + 200, 200, 200, hwnd, (HMENU)ID_COMBO_DURATION, NULL, NULL);
    SendMessageW(hCD, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hCD, CB_ADDSTRING, 0, (LPARAM)L"30 сек");
    SendMessageW(hCD, CB_ADDSTRING, 0, (LPARAM)L"60 сек");
    SendMessageW(hCD, CB_ADDSTRING, 0, (LPARAM)L"120 сек");
    SendMessageW(hCD, CB_SETCURSEL, 1, 0);
    m_modeScreenElements.push_back(hCD);

    HWND hRLbl = CreateWindowW(L"STATIC", L"Ряд клавиатуры",
        WS_CHILD | SS_LEFT, modeCX, modeY + 245, 200, 30, hwnd, NULL, NULL, NULL);
    SendMessageW(hRLbl, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_modeScreenElements.push_back(hRLbl);

    HWND hCR = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        modeCX + 200, modeY + 245, 200, 200, hwnd, (HMENU)ID_COMBO_ROW, NULL, NULL);
    SendMessageW(hCR, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(hCR, CB_ADDSTRING, 0, (LPARAM)L"Все");
    SendMessageW(hCR, CB_ADDSTRING, 0, (LPARAM)L"Верхний ряд");
    SendMessageW(hCR, CB_ADDSTRING, 0, (LPARAM)L"Средний ряд");
    SendMessageW(hCR, CB_ADDSTRING, 0, (LPARAM)L"Нижний ряд");
    SendMessageW(hCR, CB_SETCURSEL, 0, 0);
    m_modeScreenElements.push_back(hCR);

    HWND hMS = CreateWindowW(L"BUTTON", L"Начать",
        WS_CHILD | BS_OWNERDRAW, modeCX, modeY + 300, 400, 50,
        hwnd, (HMENU)ID_BTN_MODE_START, NULL, NULL);
    SendMessageW(hMS, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hMS, 12);
    m_modeScreenElements.push_back(hMS);

    // ===== ЭКРАН 6: УПРАЖНЕНИЕ =====
    HWND hExBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_EXERCISE_BACK, NULL, NULL);
    SendMessageW(hExBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hExBack, 12);
    m_exerciseScreenElements.push_back(hExBack);

    HWND hExTitle = CreateWindowW(L"STATIC", L"Выполнение упражнения",
        WS_CHILD | SS_LEFT, 150, 38, 400, 40, hwnd, NULL, NULL, NULL);
    SendMessageW(hExTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_exerciseScreenElements.push_back(hExTitle);

    HWND hWL = CreateWindowW(L"STATIC", L"WPM (скорость набора)",
        WS_CHILD | SS_LEFT, 60, 90, 300, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hWL, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_exerciseScreenElements.push_back(hWL);

    HWND hEL = CreateWindowW(L"STATIC", L"Ошибки (точность ввода)",
        WS_CHILD | SS_LEFT, 400, 90, 300, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hEL, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_exerciseScreenElements.push_back(hEL);

    HWND hTL = CreateWindowW(L"STATIC", L"Время",
        WS_CHILD | SS_LEFT, 740, 90, 200, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hTL, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_exerciseScreenElements.push_back(hTL);

    HWND hFin = CreateWindowW(L"BUTTON", L"Завершить",
        WS_CHILD | BS_OWNERDRAW, W - 160 - 30, 30, 160, 40,
        hwnd, (HMENU)ID_BTN_EXERCISE_FINISH, NULL, NULL);
    SendMessageW(hFin, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hFin, 12);
    m_exerciseScreenElements.push_back(hFin);

    // ===== ЭКРАН 7: РЕЗУЛЬТАТЫ =====
    HWND hResTitle = CreateWindowW(L"STATIC", L"Упражнение завершено",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hResTitle, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_resultsScreenElements.push_back(hResTitle);

    int resCX = cx - 200;
    int resY = cy - 120;

    HWND hResWpmLabel = CreateWindowW(L"STATIC", L"WPM",
        WS_CHILD | SS_LEFT, resCX, resY, 200, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hResWpmLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResWpmLabel);

    HWND hResWpm = CreateWindowW(L"STATIC", L"0",
        WS_CHILD | SS_RIGHT, resCX + 200, resY - 5, 200, 30,
        hwnd, (HMENU)ID_LABEL_RESULT_WPM, NULL, NULL);
    SendMessageW(hResWpm, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResWpm);

    HWND hResErrLabel = CreateWindowW(L"STATIC", L"Ошибки",
        WS_CHILD | SS_LEFT, resCX, resY + 40, 200, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hResErrLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResErrLabel);

    HWND hResErr = CreateWindowW(L"STATIC", L"0%",
        WS_CHILD | SS_RIGHT, resCX + 200, resY + 35, 200, 30,
        hwnd, (HMENU)ID_LABEL_RESULT_ERRORS, NULL, NULL);
    SendMessageW(hResErr, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResErr);

    HWND hResTimeLabel = CreateWindowW(L"STATIC", L"Время",
        WS_CHILD | SS_LEFT, resCX, resY + 80, 200, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hResTimeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResTimeLabel);

    HWND hResTime = CreateWindowW(L"STATIC", L"00:00",
        WS_CHILD | SS_RIGHT, resCX + 200, resY + 75, 200, 30,
        hwnd, (HMENU)ID_LABEL_RESULT_TIME, NULL, NULL);
    SendMessageW(hResTime, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResTime);

    HWND hResCharsLabel = CreateWindowW(L"STATIC", L"Символов",
        WS_CHILD | SS_LEFT, resCX, resY + 120, 200, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hResCharsLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResCharsLabel);

    HWND hResChars = CreateWindowW(L"STATIC", L"0",
        WS_CHILD | SS_RIGHT, resCX + 200, resY + 115, 200, 30,
        hwnd, (HMENU)ID_LABEL_RESULT_CHARS, NULL, NULL);
    SendMessageW(hResChars, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_resultsScreenElements.push_back(hResChars);

    HWND hResHeatmap = CreateWindowW(L"BUTTON", L"Тепловая карта",
        WS_CHILD | BS_OWNERDRAW, resCX, resY + 190, 190, 50,
        hwnd, (HMENU)ID_BTN_RESULT_HEATMAP, NULL, NULL);
    SendMessageW(hResHeatmap, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hResHeatmap, 12);
    m_resultsScreenElements.push_back(hResHeatmap);

    HWND hResMenu = CreateWindowW(L"BUTTON", L"В меню",
        WS_CHILD | BS_OWNERDRAW, resCX + 210, resY + 190, 190, 50,
        hwnd, (HMENU)ID_BTN_RESULT_MENU, NULL, NULL);
    SendMessageW(hResMenu, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hResMenu, 12);
    m_resultsScreenElements.push_back(hResMenu);

    // ===== ЭКРАН 8: ИСТОРИЯ =====
    HWND hHistBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_HISTORY_BACK, NULL, NULL);
    SendMessageW(hHistBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hHistBack, 12);
    m_historyScreenElements.push_back(hHistBack);

    HWND hHistTitle = CreateWindowW(L"STATIC", L"История",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hHistTitle, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_historyScreenElements.push_back(hHistTitle);

    int histListW = W - 120;
    int histListX = 60;
    int histListY = 160;
    int histListH = H - 260;

        // ИСПРАВЛЕНО: используем ListView вместо ListBox
    HWND hHistList = CreateWindowW(WC_LISTVIEWW, L"",
        WS_CHILD | WS_BORDER | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
        histListX, histListY, histListW, histListH,
        hwnd, (HMENU)ID_LIST_HISTORY, NULL, NULL);

    ListView_SetExtendedListViewStyle(hHistList,
        LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    SendMessageW(hHistList, WM_SETFONT, (WPARAM)hHistFont, TRUE);

    // Добавляем колонки (явная Unicode-версия)
    LVCOLUMNW col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

    col.pszText = (LPWSTR)L"Дата";
    col.cx = 200;
    col.iSubItem = 0;
    SendMessageW(hHistList, LVM_INSERTCOLUMNW, 0, (LPARAM)&col);

    col.pszText = (LPWSTR)L"Режим";
    col.cx = 120;
    col.iSubItem = 1;
    SendMessageW(hHistList, LVM_INSERTCOLUMNW, 1, (LPARAM)&col);

    col.pszText = (LPWSTR)L"WPM";
    col.cx = 100;
    col.iSubItem = 2;
    SendMessageW(hHistList, LVM_INSERTCOLUMNW, 2, (LPARAM)&col);

    col.pszText = (LPWSTR)L"Ошибки";
    col.cx = 120;
    col.iSubItem = 3;
    SendMessageW(hHistList, LVM_INSERTCOLUMNW, 3, (LPARAM)&col);

    col.pszText = (LPWSTR)L"Время";
    col.cx = 120;
    col.iSubItem = 4;
    SendMessageW(hHistList, LVM_INSERTCOLUMNW, 4, (LPARAM)&col);

    m_historyScreenElements.push_back(hHistList);

    HWND hHistClear = CreateWindowW(L"BUTTON", L"Очистить",
        WS_CHILD | BS_OWNERDRAW, W - 154 - 30, H - 48 - 30, 154, 44,
        hwnd, (HMENU)ID_BTN_HISTORY_CLEAR, NULL, NULL);
    SendMessageW(hHistClear, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hHistClear, 12);
    m_historyScreenElements.push_back(hHistClear);

    // ===== ЭКРАН 9: НАСТРОЙКИ =====
    HWND hSetBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_SETTINGS_BACK, NULL, NULL);
    SendMessageW(hSetBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hSetBack, 12);
    m_settingsScreenElements.push_back(hSetBack);

    HWND hSetTitle = CreateWindowW(L"STATIC", L"Настройки",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hSetTitle, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_settingsScreenElements.push_back(hSetTitle);

    int setCX = cx - 200;
    int setY = cy - 180;

    HWND hChkSound = CreateWindowW(L"BUTTON", L"Звук при ошибке",
        WS_CHILD | BS_AUTOCHECKBOX, setCX, setY, 400, 30,
        hwnd, (HMENU)ID_CHK_SOUND, NULL, NULL);
    SendMessageW(hChkSound, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hChkSound);

    HWND hVolLabel = CreateWindowW(L"STATIC", L"Громкость",
        WS_CHILD | SS_LEFT, setCX, setY + 50, 150, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hVolLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hVolLabel);

    HWND hVolume = CreateWindowW(TRACKBAR_CLASSW, L"",
        WS_CHILD | TBS_HORZ | TBS_NOTICKS | TBS_BOTH,
        setCX + 150, setY + 50, 250, 30, hwnd, (HMENU)ID_TRACK_VOLUME, NULL, NULL);
    SendMessageW(hVolume, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessageW(hVolume, TBM_SETPOS, TRUE, 50);
    m_settingsScreenElements.push_back(hVolume);

    HWND hThemeLabel = CreateWindowW(L"STATIC", L"Тема оформления",
        WS_CHILD | SS_LEFT, setCX, setY + 110, 400, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hThemeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hThemeLabel);

    HWND hThemeLight = CreateWindowW(L"BUTTON", L"Светлая",
        WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP, setCX, setY + 140, 200, 30,
        hwnd, (HMENU)ID_RADIO_THEME_LIGHT, NULL, NULL);
    SendMessageW(hThemeLight, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hThemeLight);

    HWND hThemeDark = CreateWindowW(L"BUTTON", L"Тёмная",
        WS_CHILD | BS_AUTORADIOBUTTON, setCX + 220, setY + 140, 200, 30,
        hwnd, (HMENU)ID_RADIO_THEME_DARK, NULL, NULL);
    SendMessageW(hThemeDark, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hThemeDark);

    HWND hFontLabel = CreateWindowW(L"STATIC", L"Размер шрифта",
        WS_CHILD | SS_LEFT, setCX, setY + 190, 200, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hFontLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hFontLabel);

    HWND hFontEdit = CreateWindowW(L"EDIT", L"20",
        WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL,
        setCX + 200, setY + 190, 100, 30, hwnd, (HMENU)ID_EDIT_FONT, NULL, NULL);
    SendMessageW(hFontEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hFontEdit);

    HWND hFontHintLabel = CreateWindowW(L"STATIC", L"(12-48)",
        WS_CHILD | SS_LEFT, setCX + 310, setY + 195, 100, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hFontHintLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_settingsScreenElements.push_back(hFontHintLabel);

    HWND hSetSave = CreateWindowW(L"BUTTON", L"Сохранить",
        WS_CHILD | BS_OWNERDRAW, setCX, setY + 260, 400, 50,
        hwnd, (HMENU)ID_BTN_SETTINGS_SAVE, NULL, NULL);
    SendMessageW(hSetSave, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hSetSave, 12);
    m_settingsScreenElements.push_back(hSetSave);

    // ===== ЭКРАН 10: ТЕПЛОВАЯ КАРТА =====
    HWND hHeatBack = CreateWindowW(L"BUTTON", L"← Назад",
        WS_CHILD | BS_OWNERDRAW, 30, 30, 110, 40,
        hwnd, (HMENU)ID_BTN_HEATMAP_BACK, NULL, NULL);
    SendMessageW(hHeatBack, WM_SETFONT, (WPARAM)hFont, TRUE);
    roundWindowCorners(hHeatBack, 12);
    m_heatmapScreenElements.push_back(hHeatBack);

    HWND hHeatTitle = CreateWindowW(L"STATIC", L"Тепловая карта ошибок",
        WS_CHILD | SS_CENTER, cx - TITLE_W/2, TITLE_Y, TITLE_W, TITLE_H,
        hwnd, NULL, NULL, NULL);
    SendMessageW(hHeatTitle, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);
    m_heatmapScreenElements.push_back(hHeatTitle);

    int heatW = W - 120;
    int heatH = H - 260;
    int heatX = 60;
    int heatY = 160;

    HWND hHeatArea = CreateWindowW(L"STATIC", L"",
        WS_CHILD | WS_BORDER,
        heatX, heatY, heatW, heatH,
        hwnd, (HMENU)ID_HEATMAP_AREA, NULL, NULL);
    roundWindowCorners(hHeatArea, 12);
    m_heatmapScreenElements.push_back(hHeatArea);

    HWND hLegend = CreateWindowW(L"STATIC",
        L"Зелёный — мало ошибок   |   Жёлтый — средне   |   Красный — много ошибок",
        WS_CHILD | SS_CENTER, cx - 500, H - 80, 1000, 25, hwnd, NULL, NULL, NULL);
    SendMessageW(hLegend, WM_SETFONT, (WPARAM)hFont, TRUE);
    m_heatmapScreenElements.push_back(hLegend);

    // Освобождаем шрифты
    DeleteObject(hFont);
    DeleteObject(hTitleFont);
    DeleteObject(hHeadingFont);
    DeleteObject(hHistFont);
    DeleteObject(hHistHeaderFont);

    // Загружаем профили из БД при старте
    auto profiles = DatabaseManager::getInstance().getAllProfiles();
    HWND hListBox = GetDlgItem(hwnd, ID_LIST_PROFILES);
    for (const auto& p : profiles) {
        SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)p.name.c_str());
    }

    showScreen(SCREEN_PROFILES);
}

void MainWindow::startExercise(HWND hwnd)
{
    m_exercise.reset();

    ExerciseFactory::Mode mode = (ExerciseFactory::Mode)m_selectedMode;
    m_exercise.setText(ExerciseFactory::createExercise(
        mode, m_selectedRow, m_selectedDuration));

    m_elapsedSeconds = 0;
    m_remainingSeconds = m_selectedDuration;
    m_exerciseActive = true;

    m_lastExerciseId = -1;
    if (m_selectedProfileId >= 0) {
        m_lastExerciseId = DatabaseManager::getInstance().saveExercise(
            m_selectedProfileId, m_selectedMode, 0, 0, 0);
    }

    HWND hOldText   = GetDlgItem(hwnd, ID_LABEL_TEXT);
    HWND hOldTyping = GetDlgItem(hwnd, ID_EDIT_TYPING);
    if (hOldText)   DestroyWindow(hOldText);
    if (hOldTyping) DestroyWindow(hOldTyping);

    HWND hOldWpm  = GetDlgItem(hwnd, ID_LABEL_WPM);
    HWND hOldErr  = GetDlgItem(hwnd, ID_LABEL_ERRORS);
    HWND hOldTime = GetDlgItem(hwnd, ID_LABEL_TIME);
    if (hOldWpm)  DestroyWindow(hOldWpm);
    if (hOldErr)  DestroyWindow(hOldErr);
    if (hOldTime) DestroyWindow(hOldTime);

    int fontSize = Settings::getInstance().getFontSize();

    HFONT hCustomFont = CreateFontW(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    HFONT hHeadingFont = CreateFontW(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int W = clientRect.right - clientRect.left;
    int H = clientRect.bottom - clientRect.top;

    int cw = W - 120;
    int cx = 60;
    int cy = H / 2 - 100;

    int statY = cy + 30;
    int textY = cy + 100;
    int textH = 180;
    int typingY = textY + textH + 20;
    int typingH = 200;

    HWND hText = CreateWindowW(L"STATIC", m_exercise.getText().c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
        cx, textY, cw, textH,
        hwnd, (HMENU)ID_LABEL_TEXT, NULL, NULL);
    SendMessageW(hText, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    roundWindowCorners(hText, 12);

    HWND hTyping = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
        cx, typingY, cw, typingH,
        hwnd, (HMENU)ID_EDIT_TYPING, NULL, NULL);
    SendMessageW(hTyping, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
    SendMessageW(hTyping, EM_SETCUEBANNER, TRUE, (LPARAM)L"> вводимый текст здесь");

    HWND hWpm = CreateWindowW(L"STATIC", L"0",
        WS_CHILD | WS_VISIBLE | SS_LEFT, cx, statY, 300, 40,
        hwnd, (HMENU)ID_LABEL_WPM, NULL, NULL);
    SendMessageW(hWpm, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);

    HWND hErr = CreateWindowW(L"STATIC", L"0%",
        WS_CHILD | WS_VISIBLE | SS_LEFT, cx + 340, statY, 300, 40,
        hwnd, (HMENU)ID_LABEL_ERRORS, NULL, NULL);
    SendMessageW(hErr, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);

    HWND hTime = CreateWindowW(L"STATIC", L"00:00",
        WS_CHILD | WS_VISIBLE | SS_LEFT, cx + 680, statY, 200, 40,
        hwnd, (HMENU)ID_LABEL_TIME, NULL, NULL);
    SendMessageW(hTime, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);

    DeleteObject(hCustomFont);
    DeleteObject(hHeadingFont);

    SetTimer(hwnd, TIMER_EXERCISE_ID, 1000, NULL);
    SetFocus(GetDlgItem(hwnd, ID_EDIT_TYPING));
}

void MainWindow::updateStats(HWND hwnd)
{
    int wpm = m_exercise.calculateWPM(m_elapsedSeconds);
    int errorRate = m_exercise.calculateErrorRate();

    HFONT hHeadingFont = CreateFontW(36, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

    wchar_t buf[128];

    int cx = 60;
    int statY = 120;

    HWND hWpmOld = GetDlgItem(hwnd, ID_LABEL_WPM);
    if (hWpmOld) DestroyWindow(hWpmOld);
    swprintf(buf, 128, L"%d", wpm);
    HWND hWpmNew = CreateWindowW(L"STATIC", buf,
        WS_CHILD | WS_VISIBLE | SS_LEFT, cx, statY, 300, 40,
        hwnd, (HMENU)ID_LABEL_WPM, NULL, NULL);
    SendMessageW(hWpmNew, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);

    HWND hErrOld = GetDlgItem(hwnd, ID_LABEL_ERRORS);
    if (hErrOld) DestroyWindow(hErrOld);
    swprintf(buf, 128, L"%d%%", errorRate);
    HWND hErrNew = CreateWindowW(L"STATIC", buf,
        WS_CHILD | WS_VISIBLE | SS_LEFT, cx + 340, statY, 300, 40,
        hwnd, (HMENU)ID_LABEL_ERRORS, NULL, NULL);
    SendMessageW(hErrNew, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);

    HWND hTimeOld = GetDlgItem(hwnd, ID_LABEL_TIME);
    if (hTimeOld) DestroyWindow(hTimeOld);
    int min = m_elapsedSeconds / 60;
    int sec = m_elapsedSeconds % 60;
    swprintf(buf, 128, L"%02d:%02d", min, sec);
    HWND hTimeNew = CreateWindowW(L"STATIC", buf,
        WS_CHILD | WS_VISIBLE | SS_LEFT, cx + 680, statY, 200, 40,
        hwnd, (HMENU)ID_LABEL_TIME, NULL, NULL);
    SendMessageW(hTimeNew, WM_SETFONT, (WPARAM)hHeadingFont, TRUE);

    DeleteObject(hHeadingFont);
}

void MainWindow::finishExercise(HWND hwnd)
{
    if (!m_exerciseActive) return;
    m_exerciseActive = false;
    KillTimer(hwnd, TIMER_EXERCISE_ID);

    int wpm = m_exercise.calculateWPM(m_elapsedSeconds);
    int errorRate = m_exercise.calculateErrorRate();
    int totalChars = m_exercise.getTotalChars();
    int elapsed = m_elapsedSeconds;

    m_lastWpm = wpm;
    m_lastErrorRate = errorRate;
    m_lastElapsedSeconds = elapsed;
    m_lastTotalChars = totalChars;

    if (m_lastExerciseId >= 0) {
        DatabaseManager::getInstance().updateExercise(
            m_lastExerciseId, wpm, errorRate, elapsed);
    }

    HWND hWpm  = GetDlgItem(hwnd, ID_LABEL_WPM);
    HWND hErr  = GetDlgItem(hwnd, ID_LABEL_ERRORS);
    HWND hTime = GetDlgItem(hwnd, ID_LABEL_TIME);
    if (hWpm)  { ShowWindow(hWpm,  SW_HIDE); DestroyWindow(hWpm);  }
    if (hErr)  { ShowWindow(hErr,  SW_HIDE); DestroyWindow(hErr);  }
    if (hTime) { ShowWindow(hTime, SW_HIDE); DestroyWindow(hTime); }

    wchar_t buf[128];

    swprintf(buf, 128, L"%d", wpm);
    SetDlgItemTextW(hwnd, ID_LABEL_RESULT_WPM, buf);

    swprintf(buf, 128, L"%d%%", errorRate);
    SetDlgItemTextW(hwnd, ID_LABEL_RESULT_ERRORS, buf);

    int min = elapsed / 60;
    int sec = elapsed % 60;
    swprintf(buf, 128, L"%02d:%02d", min, sec);
    SetDlgItemTextW(hwnd, ID_LABEL_RESULT_TIME, buf);

    swprintf(buf, 128, L"%d", totalChars);
    SetDlgItemTextW(hwnd, ID_LABEL_RESULT_CHARS, buf);

    m_exercise.reset();
    m_elapsedSeconds = 0;
    m_remainingSeconds = 0;

    showScreen(SCREEN_RESULTS);
}

void MainWindow::onCommand(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    if (HIWORD(wParam) == CBN_SELCHANGE && LOWORD(wParam) == ID_COMBO_ROW) {
        int row = (int)SendDlgItemMessageW(hwnd, ID_COMBO_ROW, CB_GETCURSEL, 0, 0);
        BOOL enableWordsText = (row == 0);

        EnableWindow(GetDlgItem(hwnd, ID_RADIO_WORDS), enableWordsText);
        EnableWindow(GetDlgItem(hwnd, ID_RADIO_TEXT),  enableWordsText);

        if (!enableWordsText) {
            if (IsDlgButtonChecked(hwnd, ID_RADIO_WORDS) == BST_CHECKED ||
                IsDlgButtonChecked(hwnd, ID_RADIO_TEXT) == BST_CHECKED) {
                CheckRadioButton(hwnd, ID_RADIO_LETTERS, ID_RADIO_TEXT, ID_RADIO_LETTERS);
            }
        }
    }

    if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == ID_EDIT_TYPING) {
        if (m_exerciseActive) {
            wchar_t typed[2048] = {};
            GetDlgItemTextW(hwnd, ID_EDIT_TYPING, typed, 2048);

            int prevErrors = m_exercise.getErrorChars();
            m_exercise.updateFromTyped(typed);

            if (m_exercise.getErrorChars() > prevErrors) {
                int len = (int)wcslen(typed);
                const std::wstring& text = m_exercise.getText();

                if (len > 0 && len <= (int)text.length()) {
                    wchar_t expected = text[len - 1];
                    wchar_t actual   = typed[len - 1];

                    if (normalizeChar(expected) != normalizeChar(actual) && m_lastExerciseId >= 0) {
                        DatabaseManager::getInstance().saveKeyError(
                            m_lastExerciseId, expected);
                    }
                }

                if (Settings::getInstance().getSoundEnabled()) {
                    int vol = Settings::getInstance().getVolume();
                    DWORD volValue = (DWORD)((vol * 0xFFFF) / 100);
                    DWORD volBoth = (volValue << 16) | volValue;
                    waveOutSetVolume(NULL, volBoth);

                    PlaySoundW(L"error.wav", NULL,
                               SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
                }
            }

            const std::wstring& text = m_exercise.getText();
            int typedLen = (int)wcslen(typed);
            int textLen  = (int)text.length();

            if (textLen > 0 && typedLen >= textLen && m_exercise.getErrorChars() == 0) {
                PostMessageW(hwnd, WM_COMMAND,
                             MAKEWPARAM(ID_BTN_EXERCISE_FINISH, BN_CLICKED), 0);
            }
        }
    }

    if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_CHK_SOUND) {
        BOOL enabled = (IsDlgButtonChecked(hwnd, ID_CHK_SOUND) == BST_CHECKED);
        EnableWindow(GetDlgItem(hwnd, ID_TRACK_VOLUME), enabled);
    }

    switch (LOWORD(wParam))
    {
    case ID_BTN_CREATE:      showScreen(SCREEN_CREATE); break;
    case ID_BTN_BACK:        showScreen(SCREEN_PROFILES); break;
    case ID_BTN_AUTH_BACK:   showScreen(SCREEN_PROFILES); break;

    case ID_BTN_SAVE:
    {
        wchar_t login[100] = {}, password[100] = {};
        GetDlgItemTextW(hwnd, ID_EDIT_LOGIN, login, 100);
        GetDlgItemTextW(hwnd, ID_EDIT_PASSWORD, password, 100);

        if (wcslen(login) == 0 || wcslen(password) == 0) {
            MessageBoxW(hwnd, L"Заполните все поля", L"Ошибка", MB_OK | MB_ICONWARNING);
        } else {
            DatabaseManager::getInstance().addProfile(login, password);
            SendDlgItemMessageW(hwnd, ID_LIST_PROFILES, LB_ADDSTRING, 0, (LPARAM)login);
            SetDlgItemTextW(hwnd, ID_EDIT_LOGIN, L"");
            SetDlgItemTextW(hwnd, ID_EDIT_PASSWORD, L"");
            MessageBoxW(hwnd, L"Профиль создан", L"Успех", MB_OK | MB_ICONINFORMATION);
            showScreen(SCREEN_PROFILES);
        }
        break;
    }

    case ID_BTN_DELETE:
    {
        int sel = (int)SendDlgItemMessageW(hwnd, ID_LIST_PROFILES, LB_GETCURSEL, 0, 0);
        if (sel != LB_ERR) {
            auto profiles = DatabaseManager::getInstance().getAllProfiles();
            if (sel < (int)profiles.size()) {
                DatabaseManager::getInstance().deleteProfile(profiles[sel].id);
            }
            SendDlgItemMessageW(hwnd, ID_LIST_PROFILES, LB_DELETESTRING, sel, 0);
        } else {
            MessageBoxW(hwnd, L"Выберите профиль", L"Ошибка", MB_OK | MB_ICONWARNING);
        }
        break;
    }

    case ID_BTN_LOGIN:
    {
        int sel = (int)SendDlgItemMessageW(hwnd, ID_LIST_PROFILES, LB_GETCURSEL, 0, 0);
        if (sel == LB_ERR) {
            MessageBoxW(hwnd, L"Выберите профиль", L"Ошибка", MB_OK | MB_ICONWARNING);
        } else {
            auto profiles = DatabaseManager::getInstance().getAllProfiles();
            if (sel < (int)profiles.size()) {
                m_selectedProfileId = profiles[sel].id;
                SetDlgItemTextW(hwnd, ID_LABEL_AUTH_PROFILE,
                                profiles[sel].name.c_str());
            }
            SetDlgItemTextW(hwnd, ID_EDIT_AUTH_PASSWORD, L"");
            showScreen(SCREEN_AUTH);
        }
        break;
    }

    case ID_BTN_AUTH_LOGIN:
    {
        wchar_t pass[100] = {};
        GetDlgItemTextW(hwnd, ID_EDIT_AUTH_PASSWORD, pass, 100);

        if (m_selectedProfileId >= 0) {
            if (DatabaseManager::getInstance().checkPassword(m_selectedProfileId, pass)) {
                SetDlgItemTextW(hwnd, ID_EDIT_AUTH_PASSWORD, L"");
                showScreen(SCREEN_MENU);
            } else {
                MessageBoxW(hwnd, L"Неверный пароль", L"Ошибка", MB_OK | MB_ICONERROR);
            }
        }
        break;
    }

    case ID_BTN_MENU_START:
        showScreen(SCREEN_MODE);
        EnableWindow(GetDlgItem(hwnd, ID_RADIO_WORDS), TRUE);
        EnableWindow(GetDlgItem(hwnd, ID_RADIO_TEXT),  TRUE);
        SendDlgItemMessageW(hwnd, ID_COMBO_ROW, CB_SETCURSEL, 0, 0);
        break;

    case ID_BTN_MENU_HISTORY:
        showScreen(SCREEN_HISTORY);
        loadHistory();
        break;

    case ID_BTN_MENU_HEATMAP:
        showScreen(SCREEN_HEATMAP);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        break;

    case ID_BTN_MENU_SETTINGS:
        showScreen(SCREEN_SETTINGS);
        loadSettingsIntoUI();
        break;

    case ID_BTN_MENU_EXIT:
        showScreen(SCREEN_PROFILES);
        break;

    case ID_BTN_MODE_BACK:
        showScreen(SCREEN_MENU);
        break;

    case ID_BTN_MODE_START:
    {
        int row = (int)SendDlgItemMessageW(hwnd, ID_COMBO_ROW, CB_GETCURSEL, 0, 0);

        if (row != 0 && (
            IsDlgButtonChecked(hwnd, ID_RADIO_WORDS) == BST_CHECKED ||
            IsDlgButtonChecked(hwnd, ID_RADIO_TEXT)  == BST_CHECKED))
        {
            MessageBoxW(hwnd,
                L"Слова и Текст доступны только при выборе ряда «Все»",
                L"Ошибка", MB_OK | MB_ICONWARNING);
            break;
        }

        if (IsDlgButtonChecked(hwnd, ID_RADIO_LETTERS) == BST_CHECKED)   m_selectedMode = 0;
        if (IsDlgButtonChecked(hwnd, ID_RADIO_SYLLABLES) == BST_CHECKED) m_selectedMode = 1;
        if (IsDlgButtonChecked(hwnd, ID_RADIO_WORDS) == BST_CHECKED)     m_selectedMode = 2;
        if (IsDlgButtonChecked(hwnd, ID_RADIO_TEXT) == BST_CHECKED)      m_selectedMode = 3;

        int durSel = (int)SendDlgItemMessageW(hwnd, ID_COMBO_DURATION, CB_GETCURSEL, 0, 0);
        m_selectedDuration = (durSel == 0) ? 30 : (durSel == 1) ? 60 : 120;

        m_selectedRow = row;

        showScreen(SCREEN_EXERCISE);
        startExercise(hwnd);
        break;
    }

    case ID_BTN_EXERCISE_FINISH:
        finishExercise(hwnd);
        break;

    case ID_BTN_EXERCISE_BACK:
        if (m_exerciseActive) {
            m_exerciseActive = false;
            KillTimer(hwnd, TIMER_EXERCISE_ID);
        }
        showScreen(SCREEN_MENU);
        break;

    case ID_BTN_RESULT_HEATMAP:
        showScreen(SCREEN_HEATMAP);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
        break;

    case ID_BTN_RESULT_MENU:
        showScreen(SCREEN_MENU);
        break;

    case ID_BTN_HISTORY_BACK:
        showScreen(SCREEN_MENU);
        break;

    case ID_BTN_HISTORY_CLEAR:
    {
        if (m_selectedProfileId < 0) break;

        int answer = MessageBoxW(hwnd,
            L"Удалить всю историю?",
            L"Подтверждение",
            MB_YESNO | MB_ICONQUESTION);

        if (answer == IDYES) {
            DatabaseManager::getInstance().clearHistory(m_selectedProfileId);
            loadHistory();
        }
        break;
    }

    case ID_BTN_SETTINGS_BACK:
        showScreen(SCREEN_MENU);
        break;

    case ID_BTN_HEATMAP_BACK:
        showScreen(SCREEN_MENU);
        break;

    case ID_BTN_SETTINGS_SAVE:
    {
        Settings& s = Settings::getInstance();

        int oldTheme = s.getTheme();

        s.setSoundEnabled(IsDlgButtonChecked(hwnd, ID_CHK_SOUND) == BST_CHECKED);
        int volume = (int)SendDlgItemMessageW(hwnd, ID_TRACK_VOLUME, TBM_GETPOS, 0, 0);
        s.setVolume(volume);

        int newTheme = (IsDlgButtonChecked(hwnd, ID_RADIO_THEME_DARK) == BST_CHECKED) ? 1 : 0;
        s.setTheme(newTheme);

        wchar_t fontBuf[16] = {};
        GetDlgItemTextW(hwnd, ID_EDIT_FONT, fontBuf, 16);
        int fontSize = _wtoi(fontBuf);
        s.setFontSize(fontSize);

        s.save("settings.ini");

        if (oldTheme != newTheme) {
            int answer = MessageBoxW(hwnd,
                L"Тема изменена.\n\nДля применения требуется перезапуск.\n\nПерезапустить сейчас?",
                L"Тема изменена",
                MB_YESNO | MB_ICONQUESTION);

            if (answer == IDYES) {
                wchar_t exePath[MAX_PATH];
                GetModuleFileNameW(NULL, exePath, MAX_PATH);

                STARTUPINFOW si = { sizeof(si) };
                PROCESS_INFORMATION pi = {};
                CreateProcessW(exePath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

                DestroyWindow(hwnd);
                return;
            }
        } else {
            MessageBoxW(hwnd, L"Настройки сохранены", L"Успех", MB_OK | MB_ICONINFORMATION);
        }

        showScreen(SCREEN_MENU);
        break;
    }
    }
}

void MainWindow::onTimer(HWND hwnd, WPARAM wParam)
{
    if (wParam == TIMER_EXERCISE_ID && m_exerciseActive) {
        m_elapsedSeconds++;
        m_remainingSeconds--;

        wchar_t typed[2048] = {};
        GetDlgItemTextW(hwnd, ID_EDIT_TYPING, typed, 2048);
        m_exercise.updateFromTyped(typed);

        updateStats(hwnd);

        if (m_remainingSeconds <= 0) {
            finishExercise(hwnd);
        }
    }
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_CREATE) {
        if (s_instance) {
            s_instance->m_hwnd = hwnd;
            s_instance->onCreate(hwnd);
        }
        return 0;
    }

    if (!s_instance) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg)
    {
    case WM_SIZE:
        return 0;

    case WM_COMMAND:
        s_instance->onCommand(hwnd, wParam, lParam);
        return 0;

    case WM_TIMER:
        s_instance->onTimer(hwnd, wParam);
        return 0;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
        if (dis->CtlType != ODT_BUTTON) break;

        HWND hBtn = dis->hwndItem;
        int id = GetDlgCtrlID(hBtn);

        wchar_t text[128] = {};
        GetWindowTextW(hBtn, text, 128);

        bool primary = isPrimaryButton(id);
        bool pressed = (dis->itemState & ODS_SELECTED) != 0;
        bool hovered = (GetPropW(hBtn, L"hovered") != NULL);

        FlatButton::draw(dis, text, primary, hovered, pressed);
        return TRUE;
    }

    case WM_HSCROLL:
    {
        HWND hTrack = (HWND)lParam;
        if (hTrack && GetDlgCtrlID(hTrack) == ID_TRACK_VOLUME) {
            int vol = (int)SendMessageW(hTrack, TBM_GETPOS, 0, 0);

            DWORD volValue = (DWORD)((vol * 0xFFFF) / 100);
            DWORD volBoth = (volValue << 16) | volValue;
            waveOutSetVolume(NULL, volBoth);

            WORD code = LOWORD(wParam);
            if (code == TB_ENDTRACK || code == TB_THUMBPOSITION) {
                Beep(800, 50);
            }
        }
        return 0;
    }

    case WM_PAINT:
    {
        if (s_instance->m_currentScreen == SCREEN_HEATMAP) {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH hBg = CreateSolidBrush(Settings::getInstance().getBackgroundColor());
            FillRect(hdc, &clientRect, hBg);
            DeleteObject(hBg);

            s_instance->drawHeatmap(hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        HWND hCtrl = (HWND)lParam;
        if (GetDlgCtrlID(hCtrl) == ID_TRACK_VOLUME) {
            return DefWindowProcW(hwnd, msg, wParam, lParam);
        }

        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, Settings::getInstance().getBackgroundColor());
        SetTextColor(hdc, Settings::getInstance().getTextColor());
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH hBrush = CreateSolidBrush(Settings::getInstance().getBackgroundColor());
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
        return 1;
    }

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

COLORREF MainWindow::heatmapColor(int errorCount, int maxErrors)
{
    if (maxErrors <= 0) return RGB(200, 255, 200);

    double ratio = (double)errorCount / maxErrors;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    int r, g, b;
    if (ratio < 0.5) {
        double t = ratio * 2.0;
        r = (int)(200 * t + 100 * (1 - t));
        g = 230;
        b = (int)(100 * (1 - t));
    } else {
        double t = (ratio - 0.5) * 2.0;
        r = 230;
        g = (int)(230 * (1 - t));
        b = 100;
    }

    return RGB(r, g, b);
}

void MainWindow::drawHeatmap(HDC hdc)
{
    if (m_selectedProfileId < 0) return;

    auto keyErrors = DatabaseManager::getInstance().getKeyErrors(m_selectedProfileId);

    int maxErrors = 1;
    for (const auto& e : keyErrors) {
        if (e.errorCount > maxErrors) maxErrors = e.errorCount;
    }

    const wchar_t* row1 = L"йцукенгшщзхъ";
    const wchar_t* row2 = L"фывапролджэ";
    const wchar_t* row3 = L"ячсмитьбю";

    int keyW = 55;
    int keyH = 55;
    int spacing = 4;

    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    int kbW = 12 * (keyW + spacing);
    int kbH = 3 * (keyH + spacing);

    int startX = (clientRect.right - kbW) / 2;
    int startY = (clientRect.bottom - kbH) / 2;

    const wchar_t* rows[] = { row1, row2, row3 };
    int rowOffsets[] = { 0, 15, 30 };

    for (int r = 0; r < 3; r++) {
        const wchar_t* row = rows[r];
        int len = (int)wcslen(row);

        for (int i = 0; i < len; i++) {
            wchar_t key = row[i];

            int errCount = 0;
            for (const auto& e : keyErrors) {
                if (normalizeChar(e.keyChar) == normalizeChar(key)) {
                    errCount = e.errorCount;
                    break;
                }
            }

            int x = startX + i * (keyW + spacing) + rowOffsets[r];
            int y = startY + r * (keyH + spacing);

            COLORREF color = heatmapColor(errCount, maxErrors);
            HBRUSH hBrush = CreateSolidBrush(color);
            RECT rect = { x, y, x + keyW, y + keyH };
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);

            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, x, y, x + keyW, y + keyH);
            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);

            wchar_t keyStr[2] = { key, 0 };

            HFONT hKeyFont = CreateFontW(28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(hdc, hKeyFont);

            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(20, 20, 20));

            DrawTextW(hdc, keyStr, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            SelectObject(hdc, hOldFont);
            DeleteObject(hKeyFont);

            if (errCount > 0) {
                wchar_t numStr[8];
                swprintf(numStr, 8, L"%d", errCount);

                HFONT hNumFont = CreateFontW(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                    CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
                HFONT hOldFont2 = (HFONT)SelectObject(hdc, hNumFont);

                RECT numRect = { x, y + keyH - 18, x + keyW, y + keyH - 2 };
                SetTextColor(hdc, RGB(80, 80, 80));
                DrawTextW(hdc, numStr, -1, &numRect, DT_CENTER | DT_SINGLELINE);

                SelectObject(hdc, hOldFont2);
                DeleteObject(hNumFont);
            }
        }
    }
}