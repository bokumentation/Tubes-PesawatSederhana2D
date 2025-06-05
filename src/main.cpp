/*
STANDARISASI
Gunakan Google C++ Style Guide
Spaces: 2

CARA BUILD PROGRAM

cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
*/

#include <raylib.h>  // Mengimpor library raylib

#include <algorithm>  // For std::remove_if
#include <cstring>
#include <random>  // For random enemy spawning
#include <vector>  // For storing bullets and enemies

#include "player_data.h"  // NEW: Include our player data and leaderboard header
#include "window.h"       // Header for InisialisasiGameWindow()

// Removed: <string>, <fstream>, <sstream>, <map> as they are now in
// player_data.cpp

// --- Game States ---
enum GameState { TITLE_SCREEN, NAME_INPUT, GAMEPLAY, GAME_OVER, LEADERBOARD };

// --- Player Structure ---
struct Player {
  Rectangle rect;
  Color color;
};

// --- Bullet Structure ---
struct Bullet {
  Rectangle rect;
  float speed;
  Color color;
  bool active;
};

// --- Enemy Structure ---
struct Enemy {
  Rectangle rect;
  Color color;
  float speed;
  int health;
  bool active;
};

// --- Global Game State Variables ---
GameState currentGameState = TITLE_SCREEN;
int currentScore = 0;  // Renamed to avoid conflict with g_score in player_data
int currentLives = 1;  // Renamed for clarity

// For blinking cursor (kept in main for simplicity as it's UI specific)
int framesCounter = 0;

// --- Gameplay-specific variables (not part of player_data) ---
std::vector<Bullet> playerBullets;
std::vector<Enemy> enemies;

float scrollingBack = 0.0f;
float scrollingMid = 0.0f;
float scrollingFore = 0.0f;

Texture2D backgroundTex;  // Declared here, loaded in main
Texture2D midgroundTex;
Texture2D foregroundTex;

// Random number generation for enemy spawning
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> enemySpawnY(0, SCREEN_HEIGHT - 50);
std::uniform_real_distribution<> enemySpeed(100.0f, 250.0f);

// Timer for enemy spawning
double lastEnemySpawnTime = 0.0;
float enemySpawnInterval = 2.0f;

// -- New Variables for player's firing rate

// Time since the last bullet was fired
double lastShotTime = 0.0;
// Seconds between shots (e.g., 0.15s for rapid fire)
float playerFireRate = 0.15f;

// --- Function Prototypes (for main.cpp's responsibilities) ---
void ResetGameplayElements();  // Resets only game-specific elements

