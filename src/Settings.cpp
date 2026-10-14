#include "Settings.h"
#include <fstream>

Settings& Settings::getInstance()
{
    static Settings instance;
    return instance;
}

Settings::Settings()
    : m_soundEnabled(true),
      m_volume(50),
      m_theme(0),
      m_fontSize(20)
{
}

void Settings::load(const std::string& filename)
{
    std::ifstream f(filename);
    if (!f.is_open()) return;

    std::string line;
    while (std::getline(f, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        if (key == "sound")   m_soundEnabled = (val == "1");
        if (key == "volume")  m_volume = std::stoi(val);
        if (key == "theme")   m_theme = std::stoi(val);
        if (key == "font")    m_fontSize = std::stoi(val);
    }
}

void Settings::save(const std::string& filename)
{
    std::ofstream f(filename);
    if (!f.is_open()) return;

    f << "sound=" << (m_soundEnabled ? "1" : "0") << "\n";
    f << "volume=" << m_volume << "\n";
    f << "theme=" << m_theme << "\n";
    f << "font="  << m_fontSize << "\n";
}