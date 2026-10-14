#pragma once
#include <windows.h>
#include <string>

class FlatButton {
public:
    static void draw(LPDRAWITEMSTRUCT dis, const std::wstring& text,
                     bool primary, bool hovered, bool pressed);

    static void fillRoundRect(HDC hdc, RECT rc, int radius,
                              COLORREF fill, COLORREF border);
};