int main() {
  //                                //
  // 1. INISIALISASI                //
  //                                //

  InisialisasiGameWindow();

  Font customFont =
      LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 20, 0, 0);
  SetTextureFilter(customFont.texture, TEXTURE_FILTER_BILINEAR);

  Font titleFont =
      LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 30, 0, 0);
  SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);

  // Load textures (better to do once)
  backgroundTex = LoadTexture("assets/backgrounds/space_bg.png");
  midgroundTex = LoadTexture("assets/backgrounds/stars_mid.png");
  foregroundTex = LoadTexture("assets/backgrounds/stars_fore.png");

  // Initialize player (initial state, will be reset on game start)
  Player player;  // Player object remains in main or game manager
  player.rect = {(float)SCREEN_WIDTH / 2 - 25, (float)SCREEN_HEIGHT / 2 - 25,
                 50, 50};
  player.color = BLUE;

  // Initialize player data and load leaderboard
  InitPlayerData();  // Call the function from player_data.cpp

  HideCursor();
  SetTargetFPS(60);

  //                                //
  // 2. GAME LOOP                   //
  //                                //

  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();
    framesCounter++;

    // --- State-based Logic ---
    switch (currentGameState) {
      case TITLE_SCREEN: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          currentGameState = NAME_INPUT;
          g_playerName = "";  // Clear name for new input
          memset(g_nameBuffer, 0, sizeof(g_nameBuffer));  // Clear buffer
        }
      } break;

      case NAME_INPUT: {
        int key = GetCharPressed();
        while (key > 0) {
          if ((key >= 32) && (key <= 125) && (strlen(g_nameBuffer) < 31)) {
            g_nameBuffer[strlen(g_nameBuffer)] =
                (char)key;  // Add character to buffer
          }
          key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
          if (strlen(g_nameBuffer) > 0) {
            g_nameBuffer[strlen(g_nameBuffer) - 1] =
                '\0';  // Remove last character
          }
        }

        if (IsKeyPressed(KEY_ENTER)) {
          if (strlen(g_nameBuffer) > 0) {
            g_playerName = std::string(g_nameBuffer);
            ResetGameplayElements();  // Reset gameplay specific elements
            currentGameState = GAMEPLAY;
          }
        }
      } break;

      case GAMEPLAY: {
        Vector2 mousePosisi = GetMousePosition();
        player.rect.x = mousePosisi.x - player.rect.width / 2;
        player.rect.y = mousePosisi.y - player.rect.height / 2;

        if (player.rect.x < 0) {
          player.rect.x = 0;
        };
        if (player.rect.x + player.rect.width > SCREEN_WIDTH) {
          player.rect.x = SCREEN_WIDTH - player.rect.width;
        };
        if (player.rect.y < 0) {
          player.rect.y = 0;
        };
        if (player.rect.y + player.rect.height > SCREEN_HEIGHT) {
          player.rect.y = SCREEN_HEIGHT - player.rect.height;
        };

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
            (GetTime() - lastShotTime >= playerFireRate)) {
          Bullet newBullet;
          newBullet.rect = {player.rect.x + player.rect.width,
                            player.rect.y + player.rect.height / 2 - 5, 20, 10};
          newBullet.speed = 1000;
          newBullet.color = RED;
          newBullet.active = true;
          playerBullets.push_back(newBullet);

          lastShotTime = GetTime();
        }

        for (auto& bullet : playerBullets) {
          if (bullet.active) {
            bullet.rect.x += bullet.speed * deltaTime;
            if (bullet.rect.x > SCREEN_WIDTH) {
              bullet.active = false;
            }
          }
        }
        playerBullets.erase(
            std::remove_if(playerBullets.begin(), playerBullets.end(),
                           [](const Bullet& b) { return !b.active; }),
            playerBullets.end());

        if (GetTime() - lastEnemySpawnTime > enemySpawnInterval) {
          Enemy newEnemy;
          newEnemy.rect = {(float)SCREEN_WIDTH, (float)enemySpawnY(gen), 50,
                           50};
          newEnemy.color = LIME;
          newEnemy.speed = enemySpeed(gen);
          newEnemy.health = 1;
          newEnemy.active = true;
          enemies.push_back(newEnemy);
          lastEnemySpawnTime = GetTime();
          enemySpawnInterval = GetRandomValue(50, 150) / 100.0f;
        }

        for (auto& enemy : enemies) {
          if (enemy.active) {
            enemy.rect.x -= enemy.speed * deltaTime;
            if (enemy.rect.x + enemy.rect.width < 0) {
              enemy.active = false;
            }

            if (CheckCollisionRecs(player.rect, enemy.rect)) {
              currentLives--;  // Use currentLives
              enemy.active = false;
              if (currentLives <= 0) {  // Check currentLives
                AddScoreToLeaderboard(
                    g_playerName,
                    currentScore);  // Use g_playerName and currentScore
                currentGameState = GAME_OVER;
              }
            }
          }
        }
        enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                     [](const Enemy& e) { return !e.active; }),
                      enemies.end());

        for (auto& bullet : playerBullets) {
          if (bullet.active) {
            for (auto& enemy : enemies) {
              if (enemy.active && CheckCollisionRecs(bullet.rect, enemy.rect)) {
                enemy.health--;
                bullet.active = false;
                if (enemy.health <= 0) {
                  enemy.active = false;
                  currentScore += 10;  // Use currentScore
                }
                break;
              }
            }
          }
        }

        scrollingBack -= 10.0f * deltaTime;
        scrollingMid -= 20.0f * deltaTime;
        scrollingFore -= 30.0f * deltaTime;

        if (scrollingBack <= -backgroundTex.width) scrollingBack = 0;
        if (scrollingMid <= -midgroundTex.width) scrollingMid = 0;
        if (scrollingFore <= -foregroundTex.width) scrollingFore = 0;
      } break;

      case GAME_OVER: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          currentGameState = LEADERBOARD;
        }
      } break;

      case LEADERBOARD: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          currentGameState = TITLE_SCREEN;
        }
      } break;
    }

    // --- Drawing ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background regardless of state
    DrawTextureEx(backgroundTex, (Vector2){scrollingBack, 0}, 0.0f, 1.0f,
                  WHITE);
    DrawTextureEx(backgroundTex,
                  (Vector2){backgroundTex.width + scrollingBack, 0}, 0.0f, 1.0f,
                  WHITE);
    DrawTextureEx(midgroundTex, (Vector2){scrollingMid, 0}, 0.0f, 1.0f, WHITE);
    DrawTextureEx(midgroundTex, (Vector2){midgroundTex.width + scrollingMid, 0},
                  0.0f, 1.0f, WHITE);
    DrawTextureEx(foregroundTex, (Vector2){scrollingFore, 0}, 0.0f, 1.0f,
                  WHITE);

    switch (currentGameState) {
      case TITLE_SCREEN: {
        const char* titleText = "PESAWAT SEDERHANA 2D";
        const char* startText = "Press ENTER or CLICK to Start";
        int titleWidth =
            MeasureTextEx(titleFont, titleText, customFont.baseSize * 1.5, 2).x;
        int startWidth =
            MeasureTextEx(customFont, startText, customFont.baseSize, 2).x;

        DrawTextEx(titleFont, titleText,
                   Vector2{(float)SCREEN_WIDTH / 2 - titleWidth / 2,
                           (float)SCREEN_HEIGHT / 2 - 50},
                   customFont.baseSize * 1.5, 2, BLACK);
        DrawTextEx(customFont, startText,
                   Vector2{(float)SCREEN_WIDTH / 2 - startWidth / 2,
                           (float)SCREEN_HEIGHT / 2 + 20},
                   customFont.baseSize, 2, BLACK);
      } break;

      case NAME_INPUT: {
        const char* promptText = "ENTER YOUR NAME:";
        int promptWidth =
            MeasureTextEx(customFont, promptText, customFont.baseSize, 2).x;
        int nameBufferWidth =
            MeasureTextEx(customFont, g_nameBuffer, customFont.baseSize, 2)
                .x;  // Use g_nameBuffer

        DrawTextEx(customFont, promptText,
                   Vector2{(float)SCREEN_WIDTH / 2 - promptWidth / 2,
                           (float)SCREEN_HEIGHT / 2 - 50},
                   customFont.baseSize, 2, BLACK);
        DrawRectangleLines(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 40,
                           BLACK);
        DrawTextEx(customFont, g_nameBuffer,
                   Vector2{(float)SCREEN_WIDTH / 2 - nameBufferWidth / 2,
                           (float)SCREEN_HEIGHT / 2 + 10},
                   customFont.baseSize, 2, BLACK);

        if (((framesCounter / 30) % 2) == 0) {
          DrawTextEx(customFont, "_",
                     Vector2{(float)SCREEN_WIDTH / 2 - nameBufferWidth / 2 +
                                 nameBufferWidth,
                             (float)SCREEN_HEIGHT / 2 + 10},
                     customFont.baseSize, 2, BLACK);
        }
      } break;

      case GAMEPLAY: {
        DrawRectangleRec(player.rect, player.color);

        for (const auto& bullet : playerBullets) {
          if (bullet.active) {
            DrawRectangleRec(bullet.rect, bullet.color);
          }
        }

        for (const auto& enemy : enemies) {
          if (enemy.active) {
            DrawRectangleRec(enemy.rect, enemy.color);
          }
        }

        DrawTextEx(customFont, TextFormat("Score: %i", currentScore),
                   Vector2{10, 10},  // Use currentScore
                   customFont.baseSize, 2, BLACK);
        DrawTextEx(customFont, TextFormat("Lives: %i", currentLives),
                   Vector2{10, 40},  // Use currentLives
                   customFont.baseSize, 2, BLACK);
        DrawFPS(SCREEN_WIDTH - 100, 10);
      } break;

      case GAME_OVER: {
        const char* gameOverText = "GAME OVER!";
        const char* scoreText =
            TextFormat("YOUR SCORE: %i", currentScore);  // Use currentScore
        const char* continueText = "Press ENTER or CLICK to see Leaderboard";
        int gameOverWidth = MeasureTextEx(customFont, gameOverText,
                                          customFont.baseSize * 1.5, 2)
                                .x;
        int scoreWidth =
            MeasureTextEx(customFont, scoreText, customFont.baseSize, 2).x;
        int continueWidth = MeasureTextEx(customFont, continueText,
                                          customFont.baseSize * 0.8, 2)
                                .x;

        DrawTextEx(titleFont, gameOverText,
                   Vector2{(float)SCREEN_WIDTH / 2 - gameOverWidth / 2,
                           (float)SCREEN_HEIGHT / 2 - 70},
                   customFont.baseSize * 1.5, 2, BLACK);
        DrawTextEx(customFont, scoreText,
                   Vector2{(float)SCREEN_WIDTH / 2 - scoreWidth / 2,
                           (float)SCREEN_HEIGHT / 2},
                   customFont.baseSize, 2, BLACK);
        DrawTextEx(customFont, continueText,
                   Vector2{(float)SCREEN_WIDTH / 2 - continueWidth / 2,
                           (float)SCREEN_HEIGHT / 2 + 50},
                   customFont.baseSize * 0.8, 2, BLACK);
      } break;

      case LEADERBOARD: {
        const char* leaderboardTitle = "LEADERBOARD";
        int titleWidth = MeasureTextEx(customFont, leaderboardTitle,
                                       customFont.baseSize * 1.5, 2)
                             .x;
        DrawTextEx(titleFont, leaderboardTitle,
                   Vector2{(float)SCREEN_WIDTH / 2 - titleWidth / 2, 30},
                   customFont.baseSize * 1.5, 2, BLACK);

        DrawLeaderboard(customFont, customFont.baseSize, BLACK, SCREEN_WIDTH,
                        SCREEN_HEIGHT);  // Pass screen dimensions

        const char* backText = "Press ENTER or CLICK to Main Menu";
        int backWidth =
            MeasureTextEx(customFont, backText, customFont.baseSize * 0.8, 2).x;
        DrawTextEx(customFont, backText,
                   Vector2{(float)SCREEN_WIDTH / 2 - backWidth / 2,
                           (float)SCREEN_HEIGHT - 50},
                   customFont.baseSize * 0.8, 2, BLACK);
      } break;
    }

    EndDrawing();
  }

  //                                //
  // 5. De-Initialization           //
  //                                //

  UnloadTexture(backgroundTex);
  UnloadTexture(midgroundTex);
  UnloadTexture(foregroundTex);
  UnloadFont(customFont);
  UnloadFont(titleFont);
  CloseWindow();
  return 0;
}

// --- Function Definitions for main.cpp ---
void ResetGameplayElements() {
  currentScore = 0;
  currentLives = 1;
  playerBullets.clear();
  enemies.clear();
  // Reset enemy spawning timer as well for a fresh start
  lastEnemySpawnTime = GetTime();
  enemySpawnInterval = 2.0f;
}