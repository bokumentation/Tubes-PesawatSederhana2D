#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <raylib.h>

#include <algorithm>
#include <string>
#include <vector>

// This is correct: include the full definition of database.h
#include "database.h"

/**
 * @brief Struktur data untuk merepresentasikan satu entri di leaderboard.
 *
 * Berisi nama pemain (string) dan skor yang dicapai (integer).
 */
struct EntriLeaderboard {
  std::string nama;  // Nama Pemain
  int skor;          // Skor Pemain

  /**
   * @brief Operator perbandingan overload untuk mengurutkan EntriLeaderboard
   * berdasarkan skor.
   *
   * Digunakan untuk mengurutkan entri dari skor tertinggi ke terendah secara
   * default.
   * @param other EntriLeaderboard lain untuk dibandingkan.
   * @return true jika skor entri ini lebih besar dari skor entri lain, false
   * jika sebaliknya.
   */
  bool operator>(const EntriLeaderboard& other) const {
    return skor > other.skor;
  }
};

extern std::string g_namaPemain;  // Nama pemain saat ini (global).
extern char g_bufferNama[32];     // Buffer untuk input nama pemain.
extern std::vector<EntriLeaderboard>
    g_leaderboard;  // Vektor global yang menyimpan entri leaderboard yang
                    // sedang ditampilkan.

/**
 * @brief Menginisialisasi data pemain dan memuat leaderboard awal.
 *
 * Fungsi ini memanggil InitDatabase() untuk membuka koneksi ke database
 * dan kemudian memuat 10 skor teratas secara default untuk tampilan awal
 * leaderboard.
 */
void InitPlayerData();

/**
 * @brief Menambahkan skor pemain ke leaderboard.
 *
 * Fungsi ini menyisipkan nama dan skor pemain ke database, lalu memperbarui
 * tampilan leaderboard global dengan 10 skor teratas yang baru.
 *
 * @param nama Nama pemain yang akan ditambahkan.
 * @param skor Skor pemain yang akan ditambahkan.
 */
void AddScoreToLeaderboard(const std::string& nama, int skor);

/**
 * @brief Menggambar leaderboard di layar.
 *
 * Fungsi ini mengiterasi melalui vektor g_leaderboard global dan
 * menggambar setiap entri ke layar game. Jika leaderboard kosong,
 * pesan "Belum ada skor!" akan ditampilkan.
 *
 * @param font Font Raylib yang akan digunakan untuk menggambar teks.
 * @param fontSize Ukuran font untuk teks leaderboard.
 * @param textColor Warna teks leaderboard.
 * @param screenWidth Lebar layar game.
 * @param screenHeight Tinggi layar game.
 */
void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight);

/**
 * @brief Memperbarui data leaderboard yang ditampilkan dari database.
 *
 * Fungsi ini memuat entri leaderboard dari database berdasarkan batas
 * dan urutan pengurutan yang ditentukan, lalu menyimpannya ke g_leaderboard.
 *
 * @param limit Jumlah entri yang akan dimuat (default 10).
 * @param order Urutan pengurutan (default SORT_BY_SCORE_DESC).
 */
void UpdateLeaderboardDisplay(int limit = 10,
                              LeaderboardSortOrder order = SORT_BY_SCORE_DESC);

/**
 * @brief Mencari dan menampilkan entri leaderboard berdasarkan istilah
 * pencarian.
 *
 * Fungsi ini memuat entri dari database yang namanya cocok dengan 'searchTerm'
 * (sebagian atau seluruhnya) dan mengurutkan hasilnya.
 *
 * @param searchTerm String yang akan digunakan untuk mencari nama pemain.
 * @param order Urutan pengurutan hasil (default SORT_BY_SCORE_DESC).
 */
void SearchAndDisplayLeaderboard(
    const std::string& searchTerm,
    LeaderboardSortOrder order = SORT_BY_SCORE_DESC);

#endif  // PLAYER_DATA_H