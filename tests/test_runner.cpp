#include "../src/Exercise.h"
#include "../src/ExerciseFactory.h"
#include "../src/Settings.h"
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <sstream>

// ===== Глобальные результаты =====
static int g_passed = 0;
static int g_failed = 0;
static std::wstringstream g_report;

#define CHECK(cond, msg) do { \
    if (cond) { g_passed++; g_report << L"  [OK]   " << msg << L"\r\n"; } \
    else      { g_failed++; g_report << L"  [FAIL] " << msg << L"\r\n"; } \
} while (0)

// ===== Тесты =====

static void testExerciseBasic()
{
    g_report << L"Exercise: базовые сценарии\r\n";

    Exercise ex;
    ex.setText(L"дом");
    ex.updateFromTyped(L"дом");
    CHECK(ex.getCorrectChars() == 3, L"все 3 символа верны");
    CHECK(ex.getErrorChars()   == 0, L"0 ошибок");

    ex.updateFromTyped(L"длм");
    CHECK(ex.getCorrectChars() == 2, L"2 верных символа");
    CHECK(ex.getErrorChars()   == 1, L"1 ошибка");

    ex.updateFromTyped(L"");
    CHECK(ex.getCorrectChars() == 0, L"пустой ввод: 0 верных");
    CHECK(ex.getErrorChars()   == 0, L"пустой ввод: 0 ошибок");

    ex.setText(L"дом");
    ex.updateFromTyped(L"ДОМ");
    CHECK(ex.getErrorChars() == 0, L"регистр не важен (ДОМ == дом)");

    ex.setText(L"дом");
    ex.updateFromTyped(L"домик");
    CHECK(ex.getCorrectChars() == 3, L"лишние символы игнорируются");
}

static void testReset()
{
    g_report << L"Exercise: reset\r\n";

    Exercise ex;
    ex.setText(L"тест");
    ex.updateFromTyped(L"тест");
    CHECK(ex.getTotalChars() > 0, L"до reset счётчики заполнены");

    ex.reset();
    CHECK(ex.getCorrectChars() == 0, L"после reset correct == 0");
    CHECK(ex.getErrorChars()   == 0, L"после reset errors == 0");
    CHECK(ex.getTotalChars()   == 0, L"после reset total == 0");
}

static void testWpm()
{
    g_report << L"Exercise: WPM\r\n";

    Exercise ex;
    ex.setText(L"абвгдеёжзийклмнопрстуфхцчшщъыьэюя");
    ex.updateFromTyped(L"абвгдеёжзийклмнопрстуфхцчшщъыьэюя");

    int wpm = ex.calculateWPM(60);
    CHECK(wpm == 6 || wpm == 7, L"WPM = 6-7 при 33 символах и 60 сек");

    int wpm30 = ex.calculateWPM(30);
    CHECK(wpm30 == 13 || wpm30 == 14, L"WPM = 13-14 при 33 символах и 30 сек");

    CHECK(ex.calculateWPM(0) == 0, L"WPM = 0 при нулевом времени");
    CHECK(ex.calculateWPM(-5) == 0, L"WPM = 0 при отрицательном времени");
}

static void testErrorRate()
{
    g_report << L"Exercise: процент ошибок\r\n";

    Exercise ex;
    ex.setText(L"абвгд");
    ex.updateFromTyped(L"абвгд");
    CHECK(ex.calculateErrorRate() == 0, L"0% при безошибочном вводе");

    ex.updateFromTyped(L"абвге");
    CHECK(ex.calculateErrorRate() == 20, L"20% при 1 ошибке из 5");

    ex.updateFromTyped(L"абааа");
    CHECK(ex.calculateErrorRate() == 60, L"60% при 3 ошибках из 5");

    ex.updateFromTyped(L"ааааа");
    CHECK(ex.calculateErrorRate() == 80, L"80% при 4 ошибках из 5");
}

