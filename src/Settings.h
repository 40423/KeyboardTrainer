#pragma once
#include <string>
#include <windows.h>

class Settings {
public:
    static Settings& getInstance();

    void load(const std::string& filename);
    void save(const std::string& filename);

    bool getSoundEnabled() const { return m_soundEnabled; }
    void setSoundEnabled(bool v) { m_soundEnabled = v; }

    int getVolume() const { return m_volume; }
    void setVolume(int v) {
        if (v < 0) v = 0;
        if (v > 100) v = 100;
        m_volume = v;
    }

    int getTheme() const { return m_theme; }
    void setTheme(int v) { m_theme = v; }

    int getFontSize() const { return m_fontSize; }
    void setFontSize(int v) {
        if (v < 12) v = 12;
        if (v > 48) v = 48;
        m_fontSize = v;
    }

    // ===== Фон окна =====
    COLORREF getBackgroundColor() const {
        return (m_theme == 0) ? RGB(245, 245, 245) : RGB(30, 34, 42);
    }

    // ===== Цвет текста =====
    COLORREF getTextColor() const {
        return (m_theme == 0) ? RGB(26, 26, 26) : RGB(212, 216, 224);
    }

    // ===== Цвет панелей =====
    COLORREF getPanelColor() const {
        return (m_theme == 0) ? RGB(255, 255, 255) : RGB(40, 46, 56);
    }

    // ===== Акцент =====
    COLORREF getAccentColor() const {
        return (m_theme == 0) ? RGB(45, 108, 223) : RGB(82, 139, 255);
    }
    COLORREF getAccentHoverColor() const {
        return (m_theme == 0) ? RGB(30, 90, 200) : RGB(110, 160, 255);
    }
    COLORREF getAccentPressedColor() const {
        return (m_theme == 0) ? RGB(20, 70, 170) : RGB(60, 110, 220);
    }

    // ===== Рамки =====
    COLORREF getBorderColor() const {
        return (m_theme == 0) ? RGB(220, 220, 220) : RGB(60, 68, 80);
    }

    // ===== Карточки =====
    COLORREF getCardColor() const {
        return (m_theme == 0) ? RGB(255, 255, 255) : RGB(38, 44, 54);
    }

private:
    Settings();
    ~Settings() = default;

    bool m_soundEnabled;
    int  m_volume;
    int  m_theme;
    int  m_fontSize;
};