#include <windows.h>
#include "DatabaseManager.h"
#include "../lib/sqlite3.h"
#include <cstring>

struct DatabaseManager::Impl {
    sqlite3* db = nullptr;
};

DatabaseManager& DatabaseManager::getInstance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() : m_impl(new Impl()) {}
DatabaseManager::~DatabaseManager() { close(); delete m_impl; }

bool DatabaseManager::open(const std::string& filename)
{
    if (m_impl->db) return true;

    int rc = sqlite3_open(filename.c_str(), &m_impl->db);
    if (rc != SQLITE_OK) {
        m_impl->db = nullptr;
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS profiles ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "password TEXT NOT NULL"
        ");";

    char* errMsg = nullptr;
    sqlite3_exec(m_impl->db, sql, nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);

    return true;
}

void DatabaseManager::close()
{
    if (m_impl->db) {
        sqlite3_close(m_impl->db);
        m_impl->db = nullptr;
    }
}

static std::string toUtf8(const std::wstring& w)
{
    if (w.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
                                   nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(),
                        &result[0], size, nullptr, nullptr);
    return result;
}

static std::wstring toWString(const char* s)
{
    if (!s) return {};
    int size = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    if (size <= 0) return {};
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, s, -1, &result[0], size);
    return result;
}

bool DatabaseManager::addProfile(const std::wstring& name, const std::wstring& password)
{
    if (!m_impl->db) return false;

    std::string n = toUtf8(name);
    std::string p = toUtf8(password);

    const char* sql = "INSERT INTO profiles (name, password) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, n.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, p.c_str(), -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<Profile> DatabaseManager::getAllProfiles()
{
    std::vector<Profile> result;
    if (!m_impl->db) return result;

    const char* sql = "SELECT id, name, password FROM profiles;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Profile p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = toWString((const char*)sqlite3_column_text(stmt, 1));
        p.password = toWString((const char*)sqlite3_column_text(stmt, 2));
        result.push_back(p);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::deleteProfile(int id)
{
    if (!m_impl->db) return false;

    const char* sql = "DELETE FROM profiles WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DatabaseManager::checkPassword(int id, const std::wstring& password)
{
    if (!m_impl->db) return false;

    std::string p = toUtf8(password);

    const char* sql = "SELECT password FROM profiles WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, id);

    bool ok = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* stored = (const char*)sqlite3_column_text(stmt, 0);
        if (stored && p == stored) ok = true;
    }

    sqlite3_finalize(stmt);
    return ok;
}

int DatabaseManager::saveExercise(int userId, int mode, int wpm, int errorRate, int duration)
{
    if (!m_impl->db) return -1;

    const char* createSql =
        "CREATE TABLE IF NOT EXISTS exercises ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "mode INTEGER NOT NULL,"
        "wpm INTEGER NOT NULL,"
        "error_rate INTEGER NOT NULL,"
        "duration INTEGER NOT NULL,"
        "created_at DATETIME"
        ");";

    char* errMsg = nullptr;
    sqlite3_exec(m_impl->db, createSql, nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);

    SYSTEMTIME st;
    GetLocalTime(&st);

    char dateBuf[32];
    sprintf(dateBuf, "%04d-%02d-%02d %02d:%02d:%02d",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond);

    const char* sql =
        "INSERT INTO exercises (user_id, mode, wpm, error_rate, duration, created_at) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, mode);
    sqlite3_bind_int(stmt, 3, wpm);
    sqlite3_bind_int(stmt, 4, errorRate);
    sqlite3_bind_int(stmt, 5, duration);
    sqlite3_bind_text(stmt, 6, dateBuf, -1, SQLITE_TRANSIENT);

    int newId = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        newId = (int)sqlite3_last_insert_rowid(m_impl->db);
    }
    sqlite3_finalize(stmt);
    return newId;
}

bool DatabaseManager::updateExercise(int exerciseId, int wpm, int errorRate, int duration)
{
    if (!m_impl->db) return false;

    const char* sql =
        "UPDATE exercises SET wpm = ?, error_rate = ?, duration = ? "
        "WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, wpm);
    sqlite3_bind_int(stmt, 2, errorRate);
    sqlite3_bind_int(stmt, 3, duration);
    sqlite3_bind_int(stmt, 4, exerciseId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<HistoryEntry> DatabaseManager::getHistory(int userId)
{
    std::vector<HistoryEntry> result;
    if (!m_impl->db) return result;

    const char* sql =
        "SELECT id, mode, wpm, error_rate, duration, created_at "
        "FROM exercises WHERE user_id = ? "
        "ORDER BY created_at DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    sqlite3_bind_int(stmt, 1, userId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        HistoryEntry e;
        e.id = sqlite3_column_int(stmt, 0);
        e.mode = sqlite3_column_int(stmt, 1);
        e.wpm = sqlite3_column_int(stmt, 2);
        e.errorRate = sqlite3_column_int(stmt, 3);
        e.duration = sqlite3_column_int(stmt, 4);

        const char* dateStr = (const char*)sqlite3_column_text(stmt, 5);
        if (dateStr) {
            e.date = toWString(dateStr);
        }

        result.push_back(e);
    }

    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::clearHistory(int userId)
{
    if (!m_impl->db) return false;

    const char* sql1 =
        "DELETE FROM key_errors WHERE exercise_id IN "
        "(SELECT id FROM exercises WHERE user_id = ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_impl->db, sql1, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char* sql = "DELETE FROM exercises WHERE user_id = ?;";
    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, userId);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool DatabaseManager::saveKeyError(int exerciseId, wchar_t keyChar)
{
    if (!m_impl->db) return false;

    const char* createSql =
        "CREATE TABLE IF NOT EXISTS key_errors ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "exercise_id INTEGER NOT NULL,"
        "key_char TEXT NOT NULL,"
        "error_count INTEGER DEFAULT 1"
        ");";

    char* errMsg = nullptr;
    sqlite3_exec(m_impl->db, createSql, nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);

    char keyBuf[8] = {};
    WideCharToMultiByte(CP_UTF8, 0, &keyChar, 1, keyBuf, 8, nullptr, nullptr);

    const char* checkSql =
        "SELECT id, error_count FROM key_errors "
        "WHERE exercise_id = ? AND key_char = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_impl->db, checkSql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, exerciseId);
    sqlite3_bind_text(stmt, 2, keyBuf, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);
        sqlite3_finalize(stmt);

        const char* updateSql = "UPDATE key_errors SET error_count = ? WHERE id = ?;";
        if (sqlite3_prepare_v2(m_impl->db, updateSql, -1, &stmt, nullptr) != SQLITE_OK)
            return false;
        sqlite3_bind_int(stmt, 1, count + 1);
        sqlite3_bind_int(stmt, 2, id);
        bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
        sqlite3_finalize(stmt);
        return ok;
    }
    sqlite3_finalize(stmt);

    const char* insertSql =
        "INSERT INTO key_errors (exercise_id, key_char, error_count) "
        "VALUES (?, ?, 1);";

    if (sqlite3_prepare_v2(m_impl->db, insertSql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, exerciseId);
    sqlite3_bind_text(stmt, 2, keyBuf, -1, SQLITE_TRANSIENT);

    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

std::vector<KeyErrorEntry> DatabaseManager::getKeyErrors(int userId)
{
    std::vector<KeyErrorEntry> result;
    if (!m_impl->db) return result;

    const char* sql =
        "SELECT key_char, SUM(error_count) AS total "
        "FROM key_errors "
        "WHERE exercise_id IN (SELECT id FROM exercises WHERE user_id = ?) "
        "GROUP BY key_char "
        "ORDER BY total DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_impl->db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    sqlite3_bind_int(stmt, 1, userId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* keyStr = (const char*)sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);

        if (keyStr && keyStr[0]) {
            KeyErrorEntry e;
            wchar_t wch[4] = {};
            MultiByteToWideChar(CP_UTF8, 0, keyStr, -1, wch, 4);
            e.keyChar = wch[0];
            e.errorCount = count;
            result.push_back(e);
        }
    }

    sqlite3_finalize(stmt);
    return result;
}