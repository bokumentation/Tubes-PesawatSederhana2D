#include "player_data.h"

#include <raylib.h>

#include <cstring>

#include "database.h"  // Include the database header for functions and enum

std::string g_namaPemain = "";
char g_bufferNama[32] = "\0";
std::vector<EntriLeaderboard> g_leaderboard;

void InitPlayerData() {
  InitDatabase("leaderboard.db");
  UpdateLeaderboardDisplay(10, SORT_BY_SCORE_DESC);  // Initial load by score
}

void AddScoreToLeaderboard(const std::string& nama, int skor) {
  InsertScore(nama, skor);
  UpdateLeaderboardDisplay(
      10, SORT_BY_SCORE_DESC);  // Refresh with top scores after adding
}

/**
 * @brief Memperbarui data leaderboard yang ditampilkan dari database.
 *
 * Fungsi ini memuat entri leaderboard dari database berdasarkan batas
 * dan urutan pengurutan yang ditentukan, lalu menyimpannya ke g_leaderboard.
 *
 * @param limit Jumlah entri yang akan dimuat.
 * @param order Urutan pengurutan (SORT_BY_SCORE_DESC atau SORT_BY_NAME_ASC).
 */
void UpdateLeaderboardDisplay(int limit, LeaderboardSortOrder order) {
  g_leaderboard = GetLeaderboardEntries(limit, order);
  TraceLog(LOG_INFO, "LEADERBOARD: Data leaderboard diperbarui dari database.");
}

/**
 * @brief Mencari dan menampilkan entri leaderboard berdasarkan istilah
 * pencarian.
 *
 * Fungsi ini memuat entri dari database yang namanya cocok dengan 'searchTerm'
 * (sebagian atau seluruhnya) dan mengurutkan hasilnya.
 *
 * @param searchTerm String yang akan digunakan untuk mencari nama pemain.
 * @param order Urutan pengurutan hasil.
 */
void SearchAndDisplayLeaderboard(const std::string& searchTerm,
                                 LeaderboardSortOrder order) {
  g_leaderboard = SearchScores(searchTerm, order);
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