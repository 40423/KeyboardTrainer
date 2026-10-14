#include "FlatButton.h"
#include "Settings.h"

void FlatButton::fillRoundRect(HDC hdc, RECT rc, int radius,
                               COLORREF fill, COLORREF border)
{
    HBRUSH hBrush = CreateSolidBrush(fill);
    HPEN hPen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
    HGDIOBJ oldPen   = SelectObject(hdc, hPen);

    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

void FlatButton::draw(LPDRAWITEMSTRUCT dis, const std::wstring& text,
                     bool primary, bool hovered, bool pressed)
{
    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;

    Settings& s = Settings::getInstance();

    COLORREF bg;
    COLORREF fg;
    COLORREF border;

    if (primary) {
        if (pressed)      bg = s.getAccentPressedColor();
        else if (hovered) bg = s.getAccentHoverColor();
        else              bg = s.getAccentColor();
        fg = RGB(255, 255, 255);
        border = bg;
    } else {
        if (pressed)      bg = s.getCardColor();
        else if (hovered) bg = RGB(245, 245, 250);
        else              bg = s.getCardColor();
        fg = s.getTextColor();
        border = s.getBorderColor();
    }

    HBRUSH hBg = CreateSolidBrush(s.getBackgroundColor());
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);

    RECT r = rc;
    InflateRect(&r, -1, -1);
    fillRoundRect(hdc, r, 12, bg, border);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, fg);

    HFONT hFont = CreateFontW(20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

    DrawTextW(hdc, text.c_str(), -1, &rc,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}