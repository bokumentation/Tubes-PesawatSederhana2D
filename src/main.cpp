/*
STANDARISASI
Gunakan Google C++ Style Guide
Spaces: 2

CARA BUILD PROGRAM

cmake -S . -B build -G "MinGW Makefiles"
cmake --build build

-- Update Teddy 250604
*/

#include <raylib.h>  // Mengimpor library raylib

#include <algorithm>  // Untuk remove_if
#include <vector>     // Untuk menyimpan peluru

#include "window.h"  // Header untuk InisialisasiGameWindow()

struct Player {    // Mendefinisikan struktur sederhana untuk player (pesawat)
  Rectangle rect;  // Position and size
  Color color;     // Color of the player
};

struct Bullet {    // Mendefinisikan struktur pada sistem menembak
  Rectangle rect;  // Menyimpan posisi peluru & ukuran peluru
  float speed;     // Menyimpan Kecepatan peluru
  Color color;     // Warna pada peluru
};

struct Enemy {     // Struktur musuh
  Rectangle rect;  // Posisi & ukuran musuh
  Color color;     // Warna musuh
};

float shootCooldown = 0.0f;
const float shootCooldownTime = 0.2f; // 0.2 detik delay antara tembakan

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

  // Inisialisasi musuh
  Enemy enemy;
  enemy.rect = {SCREEN_WIDTH - 100, SCREEN_HEIGHT / 2 - 25, 50, 50};  // Posisi kanan tengah layar
  enemy.color = DARKGRAY;

  bool musuhHidup = true;  // Musuh hidup saat awal

  std::vector<Bullet> bullets;  // Tempat penyimpanan peluru

  int skor = 0;  // <<<<< Tambahan: variabel skor

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

    // Update cooldown
    shootCooldown -= GetFrameTime();

    // Kontrol menembak: klik kiri mouse atau tombol spasi
    if ((IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_SPACE)) && shootCooldown <= 0) {
      Bullet newBullet;
      newBullet.rect = {
          player.rect.x + player.rect.width,           // posisi X: kanan pesawat
          player.rect.y + player.rect.height / 2 - 5,  // posisi Y: tengah vertikal pesawat
          20, 10};                                     // ukuran peluru horizontal
      newBullet.speed = 500;                           // kecepatan peluru
      newBullet.color = RED;                           // Warna peluru
      bullets.push_back(newBullet);
      shootCooldown = shootCooldownTime; // Reset cooldown
    }

    // Update posisi semua peluru (sekarang ke kanan)
    for (auto& bullet : bullets) {
      bullet.rect.x += bullet.speed * GetFrameTime();  // Bergerak ke kanan
    }

    // Deteksi tabrakan peluru dengan musuh
    if (musuhHidup) {
      bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [&](const Bullet& b) {
          if (CheckCollisionRecs(b.rect, enemy.rect)) {
            musuhHidup = false;  // Musuh hancur
            skor += 10;          // <<<<< Tambahan: tambah skor
            return true;         // Hapus peluru
          }
          return b.rect.x > SCREEN_WIDTH;
        }), bullets.end());
    } else {
      bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b) {
          return b.rect.x > SCREEN_WIDTH;
        }), bullets.end());
    }

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
    DrawTextEx(customFont, "Hello, JetBrains Mono!", Vector2{10, 10},
               customFont.baseSize, 2, BLACK);

    // Gambar musuh jika masih hidup
    if (musuhHidup) {
      DrawRectangleRec(enemy.rect, enemy.color);
    } else {
      DrawText("MUSUH HANCUR!", SCREEN_WIDTH / 2 - 80, 40, 20, RED);
    }

    // Gambar semua peluru
    for (const auto& bullet : bullets) {
      DrawRectangleRec(bullet.rect, bullet.color);
    }

    // <<<<< Tambahan: tampilkan skor
    DrawText(TextFormat("SKOR: %d", skor), 10, 40, 20, DARKGREEN);

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
