#include <windows.h>
#include <commctrl.h>
#include "MainWindow.h"
#include "DatabaseManager.h"
#include "Settings.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    InitCommonControls();

    DatabaseManager::getInstance().open("keyboard_trainer.db");
    Settings::getInstance().load("settings.ini");

    MainWindow window;
    if (!window.create(hInstance, nCmdShow)) {
        MessageBoxW(NULL, L"Не удалось создать окно", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DatabaseManager::getInstance().close();
    return 0;
}