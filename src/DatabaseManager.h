#pragma once
#include <string>
#include <vector>

struct Profile {
    int id;
    std::wstring name;
    std::wstring password;
};

struct HistoryEntry {
    int id;
    int mode;
    int wpm;
    int errorRate;
    int duration;
    std::wstring date;
};

struct KeyErrorEntry {
    wchar_t keyChar;
    int errorCount;
};

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    bool open(const std::string& filename);
    void close();

    bool addProfile(const std::wstring& name, const std::wstring& password);
    std::vector<Profile> getAllProfiles();
    bool deleteProfile(int id);
    bool checkPassword(int id, const std::wstring& password);

    int saveExercise(int userId, int mode, int wpm, int errorRate, int duration);
    bool updateExercise(int exerciseId, int wpm, int errorRate, int duration);
    std::vector<HistoryEntry> getHistory(int userId);
    bool clearHistory(int userId);

    bool saveKeyError(int exerciseId, wchar_t keyChar);
    std::vector<KeyErrorEntry> getKeyErrors(int userId);

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager();
    ~DatabaseManager();

    struct Impl;
    Impl* m_impl;
};