#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

// OLD: #include "player_data.h" (REMOVE THIS LINE)

// NEW: Forward declaration for EntriLeaderboard
// We only need to know it's a type here, not its full definition.
struct EntriLeaderboard;  // Forward declare EntriLeaderboard

// Enum for sort order
enum LeaderboardSortOrder {
  SORT_BY_SCORE_DESC,  // Default: Score descending
  SORT_BY_NAME_ASC     // Name ascending
};

bool InitDatabase(const std::string& dbPath);
void CloseDatabase();
void InsertScore(const std::string& nama, int skor);
std::vector<EntriLeaderboard> GetLeaderboardEntries(
    int limit = 10, LeaderboardSortOrder order = SORT_BY_SCORE_DESC);
std::vector<EntriLeaderboard> SearchScores(
    const std::string& searchTerm,
    LeaderboardSortOrder order = SORT_BY_SCORE_DESC);

#endif  // DATABASE_H