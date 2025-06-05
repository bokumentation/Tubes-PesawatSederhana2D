#ifndef PLAYER_DATA_H
#define PLAYER_DATA_H

#include <raylib.h>

#include <algorithm>  // For std::sort
#include <string>
#include <vector>

// Leaderboard Entry Structure
struct LeaderboardEntry {
  std::string name;
  int score;

  // For sorting (descending score)
  bool operator>(const LeaderboardEntry& other) const {
    return score > other.score;
  }
};

// Global variables (or you can pass them around)
extern std::string g_playerName;
extern char g_nameBuffer[32];
extern std::vector<LeaderboardEntry> g_leaderboard;
extern const std::string LEADERBOARD_FILE;

// Function Prototypes
void InitPlayerData();  // For initial setup like loading leaderboard
void SaveLeaderboard();
void LoadLeaderboard();
void AddScoreToLeaderboard(const std::string& name, int score);
void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight);

#endif  // PLAYER_DATA_H