static void testFactory()
{
    g_report << L"ExerciseFactory: генерация\r\n";

    for (int mode = 0; mode <= 3; mode++) {
        std::wstring ex = ExerciseFactory::createExercise(
            (ExerciseFactory::Mode)mode, 0);
        CHECK(!ex.empty(), L"режим вернул непустую строку");
    }

    std::wstring letters = ExerciseFactory::createExercise(
        ExerciseFactory::MODE_LETTERS, 0);
    CHECK(letters.find(L' ') != std::wstring::npos, L"режим Буквы содержит пробелы");

    std::wstring syllables = ExerciseFactory::createExercise(
        ExerciseFactory::MODE_SYLLABLES, 0);
    CHECK(syllables.find(L' ') != std::wstring::npos, L"режим Слоги содержит пробелы");

    std::wstring words = ExerciseFactory::createExercise(
        ExerciseFactory::MODE_WORDS, 0);
    CHECK(words.find(L' ') != std::wstring::npos, L"режим Слова содержит пробелы");

    std::wstring text = ExerciseFactory::createExercise(
        ExerciseFactory::MODE_TEXT, 0);
    CHECK(text.length() > 10, L"режим Текст содержит осмысленный текст");

    bool hasLongDash = false;
    for (int i = 0; i < 20; i++) {
        std::wstring t = ExerciseFactory::createExercise(
            ExerciseFactory::MODE_TEXT, 0);
        if (t.find(L'\u2014') != std::wstring::npos) hasLongDash = true;
    }
    CHECK(!hasLongDash, L"в текстах нет длинных тире");
}

static void testSettings()
{
    g_report << L"Settings: валидация\r\n";

    Settings& s = Settings::getInstance();

    s.setFontSize(5);
    CHECK(s.getFontSize() == 12, L"fontSize < 12 превращается в 12");

    s.setFontSize(100);
    CHECK(s.getFontSize() == 48, L"fontSize > 48 превращается в 48");

    s.setFontSize(20);
    CHECK(s.getFontSize() == 20, L"fontSize = 20 сохраняется");

    s.setVolume(-5);
    CHECK(s.getVolume() == 0, L"volume < 0 превращается в 0");

    s.setVolume(200);
    CHECK(s.getVolume() == 100, L"volume > 100 превращается в 100");

    s.setVolume(50);
    CHECK(s.getVolume() == 50, L"volume = 50 сохраняется");
}

// ===== Запуск всех тестов =====
static void runAllTests()
{
    g_passed = 0;
    g_failed = 0;
    g_report.str(L"");
    g_report.clear();

    g_report << L"=== Диагностика АС KeyboardTrainer ===\r\n\r\n";

    testExerciseBasic();
    testReset();
    testWpm();
    testErrorRate();
    testFactory();
    testSettings();

    g_report << L"\r\n=== Итого: " << g_passed << L" пройдено, "
             << g_failed << L" провалено ===\r\n";
}

// ===== Оконная процедура =====
#define ID_LIST_REPORT 1001
#define ID_BTN_CLOSE   1002
#define ID_LABEL_SUMMARY 1003

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

        HFONT hTitleFont = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        // Заголовок
        HWND hTitle = CreateWindowW(L"STATIC", L"Диагностика АС KeyboardTrainer",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            0, 15, 800, 40, hwnd, NULL, NULL, NULL);
        SendMessageW(hTitle, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

        // Список с отчётом
        HWND hList = CreateWindowW(L"LISTBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
            20, 70, 760, 500, hwnd, (HMENU)ID_LIST_REPORT, NULL, NULL);
        SendMessageW(hList, WM_SETFONT, (WPARAM)hFont, TRUE);

        // Разбираем отчёт на строки и добавляем в список
        std::wstring report = g_report.str();
        std::wstring line;
        for (size_t i = 0; i < report.size(); i++) {
            if (report[i] == L'\r') continue;
            if (report[i] == L'\n') {
                SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
                line.clear();
            } else {
                line += report[i];
            }
        }
        if (!line.empty()) {
            SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)line.c_str());
        }

        // Кнопка «Закрыть»
        HWND hClose = CreateWindowW(L"BUTTON", L"Закрыть",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            320, 590, 160, 40, hwnd, (HMENU)ID_BTN_CLOSE, NULL, NULL);
        SendMessageW(hClose, WM_SETFONT, (WPARAM)hTitleFont, TRUE);

        return 0;
    }

    case WM_COMMAND:
    {
        if (LOWORD(wParam) == ID_BTN_CLOSE) {
            DestroyWindow(hwnd);
            return 0;
        }
        break;
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, RGB(245, 245, 245));
        SetTextColor(hdc, RGB(26, 26, 26));
        return (LRESULT)GetStockObject(NULL_BRUSH);
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

// ===== Точка входа =====
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    InitCommonControls();

    // Прогоняем тесты
    runAllTests();

    // Регистрируем класс окна
    const wchar_t CLASS_NAME[] = L"SelfTestWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(RGB(245, 245, 245));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    // Создаём окно
    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Диагностика KeyboardTrainer",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        820, 680,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return g_failed == 0 ? 0 : 1;
}