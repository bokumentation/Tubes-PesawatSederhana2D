/*
STANDARISASI
Gunakan Google C++ Style Guide
Spaces: 2

## CARA BUILD PROGRAM ##
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build --output-on-failure
*/

#include <raylib.h>  // Mengimpor library raylib

#include "window.h"  // Header untuk InisialisasiGameWindow()

// Pixels per frame
struct Player {    // Mendefinisikan struktur sederhana untuk player (pesawat)
  Rectangle rect;  // Position and size
  Color color;     // Color of the player
};

int main() {
  //                                //
  // 1. INISIALISASI                //
  //                                //

  // inisialisasi InitGameWindow()
  InisialisasiGameWindow();

  Font customFont = LoadFont("assets/fonts/JetBrainsMono-Regular.ttf");

  // Inisialisasi player
  Player player;
  player.rect = {(float)SCREEN_WIDTH / 2 - 25, (float)SCREEN_HEIGHT / 2 - 25,
                 50, 50};  // Centered 50x50 square
  player.color = BLUE;     // Blue plane
  // Fungsi menyembunyikan kursor dalam game
  HideCursor();
  // Set target FPS
  SetTargetFPS(60);

  //                                //
  // 2. GAME LOOP                   //
  //                                //

  // Kode looping selama window tidak di close (atau ESC ditekan)
  // Mendeteksi window close button atau ESC key
  while (!WindowShouldClose()) {
    //                                //
    // 3. UPDATE (LOGIKA GAME)        //
    //                                //

    // Mendapatkan posisi Mouse
    Vector2 mousePos = GetMousePosition();

    // Set posisi player ke posisi mouse, menengahkan player rectangle
    player.rect.x = mousePos.x - player.rect.width / 2;
    player.rect.y = mousePos.y - player.rect.height / 2;

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

    //                                //
    // 4. Drawing / Menggambar        //
    //                                //

    // Start drawing operations
    BeginDrawing();

    // Menghapus background dengan warna putih
    ClearBackground(RAYWHITE);

    // Draw the player (a rectangle for now)
    DrawRectangleRec(player.rect, player.color);

    // Draw some text for debugging or info
    // DrawText("Gerakan bluebox dengan mouse!", 10, 10, 20, DARKGRAY);
    DrawTextEx(customFont, "Hello, JetBrains Mono!", (Vector2){10, 10},
               customFont.baseSize, 2, BLACK);

    DrawFPS(SCREEN_WIDTH - 100, 10);  // Display FPS

    EndDrawing();  // End drawing operations
  }

  //                                //
  // 5. De-Initialization           //
  //                                //

  // Close window and unload OpenGL context
  UnloadFont(customFont);
  CloseWindow();
  return 0;
}