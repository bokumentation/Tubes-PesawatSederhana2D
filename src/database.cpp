#include "database.h"

#include <raylib.h>   // For TraceLog and TextFormat
#include <sqlite3.h>  // SQLite C API header

#include <cstring>  // For strcmp

// Global database connection handle
static sqlite3* db = nullptr;

// Callback function for sqlite3_exec to process query results
static int callback(void* data, int argc, char** argv, char** azColName) {
  std::vector<EntriLeaderboard>* results =
      static_cast<std::vector<EntriLeaderboard>*>(data);
  EntriLeaderboard entry;
  // Initialize with default values to prevent garbage
  entry.nama = "";
  entry.skor = 0;

  for (int i = 0; i < argc; i++) {
    if (azColName[i] != nullptr &&
        argv[i] != nullptr) {  // Check for nullptr before strcmp
      if (strcmp(azColName[i], "nama") == 0) {
        entry.nama = argv[i];
      } else if (strcmp(azColName[i], "skor") == 0) {
        try {
          entry.skor = std::stoi(argv[i]);
        } catch (const std::exception& e) {
          TraceLog(LOG_WARNING,
                   TextFormat("SQLITE: Gagal mengonversi skor '%s': %s",
                              argv[i], e.what()));
          entry.skor = 0;  // Default to 0 on error
        }
      }
    }
  }
  results->push_back(entry);
  return 0;  // Return 0 to continue processing rows
}

bool InitDatabase(const std::string& dbPath) {
  int rc = sqlite3_open(dbPath.c_str(), &db);
  if (rc) {
    TraceLog(LOG_ERROR, TextFormat("SQLITE: Gagal membuka database '%s': %s",
                                   dbPath.c_str(), sqlite3_errmsg(db)));
    db = nullptr;  // Ensure db is null if opening failed
    return false;
  } else {
    TraceLog(LOG_INFO, TextFormat("SQLITE: Database '%s' berhasil dibuka.",
                                  dbPath.c_str()));
  }

  // Create table if it doesn't exist
  const char* sql_create_table =
      "CREATE TABLE IF NOT EXISTS leaderboard ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
      "nama TEXT NOT NULL,"
      "skor INTEGER NOT NULL);";
  char* zErrMsg = nullptr;
  rc = sqlite3_exec(db, sql_create_table, nullptr, nullptr, &zErrMsg);
  if (rc != SQLITE_OK) {
    TraceLog(
        LOG_ERROR,
        TextFormat("SQLITE: Gagal membuat tabel 'leaderboard': %s", zErrMsg));
    sqlite3_free(zErrMsg);
    sqlite3_close(db);
    db = nullptr;
    return false;
  }
  TraceLog(LOG_INFO, "SQLITE: Tabel 'leaderboard' siap.");
  return true;
}

void CloseDatabase() {
  if (db) {
    sqlite3_close(db);
    db = nullptr;
    TraceLog(LOG_INFO, "SQLITE: Database ditutup.");
  }
}

void InsertScore(const std::string& nama, int skor) {
  if (!db) {
    TraceLog(LOG_WARNING,
             "SQLITE: Tidak ada database yang terbuka untuk menyisipkan skor.");
    return;
  }
  char* zErrMsg = nullptr;
  // Using a prepared statement is safer for user input, but for simplicity
  // with sqlite3_exec and known input constraints, string formatting is used
  // here. In a real application, prepared statements are strongly recommended
  // to prevent SQL injection.
  std::string sql =
      TextFormat("INSERT INTO leaderboard (nama, skor) VALUES ('%s', %i);",
                 nama.c_str(), skor);
  int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &zErrMsg);
  if (rc != SQLITE_OK) {
    TraceLog(LOG_ERROR,
             TextFormat("SQLITE: Gagal menyisipkan skor untuk '%s': %s",
                        nama.c_str(), zErrMsg));
    sqlite3_free(zErrMsg);
  } else {
    TraceLog(LOG_INFO, TextFormat("SQLITE: Skor '%s': %i berhasil disisipkan.",
                                  nama.c_str(), skor));
  }
}

std::vector<EntriLeaderboard> GetTopScores(int limit) {
  std::vector<EntriLeaderboard> scores;
  if (!db) {
    TraceLog(LOG_WARNING,
             "SQLITE: Tidak ada database yang terbuka untuk mendapatkan skor "
             "teratas.");
    return scores;
  }
  char* zErrMsg = nullptr;
  std::string sql = TextFormat(
      "SELECT nama, skor FROM leaderboard ORDER BY skor DESC LIMIT %i;", limit);
  int rc = sqlite3_exec(db, sql.c_str(), callback, &scores, &zErrMsg);
  if (rc != SQLITE_OK) {
    TraceLog(LOG_ERROR,
             TextFormat("SQLITE: Gagal mendapatkan skor teratas: %s", zErrMsg));
    sqlite3_free(zErrMsg);
  }
  return scores;
}

std::vector<EntriLeaderboard> SearchScores(const std::string& searchTerm) {
  std::vector<EntriLeaderboard> scores;
  if (!db) {
    TraceLog(LOG_WARNING,
             "SQLITE: Tidak ada database yang terbuka untuk pencarian skor.");
    return scores;
  }
  char* zErrMsg = nullptr;
  // SQL LIKE operator requires '%' wildcards. Escape single quotes in
  // searchTerm for safety. In a real application, use sqlite3_bind_text with
  // prepared statements for safer LIKE queries.
  std::string escapedSearchTerm = searchTerm;
  // Replace single quotes with two single quotes to escape them in SQL string
  // literals
  size_t pos = escapedSearchTerm.find('\'');
  while (pos != std::string::npos) {
    escapedSearchTerm.replace(pos, 1, "''");
    pos = escapedSearchTerm.find(
        '\'', pos + 2);  // Find next quote after the inserted one
  }

  std::string sql = TextFormat(
      "SELECT nama, skor FROM leaderboard WHERE nama LIKE '%%%s%%' ORDER BY "
      "skor DESC, nama ASC;",
      escapedSearchTerm.c_str());
  int rc = sqlite3_exec(db, sql.c_str(), callback, &scores, &zErrMsg);
  if (rc != SQLITE_OK) {
    TraceLog(LOG_ERROR, TextFormat("SQLITE: Gagal mencari skor: %s", zErrMsg));
    sqlite3_free(zErrMsg);
  }
  return scores;
}