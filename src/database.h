/**
 * @file database.h
 * @brief fungsi-fungsi untuk interaksi dengan database SQLite.
 *
 * File ini berisi definisi fungsi untuk mengelola koneksi database,
 * menyisipkan skor, serta mengambil dan mencari entri leaderboard.
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

// JANGAN DIOTAK-ATIK!! - ibe
// BARU: Forward declaration for EntriLeaderboard
struct EntriLeaderboard;  // Forward declare EntriLeaderboard

// Enum for sort order
enum LeaderboardSortOrder {
  SORT_BY_SCORE_DESC,  // Default: Score descending
  SORT_BY_NAME_ASC     // Name ascending
};

/**
 * @brief Menginisialisasi koneksi ke database SQLite dan membuat tabel
 * leaderboard jika belum ada.
 *
 * Fungsi ini mencoba membuka atau membuat file database pada jalur yang
 * ditentukan. Jika berhasil, ia akan memastikan tabel 'leaderboard' ada dengan
 * skema: (id INTEGER PRIMARY KEY AUTOINCREMENT, nama TEXT NOT NULL, skor
 * INTEGER NOT NULL).
 *
 * @param dbPath Jalur ke file database SQLite.
 * @return true jika database berhasil dibuka dan tabel siap; false jika terjadi
 * error.
 */
bool InitDatabase(const std::string& dbPath);

/**
 * @brief Menutup koneksi database SQLite yang terbuka.
 *
 * Jika ada koneksi database yang aktif, fungsi ini akan menutupnya dan
 * mereset pointer database ke nullptr.
 */
void CloseDatabase();

/**
 * @brief Menyisipkan entri skor baru ke dalam tabel leaderboard.
 *
 * Fungsi ini memasukkan nama pemain dan skornya ke dalam database.
 * Jika tidak ada koneksi database yang terbuka, operasi akan diabaikan.
 *
 * @param nama Nama pemain yang akan disisipkan.
 * @param skor Skor pemain yang akan disisipkan.
 */
void InsertScore(const std::string& nama, int skor);

/**
 * @brief Mengambil daftar entri leaderboard dari database.
 *
 * Fungsi ini mengambil data nama dan skor dari tabel 'leaderboard'.
 * Hasil dapat dibatasi jumlahnya dan diurutkan berdasarkan skor (menurun)
 * atau nama (menaik).
 *
 * @param limit Jumlah entri yang ingin diambil. Jika 0 atau negatif, semua
 * entri akan diambil.
 * @param order Urutan pengurutan hasil (SORT_BY_SCORE_DESC atau
 * SORT_BY_NAME_ASC).
 * @return std::vector<EntriLeaderboard> berisi entri-entri leaderboard yang
 * diambil.
 */
std::vector<EntriLeaderboard> GetLeaderboardEntries(
    int limit = 10, LeaderboardSortOrder order = SORT_BY_SCORE_DESC);

/**
 * @brief Mencari entri leaderboard berdasarkan nama dan mengurutkan hasilnya.
 *
 * Fungsi ini mencari entri di tabel 'leaderboard' di mana kolom 'nama'
 * mengandung string pencarian yang diberikan (case-insensitive).
 * Hasil dapat diurutkan berdasarkan skor (menurun) atau nama (menaik).
 *
 * @param searchTerm String yang akan dicari di kolom nama.
 * @param order Urutan pengurutan hasil (SORT_BY_SCORE_DESC atau
 * SORT_BY_NAME_ASC).
 * @return std::vector<EntriLeaderboard> berisi entri-entri yang cocok dengan
 * pencarian.
 */
std::vector<EntriLeaderboard> SearchScores(
    const std::string& searchTerm,
    LeaderboardSortOrder order = SORT_BY_SCORE_DESC);

#endif  // DATABASE_H