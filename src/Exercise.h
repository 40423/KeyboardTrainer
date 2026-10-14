#pragma once
#include <string>

// Нормализует символ: приводит к нижнему регистру (кириллица + ASCII).
wchar_t normalizeChar(wchar_t c);

class Exercise {
public:
    Exercise();

    void reset();

    void setText(const std::wstring& text);
    const std::wstring& getText() const;

    void updateFromTyped(const std::wstring& typed);

    int getCorrectChars() const;
    int getErrorChars() const;
    int getTotalChars() const;

    int calculateWPM(int elapsedSeconds) const;
    int calculateErrorRate() const;

private:
    std::wstring m_text;
    int m_correctChars;
    int m_errorChars;
};