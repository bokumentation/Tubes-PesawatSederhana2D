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
#include <random>     // Untuk spawning musuh secara acak
#include <vector>     // Untuk menyimpan peluru dan musuh

#include "window.h"  // Header untuk InisialisasiGameWindow()

struct Player {    // Mendefinisikan struktur sederhana untuk player (pesawat)
  Rectangle rect;  // Position and size
  Color color;     // Color of the player
};

struct Bullet {    // Mendefinisikan struktur pada sistem menembak
  Rectangle rect;  // Menyimpan posisi peluru & ukuran peluru
  float speed;     // Menyimpan Kecepatan peluru
  Color color;     // Warna pada peluru
  bool active;     // True if the bullet is currently active/on screen
};

struct Enemy {     // Mendefinisikan struktur untuk musuh
  Rectangle rect;  // Position and size
  Color color;     // Color of the enemy
  float speed;     // Movement speed of the enemy
  int health;      // Enemy health
  bool active;     // True if the enemy is currently active/on screen
};

// Global variables for game state (you might want to encapsulate these in a
// Game class later)
int score = 0;
int lives = 3;

int main() {
  //                                //
  // 1. INISIALISASI                //
  //                                //

  InisialisasiGameWindow();

  Font customFont = LoadFont("assets/fonts/JetBrainsMono-Regular.ttf");

  // Inisialisasi player
  Player player;
  player.rect = {(float)SCREEN_WIDTH / 2 - 25, (float)SCREEN_HEIGHT / 2 - 25,
                 50, 50};  // Centered 50x50 square
  player.color = BLUE;     // Blue plane

  std::vector<Bullet> playerBullets;  // Tempat penyimpanan peluru player
  std::vector<Enemy> enemies;         // Tempat penyimpanan musuh

  // Background scrolling variables
  float scrollingBack = 0.0f;
  float scrollingMid = 0.0f;
  float scrollingFore = 0.0f;

  // IMPORTANT: You need to have these image files in your assets/backgrounds/
  // directory For example: assets/backgrounds/space_bg.png
  // assets/backgrounds/stars_mid.png
  // assets/backgrounds/stars_fore.png
  Texture2D backgroundTex = LoadTexture("assets/backgrounds/space_bg.png");
  Texture2D midgroundTex = LoadTexture("assets/backgrounds/stars_mid.png");
  Texture2D foregroundTex = LoadTexture("assets/backgrounds/stars_fore.png");

  // Random number generation for enemy spawning
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> enemySpawnY(
      0, SCREEN_HEIGHT - 50);  // Random Y position for enemies
  std::uniform_real_distribution<> enemySpeed(
      100.0f, 250.0f);  // Random speed for enemies

  // Timer for enemy spawning
  double lastEnemySpawnTime = GetTime();
  float enemySpawnInterval = 2.0f;  // Spawn an enemy every 2 seconds

  // Fungsi menyembunyikan kursor dalam game
  HideCursor();
  // Set target FPS
  SetTargetFPS(60);

  //                                //
  // 2. GAME LOOP                   //
  //                                //

  while (!WindowShouldClose()) {
    // Get frame time for consistent movement
    float deltaTime = GetFrameTime();

    //                                //
    // 3. UPDATE (LOGIKA GAME)        //
    //                                //

    // Mendapatkan posisi Mouse
    Vector2 mousePosisi = GetMousePosition();

    // Set posisi player ke posisi mouse, menengahkan player rectangle
    player.rect.x = mousePosisi.x - player.rect.width / 2;
    player.rect.y = mousePosisi.y - player.rect.height / 2;

    // Supaya player tetap di dalam window
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

    // Kontrol menembak: klik kiri mouse (or KEY_SPACE if you want to keep that
    // option)
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Bullet newBullet;
      newBullet.rect = {
          player.rect.x +
              player.rect.width,  // appears from the right edge of the player
          player.rect.y + player.rect.height / 2 -
              5,    // centered vertically with player
          20, 10};  // Adjust size for a horizontal bullet (width, height)
      newBullet.speed = 500;
      newBullet.color = RED;
      newBullet.active = true;
      playerBullets.push_back(newBullet);
    }

    // Update player bullets
    for (auto& bullet : playerBullets) {
      if (bullet.active) {
        bullet.rect.x += bullet.speed * deltaTime;  // Bullets fly to the right
        if (bullet.rect.x >
            SCREEN_WIDTH) {  // Check if bullet goes off screen to the right
          bullet.active = false;  // Deactivate bullet when it goes off screen
        }
      }
    }

    // Remove inactive bullets to save memory
    playerBullets.erase(
        std::remove_if(playerBullets.begin(), playerBullets.end(),
                       [](const Bullet& b) { return !b.active; }),
        playerBullets.end());

    // Enemy spawning
    if (GetTime() - lastEnemySpawnTime > enemySpawnInterval) {
      Enemy newEnemy;
      newEnemy.rect = {(float)SCREEN_WIDTH, (float)enemySpawnY(gen), 50,
                       50};  // Spawn off-screen to the right
      newEnemy.color = LIME;
      newEnemy.speed = enemySpeed(gen);
      newEnemy.health = 1;
      newEnemy.active = true;
      enemies.push_back(newEnemy);
      lastEnemySpawnTime = GetTime();  // Reset spawn timer
      enemySpawnInterval =
          GetRandomValue(100, 300) /
          100.0f;  // Randomize next spawn interval (1 to 3 seconds)
    }

    // Update enemies
    for (auto& enemy : enemies) {
      if (enemy.active) {
        enemy.rect.x -= enemy.speed * deltaTime;  // Enemies move left
        if (enemy.rect.x + enemy.rect.width < 0) {
          enemy.active = false;  // Deactivate enemy when it goes off screen
        }

        // Collision detection: player vs enemy
        if (CheckCollisionRecs(player.rect, enemy.rect)) {
          lives--;               // Player loses a life
          enemy.active = false;  // Enemy is destroyed on collision
          if (lives <= 0) {
            // TODO: Implement game over state
            // For now, let's just close the window
            // CloseWindow(); // Or set a game over flag
          }
        }
      }
    }

    // Remove inactive enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                 [](const Enemy& e) { return !e.active; }),
                  enemies.end());

    // Collision detection: player bullets vs enemies
    for (auto& bullet : playerBullets) {
      if (bullet.active) {
        for (auto& enemy : enemies) {
          if (enemy.active && CheckCollisionRecs(bullet.rect, enemy.rect)) {
            enemy.health--;
            bullet.active = false;  // Bullet disappears on hit
            if (enemy.health <= 0) {
              enemy.active = false;  // Enemy is destroyed
              score += 10;           // Increase score
            }
            // Break from inner loop as bullet is gone
            break;  // A bullet can only hit one enemy
          }
        }
      }
    }

    // Background scrolling for parallax effect
    scrollingBack -= 10.0f * deltaTime;
    scrollingMid -= 20.0f * deltaTime;
    scrollingFore -= 30.0f * deltaTime;

    // Reset scrolling positions when they go off screen to create a continuous
    // loop
    if (scrollingBack <= -backgroundTex.width) scrollingBack = 0;
    if (scrollingMid <= -midgroundTex.width) scrollingMid = 0;
    if (scrollingFore <= -foregroundTex.width) scrollingFore = 0;

    //                                //
    // 4. Drawing / Menggambar        //
    //                                //

    BeginDrawing();

    ClearBackground(RAYWHITE);

    // Draw scrolling backgrounds (twice to create a continuous loop)
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
    DrawTextureEx(foregroundTex,
                  (Vector2){foregroundTex.width + scrollingFore, 0}, 0.0f, 1.0f,
                  WHITE);

    // Draw the player
    DrawRectangleRec(player.rect, player.color);

    // Draw player bullets
    for (const auto& bullet : playerBullets) {
      if (bullet.active) {
        DrawRectangleRec(bullet.rect, bullet.color);
      }
    }

    // Draw enemies
    for (const auto& enemy : enemies) {
      if (enemy.active) {
        DrawRectangleRec(enemy.rect, enemy.color);
      }
    }

    // Draw UI elements
    DrawTextEx(customFont, TextFormat("Score: %i", score), Vector2{10, 10},
               customFont.baseSize, 2, BLACK);
    DrawTextEx(customFont, TextFormat("Lives: %i", lives), Vector2{10, 40},
               customFont.baseSize, 2, BLACK);

    DrawFPS(SCREEN_WIDTH - 100, 10);  // Display FPS

    EndDrawing();  // End drawing operations
  }

  //                                //
  // 5. De-Initialization           //
  //                                //

  // Unload textures
  UnloadTexture(backgroundTex);
  UnloadTexture(midgroundTex);
  UnloadTexture(foregroundTex);
  UnloadFont(customFont);
  CloseWindow();
  return 0;
}