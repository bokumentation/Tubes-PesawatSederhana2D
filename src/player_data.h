#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <raylib.h>

#include <algorithm>
#include <string>
#include <vector>

// This is correct: include the full definition of database.h
#include "database.h"

// Struktur EntriLeaderboard (must be fully defined here)
struct EntriLeaderboard {
  std::string nama;
  int skor;

  bool operator>(const EntriLeaderboard& other) const {
    return skor > other.skor;
  }
};

extern std::string g_namaPemain;
extern char g_bufferNama[32];
extern std::vector<EntriLeaderboard> g_leaderboard;

void InitPlayerData();
void AddScoreToLeaderboard(const std::string& nama, int skor);
void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight);

void UpdateLeaderboardDisplay(int limit = 10,
                              LeaderboardSortOrder order = SORT_BY_SCORE_DESC);
void SearchAndDisplayLeaderboard(
    const std::string& searchTerm,
    LeaderboardSortOrder order = SORT_BY_SCORE_DESC);

#endif  // PLAYER_DATA_H