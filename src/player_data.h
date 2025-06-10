#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <raylib.h>

#include <algorithm>  // Untuk std::sort (akan kurang digunakan setelah SQLite)
#include <string>
#include <vector>

// Struktur Entri Leaderboard
struct EntriLeaderboard {
  std::string nama;
  int skor;

  // Untuk pengurutan (skor menurun)
  // Masih relevan jika kita ingin mengurutkan hasil pencarian atau daftar
  // singkat di memori
  bool operator>(const EntriLeaderboard& other) const {
    return skor > other.skor;
  }
};

// Variabel global
extern std::string g_namaPemain;
extern char g_bufferNama[32];
extern std::vector<EntriLeaderboard>
    g_leaderboard;  // Akan menjadi cache atau hasil query
// FILE_LEADERBOARD tidak lagi diperlukan karena menggunakan database
// extern const std::string FILE_LEADERBOARD;

// Prototipe Fungsi
void InitPlayerData();  // Akan memanggil InitDatabase
// SaveLeaderboard dan LoadLeaderboard lama akan diganti atau dimodifikasi
// void SaveLeaderboard();
// void LoadLeaderboard();
void AddScoreToLeaderboard(const std::string& nama,
                           int skor);  // Akan memanggil InsertScore
void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight);
// BARU: Fungsi untuk memuat leaderboard dari database (akan menggantikan
// LoadLeaderboard lama)
void UpdateLeaderboardDisplay(int limit = 10);
// BARU: Fungsi untuk mencari dan menampilkan hasil
void SearchAndDisplayLeaderboard(const std::string& searchTerm);

#endif  // PLAYER_DATA_H