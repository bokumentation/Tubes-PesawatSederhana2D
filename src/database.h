#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

// Forward declaration of EntriLeaderboard from player_data.h
// We need to define it here if we're not including player_data.h directly
// in database.h, but still want to use it in function signatures.
// Alternatively, include "player_data.h" here if EntriLeaderboard struct is
// defined there. For simplicity in this example, let's include it.
#include "player_data.h"  // Assuming EntriLeaderboard is defined here

// Function prototypes for database operations
bool InitDatabase(const std::string& dbPath);
void CloseDatabase();
void InsertScore(const std::string& nama, int skor);
std::vector<EntriLeaderboard> GetTopScores(int limit = 10);
std::vector<EntriLeaderboard> SearchScores(const std::string& searchTerm);

#endif  // DATABASE_H