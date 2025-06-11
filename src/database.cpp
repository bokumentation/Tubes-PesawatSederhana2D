/**
 * @file database.cpp
 * @brief Implementasi fungsi-fungsi untuk interaksi dengan database SQLite.
 *
 * File ini berisi definisi fungsi untuk mengelola koneksi database,
 * menyisipkan skor, serta mengambil dan mencari entri leaderboard.
 */

#include "database.h"

#include <raylib.h>
#include <sqlite3.h>

#include <cstring>
#include <stdexcept>  // For std::stoi exceptions

#include "player_data.h"

// JANGAN DIOTAK-ATIK! bahkan diriku nda paham kode ini - ibe

// Variabel koneksi database SQLite.
// static di sini berarti variabel ini hanya terlihat di dalam file
// database.cpp.
static sqlite3* db = nullptr;

/**
 * @brief Fungsi callback yang dipanggil oleh SQLite untuk setiap baris hasil
 * SELECT.
 *
 * Fungsi ini memproses satu baris data dari hasil query SQL, mengonversi nilai
 * kolom "nama" dan "skor" menjadi format EntriLeaderboard, dan menambahkannya
 * ke vektor hasil yang disediakan.
 *
 * @param data Pointer void* ke data yang diberikan oleh sqlite3_exec,
 * yang di-cast ke std::vector<EntriLeaderboard>* untuk menyimpan hasil.
 * @param argc Jumlah kolom dalam baris hasil saat ini.
 * @param argv Array of strings (char**) yang berisi nilai-nilai kolom.
 * @param azColName Array of strings (char**) yang berisi nama-nama kolom.
 * @return Mengembalikan 0 untuk melanjutkan pemrosesan baris berikutnya.
 * Jika mengembalikan nilai bukan nol, sqlite3_exec akan berhenti.
 */
static int callback(void* data, int argc, char** argv, char** azColName) {
  std::vector<EntriLeaderboard>* results =
      static_cast<std::vector<EntriLeaderboard>*>(data);
  EntriLeaderboard entry;
  entry.nama = "";
  entry.skor = 0;

  for (int i = 0; i < argc; i++) {
    if (azColName[i] != nullptr && argv[i] != nullptr) {
      if (strcmp(azColName[i], "nama") == 0) {
        entry.nama = argv[i];
      } else if (strcmp(azColName[i], "skor") == 0) {
        try {
          entry.skor = std::stoi(argv[i]);
        } catch (const std::exception& e) {
          TraceLog(LOG_WARNING,
                   TextFormat("SQLITE: Gagal mengonversi skor '%s': %s",
                              argv[i], e.what()));
          entry.skor = 0;
        }
      }
    }
  }
  results->push_back(entry);
  return 0;
}

bool InitDatabase(const std::string& dbPath) {
  int rc = sqlite3_open(dbPath.c_str(), &db);
  if (rc) {
    TraceLog(LOG_ERROR, TextFormat("SQLITE: Gagal membuka database '%s': %s",
                                   dbPath.c_str(), sqlite3_errmsg(db)));
    db = nullptr;
    return false;
  } else {
    TraceLog(LOG_INFO, TextFormat("SQLITE: Database '%s' berhasil dibuka.",
                                  dbPath.c_str()));
  }

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

std::vector<EntriLeaderboard> GetLeaderboardEntries(
    int limit, LeaderboardSortOrder order) {
  std::vector<EntriLeaderboard> scores;
  if (!db) {
    TraceLog(LOG_WARNING,
             "SQLITE: Tidak ada database yang terbuka untuk mendapatkan entri "
             "leaderboard.");
    return scores;
  }
  char* zErrMsg = nullptr;
  std::string orderByClause;
  if (order == SORT_BY_SCORE_DESC) {
    orderByClause = "skor DESC";
  } else {  // SORT_BY_NAME_ASC
    orderByClause = "nama ASC";
  }

  std::string sql;
  if (limit > 0) {  // Apply limit if greater than 0
    sql = TextFormat("SELECT nama, skor FROM leaderboard ORDER BY %s LIMIT %i;",
                     orderByClause.c_str(), limit);
  } else {  // No limit (get all, useful for search before filtering)
    sql = TextFormat("SELECT nama, skor FROM leaderboard ORDER BY %s;",
                     orderByClause.c_str());
  }

  int rc = sqlite3_exec(db, sql.c_str(), callback, &scores, &zErrMsg);
  if (rc != SQLITE_OK) {
    TraceLog(
        LOG_ERROR,
        TextFormat("SQLITE: Gagal mendapatkan entri leaderboard: %s", zErrMsg));
    sqlite3_free(zErrMsg);
  }
  return scores;
}

std::vector<EntriLeaderboard> SearchScores(const std::string& searchTerm,
                                           LeaderboardSortOrder order) {
  std::vector<EntriLeaderboard> scores;
  if (!db) {
    TraceLog(LOG_WARNING,
             "SQLITE: Tidak ada database yang terbuka untuk pencarian skor.");
    return scores;
  }
  char* zErrMsg = nullptr;

  std::string escapedSearchTerm = searchTerm;
  size_t pos = escapedSearchTerm.find('\'');
  while (pos != std::string::npos) {
    escapedSearchTerm.replace(pos, 1, "''");
    pos = escapedSearchTerm.find('\'', pos + 2);
  }

  std::string orderByClause;
  if (order == SORT_BY_SCORE_DESC) {
    orderByClause = "skor DESC";
  } else {  // SORT_BY_NAME_ASC
    orderByClause = "nama ASC";
  }

  // Note: LIMIT is not applied here, so search results show all matches in the
  // chosen order.
  std::string sql = TextFormat(
      "SELECT nama, skor FROM leaderboard WHERE nama LIKE '%%%s%%' ORDER BY "
      "%s;",
      escapedSearchTerm.c_str(), orderByClause.c_str());
  int rc = sqlite3_exec(db, sql.c_str(), callback, &scores, &zErrMsg);
  if (rc != SQLITE_OK) {
    TraceLog(LOG_ERROR, TextFormat("SQLITE: Gagal mencari skor: %s", zErrMsg));
    sqlite3_free(zErrMsg);
  }
  return scores;
}