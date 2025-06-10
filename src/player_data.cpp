#include "player_data.h"

#include <raylib.h>  // Untuk TraceLog, Font, Color, dll.

#include <cstring>  // Untuk memset
#include <fstream>
#include <sstream>

// Definisi variabel global (harus didefinisikan sekali dalam file .cpp)
std::string g_namaPemain = "";  // g_playerName -> g_namaPemain
char g_bufferNama[32] = "\0";   // g_nameBuffer -> g_bufferNama
std::vector<EntriLeaderboard> g_leaderboard;
const std::string FILE_LEADERBOARD =
    "leaderboard.txt";  // LEADERBOARD_FILE -> FILE_LEADERBOARD

void InitPlayerData() { LoadLeaderboard(); }

void SaveLeaderboard() {
  std::ofstream outFile(FILE_LEADERBOARD);
  if (outFile.is_open()) {
    for (const auto& entry : g_leaderboard) {
      outFile << entry.nama << " " << entry.skor << "\n";
    }
    outFile.close();
    TraceLog(LOG_INFO, "LEADERBOARD: Leaderboard disimpan.");
  } else {
    TraceLog(LOG_WARNING,
             "LEADERBOARD: Tidak dapat membuka file untuk menulis.");
  }
}

void LoadLeaderboard() {
  g_leaderboard.clear();
  std::ifstream inFile(FILE_LEADERBOARD);
  if (inFile.is_open()) {
    std::string line;
    while (std::getline(inFile, line)) {
      std::stringstream ss(line);
      EntriLeaderboard entry;
      ss >> entry.nama >> entry.skor;
      g_leaderboard.push_back(entry);
    }
    inFile.close();
    std::sort(g_leaderboard.begin(), g_leaderboard.end(),
              std::greater<EntriLeaderboard>());
    TraceLog(LOG_INFO, "LEADERBOARD: Leaderboard dimuat.");
  } else {
    TraceLog(LOG_INFO,
             "LEADERBOARD: Tidak ada file leaderboard yang ada. Yang baru akan "
             "dibuat saat disimpan.");
  }
}

void AddScoreToLeaderboard(const std::string& nama, int skor) {
  EntriLeaderboard newEntry = {nama, skor};
  g_leaderboard.push_back(newEntry);
  std::sort(g_leaderboard.begin(), g_leaderboard.end(),
            std::greater<EntriLeaderboard>());
  if (g_leaderboard.size() > 10)
    g_leaderboard.resize(10);  // Pertahankan 10 teratas
  SaveLeaderboard();           // Simpan segera setelah menambah/mengurutkan
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