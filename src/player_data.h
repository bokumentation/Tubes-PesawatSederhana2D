#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <raylib.h>

#include <algorithm>  // Untuk std::sort
#include <string>
#include <vector>

// Struktur Entri Leaderboard
struct EntriLeaderboard {
  std::string nama;
  int skor;

  // Untuk pengurutan (skor menurun)
  bool operator>(const EntriLeaderboard& other) const {
    return skor > other.skor;
  }
};

// Variabel global (atau Anda bisa melewatkannya)
extern std::string g_namaPemain;  // g_playerName -> g_namaPemain
extern char g_bufferNama[32];     // g_nameBuffer -> g_bufferNama
extern std::vector<EntriLeaderboard> g_leaderboard;
extern const std::string
    FILE_LEADERBOARD;  // LEADERBOARD_FILE -> FILE_LEADERBOARD

// Prototipe Fungsi
void InitPlayerData();  // Untuk pengaturan awal seperti memuat leaderboard
void SaveLeaderboard();
void LoadLeaderboard();
void AddScoreToLeaderboard(const std::string& nama,
                           int skor);  // name -> nama, score -> skor
void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight);

#endif  // PLAYER_DATA_H