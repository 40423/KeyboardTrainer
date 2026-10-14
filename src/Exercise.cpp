#include "Exercise.h"
#include <cwctype>

// Нормализует символ: приводит к нижнему регистру (кириллица + ASCII).
wchar_t normalizeChar(wchar_t c)
{
    if (c >= L'A' && c <= L'Z') return c + (L'a' - L'A');
    if (c >= L'А' && c <= L'Я') return c + (L'а' - L'А');
    if (c == L'Ё') return L'ё';
    return c;
}

Exercise::Exercise()
    : m_correctChars(0), m_errorChars(0)
{
}

void Exercise::reset()
{
    m_text.clear();
    m_correctChars = 0;
    m_errorChars = 0;
}

void Exercise::setText(const std::wstring& text)
{
    m_text = text;
}

const std::wstring& Exercise::getText() const
{
    return m_text;
}

void Exercise::updateFromTyped(const std::wstring& typed)
{
    m_correctChars = 0;
    m_errorChars = 0;

    int typedLen = (int)typed.length();
    int textLen  = (int)m_text.length();
    int compareLen = (typedLen < textLen) ? typedLen : textLen;

    for (int i = 0; i < compareLen; i++) {
        wchar_t a = normalizeChar(typed[i]);
        wchar_t b = normalizeChar(m_text[i]);
        if (a == b)
            m_correctChars++;
        else
            m_errorChars++;
    }
}

int Exercise::getCorrectChars() const { return m_correctChars; }
int Exercise::getErrorChars() const { return m_errorChars; }
int Exercise::getTotalChars() const { return m_correctChars + m_errorChars; }

int Exercise::calculateWPM(int elapsedSeconds) const
{
    if (elapsedSeconds < 1) return 0;
    double minutes = elapsedSeconds / 60.0;
    return (int)((m_correctChars / 5.0) / minutes);
}

int Exercise::calculateErrorRate() const
{
    int total = getTotalChars();
    if (total == 0) return 0;
    return (int)(100.0 * m_errorChars / total);
}