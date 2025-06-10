#include "player_data.h"

#include <raylib.h>  // Untuk TraceLog, Font, Color, dll.

#include <cstring>  // Untuk memset
#include <fstream>  // Tidak lagi diperlukan untuk leaderboard.txt
#include <sstream>  // Tidak lagi diperlukan untuk leaderboard.txt

#include "database.h"  // BARU: Sertakan header database kita

// Definisi variabel global (harus didefinisikan sekali dalam file .cpp)
std::string g_namaPemain = "";
char g_bufferNama[32] = "\0";
std::vector<EntriLeaderboard>
    g_leaderboard;  // Ini sekarang akan menjadi cache dari hasil query database
// FILE_LEADERBOARD tidak lagi digunakan
// const std::string FILE_LEADERBOARD = "leaderboard.txt";

void InitPlayerData() {
  // BARU: Inisialisasi database di sini
  InitDatabase("leaderboard.db");  // Nama file database Anda
  // Muat 10 skor teratas saat game dimulai atau untuk tampilan awal leaderboard
  UpdateLeaderboardDisplay(10);
}

// SaveLeaderboard dan LoadLeaderboard yang lama tidak lagi diperlukan
// karena digantikan oleh fungsi database
/*
void SaveLeaderboard() {
  // Logic ini dipindahkan ke InsertScore dalam database.cpp
}

void LoadLeaderboard() {
  // Logic ini digantikan oleh GetTopScores dalam database.cpp dan
UpdateLeaderboardDisplay di bawah
}
*/

void AddScoreToLeaderboard(const std::string& nama, int skor) {
  // BARU: Sisipkan skor langsung ke database
  InsertScore(nama, skor);
  // Perbarui tampilan leaderboard setelah skor baru ditambahkan
  UpdateLeaderboardDisplay(10);  // Muat ulang 10 skor teratas
}

// BARU: Fungsi untuk memperbarui g_leaderboard dari database
void UpdateLeaderboardDisplay(int limit) {
  g_leaderboard = GetTopScores(limit);
  // Kita tidak perlu mengurutkan di sini lagi, karena GetTopScores sudah
  // mengurutkan dari DB
  TraceLog(LOG_INFO, "LEADERBOARD: Data leaderboard diperbarui dari database.");
}

// BARU: Fungsi untuk mencari dan menampilkan hasil
void SearchAndDisplayLeaderboard(const std::string& searchTerm) {
  g_leaderboard = SearchScores(searchTerm);
  // Tidak perlu mengurutkan lagi karena SearchScores sudah mengurutkan
  TraceLog(LOG_INFO,
           TextFormat(
               "LEADERBOARD: Hasil pencarian untuk '%s' dimuat dari database.",
               searchTerm.c_str()));
}

void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight) {
  int startY = 100;
  int spacing = 30;

  if (g_leaderboard.empty()) {
    const char* noEntries = "Belum ada skor!";
    int noEntriesWidth = MeasureTextEx(font, noEntries, fontSize, 2).x;
    DrawTextEx(font, noEntries,
               Vector2{(float)screenWidth / 2 - noEntriesWidth / 2,
                       (float)screenHeight / 2},
               fontSize, 2, textColor);
    return;
  }

  for (size_t i = 0; i < g_leaderboard.size(); ++i) {
    const auto& entry = g_leaderboard[i];
    std::string entryText =
        TextFormat("%i. %s: %i", i + 1, entry.nama.c_str(), entry.skor);
    int entryWidth = MeasureTextEx(font, entryText.c_str(), fontSize, 2).x;
    DrawTextEx(font, entryText.c_str(),
               Vector2{(float)screenWidth / 2 - entryWidth / 2,
                       (float)startY + (float)i * spacing},
               fontSize, 2, textColor);
  }
}