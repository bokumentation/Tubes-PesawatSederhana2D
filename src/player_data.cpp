#include "player_data.h"

#include <raylib.h>  // For TraceLog, Font, Color, etc.

#include <cstring>  // For memset
#include <fstream>
#include <sstream>

// Global variable definitions (must be defined once in a .cpp file)
std::string g_playerName = "";
char g_nameBuffer[32] = "\0";
std::vector<LeaderboardEntry> g_leaderboard;
const std::string LEADERBOARD_FILE = "leaderboard.txt";

void InitPlayerData() { LoadLeaderboard(); }

void SaveLeaderboard() {
  std::ofstream outFile(LEADERBOARD_FILE);
  if (outFile.is_open()) {
    for (const auto& entry : g_leaderboard) {
      outFile << entry.name << " " << entry.score << "\n";
    }
    outFile.close();
    TraceLog(LOG_INFO, "LEADERBOARD: Leaderboard saved.");
  } else {
    TraceLog(LOG_WARNING, "LEADERBOARD: Could not open file for writing.");
  }
}

void LoadLeaderboard() {
  g_leaderboard.clear();
  std::ifstream inFile(LEADERBOARD_FILE);
  if (inFile.is_open()) {
    std::string line;
    while (std::getline(inFile, line)) {
      std::stringstream ss(line);
      LeaderboardEntry entry;
      ss >> entry.name >> entry.score;
      g_leaderboard.push_back(entry);
    }
    inFile.close();
    std::sort(g_leaderboard.begin(), g_leaderboard.end(),
              std::greater<LeaderboardEntry>());
    TraceLog(LOG_INFO, "LEADERBOARD: Leaderboard loaded.");
  } else {
    TraceLog(LOG_INFO,
             "LEADERBOARD: No existing leaderboard file found. New one will be "
             "created on save.");
  }
}

void AddScoreToLeaderboard(const std::string& name, int score) {
  LeaderboardEntry newEntry = {name, score};
  g_leaderboard.push_back(newEntry);
  std::sort(g_leaderboard.begin(), g_leaderboard.end(),
            std::greater<LeaderboardEntry>());
  if (g_leaderboard.size() > 10) g_leaderboard.resize(10);  // Keep top 10
  SaveLeaderboard();  // Save immediately after adding/sorting
}

void DrawLeaderboard(Font font, int fontSize, Color textColor, int screenWidth,
                     int screenHeight) {
  int startY = 100;
  int spacing = 30;

  if (g_leaderboard.empty()) {
    const char* noEntries = "No scores yet!";
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
        TextFormat("%i. %s: %i", i + 1, entry.name.c_str(), entry.score);
    int entryWidth = MeasureTextEx(font, entryText.c_str(), fontSize, 2).x;
    DrawTextEx(font, entryText.c_str(),
               Vector2{(float)screenWidth / 2 - entryWidth / 2,
                       (float)startY + (float)i * spacing},
               fontSize, 2, textColor);
  }
}