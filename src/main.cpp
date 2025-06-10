/*
STANDARISASI
Gunakan Google C++ Style Guide
Spaces: 2

CARA BUILD PROGRAM

cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
*/

#include <raylib.h>  // Mengimpor library raylib

#include <algorithm>  // Untuk std::remove_if
#include <cstring>
#include <random>  // Untuk spawn musuh acak
#include <vector>  // Untuk menyimpan peluru dan musuh

#include "database.h"  // BARU: Sertakan header database untuk menutup koneksi
#include "player_data.h"  // Sertakan data pemain dan leaderboard
#include "window.h"       // Header untuk InisialisasiGameWindow()

// --- Status Game ---
enum GameState { LAYAR_JUDUL, MASUKAN_NAMA, PERMAINAN, GAME_OVER, LEADERBOARD };

// --- Struktur Pemain ---
struct Pemain {
  Rectangle kotak;
};

// --- Struktur Peluru ---
struct Peluru {
  Rectangle kotak;
  float kecepatan;
  bool aktif;
};

// --- Struktur Musuh ---
struct Musuh {
  Rectangle kotak;
  int indeksTekstur;
  float kecepatan;
  int kesehatan;
  bool aktif;
};

// -- Struktur Bintang --
struct Bintang {
  Rectangle kotak;
  float kecepatan;
  bool aktif;
};

// --- Variabel Status Game Global ---
GameState statusGameSaatIni = LAYAR_JUDUL;
int skorSaatIni = 0;
int nyawaSaatIni = 1;
int jumlahPeluruSaatIni = 50;  // Initial bullets

// Untuk kursor berkedip (tetap di main untuk kesederhanaan karena ini spesifik
// UI)
int hitunganBingkai = 0;

// Leaderboard search/sort variables (new)
bool showingSearchResults =
    false;                     // Flag to indicate if search results are shown
char searchBuffer[32] = "\0";  // Buffer for search input
int searchFramesCounter = 0;   // For blinking search cursor

// --- Gameplay-specific variables ---
std::vector<Peluru> peluruPemain;
std::vector<Musuh> musuhMusuh;
std::vector<Bintang> koleksiBintang;

float gulirBelakang = 0.0f;
float gulirTengah = 0.0f;
float gulirDepan = 0.0f;

Texture2D teksturLatarBelakang;
Texture2D teksturPemain;
std::vector<Texture2D> teksturMusuhArray;
Texture2D teksturPeluru;
Texture2D teksturBintang;

// Random number generation
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> spawnYMusuh(0, SCREEN_HEIGHT - 50);
std::uniform_real_distribution<> kecepatanMusuh(100.0f, 250.0f);
std::uniform_int_distribution<> indeksTeksturMusuh(0, 3);

std::uniform_int_distribution<> spawnYBintang(0, SCREEN_HEIGHT - 30);
std::uniform_real_distribution<> kecepatanBintang(80.0f, 180.0f);

// Timers
double waktuSpawnMusuhTerakhir = 0.0;
float intervalSpawnMusuh = 2.0f;
double waktuSpawnBintangTerakhir = 0.0;
float intervalSpawnBintang = 5.0f;
double waktuTembakTerakhir = 0.0;
float lajuTembakPemain = 0.15f;

// --- Function Prototypes (for main.cpp's responsibilities) ---
void ResetElemenPermainan();

int main() {
  InisialisasiGameWindow();

  Font fontKustom =
      LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 20, 0, 0);
  SetTextureFilter(fontKustom.texture, TEXTURE_FILTER_BILINEAR);

  Font fontJudul =
      LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 30, 0, 0);
  SetTextureFilter(fontJudul.texture, TEXTURE_FILTER_BILINEAR);

  // Load textures
  teksturLatarBelakang = LoadTexture("assets/background.png");
  teksturPemain = LoadTexture("assets/player.png");
  teksturPeluru = LoadTexture("assets/bullet.png");
  teksturBintang = LoadTexture("assets/bintang.png");

  // Load multiple enemy textures
  teksturMusuhArray.push_back(LoadTexture("assets/enemy1.png"));
  teksturMusuhArray.push_back(LoadTexture("assets/enemy2.png"));
  teksturMusuhArray.push_back(LoadTexture("assets/enemy3.png"));
  teksturMusuhArray.push_back(LoadTexture("assets/enemy4.png"));

  // Optional: Check for texture loading errors
  for (size_t i = 0; i < teksturMusuhArray.size(); ++i) {
    if (teksturMusuhArray[i].id == 0) {
      TraceLog(LOG_ERROR,
               TextFormat("Gagal memuat tekstur musuh%i.png!", i + 1));
    }
  }

  Pemain pemain;
  pemain.kotak =
      Rectangle{(float)SCREEN_WIDTH / 2 - teksturPemain.width / 2,
                (float)SCREEN_HEIGHT / 2 - teksturPemain.height / 2,
                (float)teksturPemain.width, (float)teksturPemain.height};

  InitPlayerData();  // This will now also initialize the database

  HideCursor();
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float deltaWaktu = GetFrameTime();
    hitunganBingkai++;
    searchFramesCounter++;  // Increment for search cursor blinking

    switch (statusGameSaatIni) {
      case LAYAR_JUDUL: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          statusGameSaatIni = MASUKAN_NAMA;
          g_namaPemain = "";
          memset(g_bufferNama, 0, sizeof(g_bufferNama));
        }
      } break;

      case MASUKAN_NAMA: {
        int kunci = GetCharPressed();
        while (kunci > 0) {
          if ((kunci >= 32) && (kunci <= 125) && (strlen(g_bufferNama) < 31)) {
            g_bufferNama[strlen(g_bufferNama)] = (char)kunci;
          }
          kunci = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
          if (strlen(g_bufferNama) > 0) {
            g_bufferNama[strlen(g_bufferNama) - 1] = '\0';
          }
        }

        if (IsKeyPressed(KEY_ENTER)) {
          if (strlen(g_bufferNama) > 0) {
            g_namaPemain = std::string(g_bufferNama);
            ResetElemenPermainan();
            statusGameSaatIni = PERMAINAN;
          }
        }
      } break;

      case PERMAINAN: {
        Vector2 posisiMouse = GetMousePosition();
        pemain.kotak.x = posisiMouse.x - pemain.kotak.width / 2;
        pemain.kotak.y = posisiMouse.y - pemain.kotak.height / 2;

        // Clamp player position
        if (pemain.kotak.x < 0) pemain.kotak.x = 0;
        if (pemain.kotak.x + pemain.kotak.width > SCREEN_WIDTH)
          pemain.kotak.x = SCREEN_WIDTH - pemain.kotak.width;
        if (pemain.kotak.y < 0) pemain.kotak.y = 0;
        if (pemain.kotak.y + pemain.kotak.height > SCREEN_HEIGHT)
          pemain.kotak.y = SCREEN_HEIGHT - pemain.kotak.height;

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
            (GetTime() - waktuTembakTerakhir >= lajuTembakPemain) &&
            jumlahPeluruSaatIni > 0) {
          Peluru peluruBaru;
          peluruBaru.kotak = Rectangle{
              pemain.kotak.x + pemain.kotak.width,
              pemain.kotak.y + 25.0f,  // Bullet spawn offset
              (float)teksturPeluru.width, (float)teksturPeluru.height};
          peluruBaru.kecepatan = 1000;
          peluruBaru.aktif = true;
          peluruPemain.push_back(peluruBaru);

          jumlahPeluruSaatIni--;
          waktuTembakTerakhir = GetTime();
        }

        // Update bullets
        for (auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            peluru.kotak.x += peluru.kecepatan * deltaWaktu;
            if (peluru.kotak.x > SCREEN_WIDTH) {
              peluru.aktif = false;
            }
          }
        }
        peluruPemain.erase(
            std::remove_if(peluruPemain.begin(), peluruPemain.end(),
                           [](const Peluru& b) { return !b.aktif; }),
            peluruPemain.end());

        // Spawn enemies
        if (GetTime() - waktuSpawnMusuhTerakhir > intervalSpawnMusuh) {
          Musuh musuhBaru;
          musuhBaru.indeksTekstur = indeksTeksturMusuh(gen);
          Texture2D teksturMusuhSaatIni =
              teksturMusuhArray[musuhBaru.indeksTekstur];
          musuhBaru.kotak =
              Rectangle{(float)SCREEN_WIDTH, (float)spawnYMusuh(gen),
                        (float)teksturMusuhSaatIni.width,
                        (float)teksturMusuhSaatIni.height};
          musuhBaru.kecepatan = kecepatanMusuh(gen);
          musuhBaru.kesehatan = 1;
          musuhBaru.aktif = true;
          musuhMusuh.push_back(musuhBaru);
          waktuSpawnMusuhTerakhir = GetTime();
          intervalSpawnMusuh = GetRandomValue(50, 150) / 100.0f;
        }

        // Spawn Bintang
        if (GetTime() - waktuSpawnBintangTerakhir > intervalSpawnBintang) {
          Bintang bintangBaru;
          bintangBaru.kotak = Rectangle{
              (float)SCREEN_WIDTH, (float)spawnYBintang(gen),
              (float)teksturBintang.width, (float)teksturBintang.height};
          bintangBaru.kecepatan = kecepatanBintang(gen);
          bintangBaru.aktif = true;
          koleksiBintang.push_back(bintangBaru);
          waktuSpawnBintangTerakhir = GetTime();
          intervalSpawnBintang = GetRandomValue(400, 700) / 100.0f;
        }

        // Update enemies & Player-Enemy collision
        for (auto& musuh : musuhMusuh) {
          if (musuh.aktif) {
            musuh.kotak.x -= musuh.kecepatan * deltaWaktu;
            if (musuh.kotak.x + musuh.kotak.width < 0) {
              musuh.aktif = false;
            }
            if (CheckCollisionRecs(pemain.kotak, musuh.kotak)) {
              nyawaSaatIni--;
              musuh.aktif = false;
              if (nyawaSaatIni <= 0) {
                AddScoreToLeaderboard(g_namaPemain,
                                      skorSaatIni);  // Save score to DB
                statusGameSaatIni = GAME_OVER;
              }
            }
          }
        }
        musuhMusuh.erase(
            std::remove_if(musuhMusuh.begin(), musuhMusuh.end(),
                           [](const Musuh& e) { return !e.aktif; }),
            musuhMusuh.end());

        // Update Bintang & Player-Bintang collision
        for (auto& bintang : koleksiBintang) {
          if (bintang.aktif) {
            bintang.kotak.x -= bintang.kecepatan * deltaWaktu;
            if (bintang.kotak.x + bintang.kotak.width < 0) {
              bintang.aktif = false;
            }
            if (CheckCollisionRecs(pemain.kotak, bintang.kotak)) {
              jumlahPeluruSaatIni += 5;  // Gain bullets
              bintang.aktif = false;
            }
          }
        }
        koleksiBintang.erase(
            std::remove_if(koleksiBintang.begin(), koleksiBintang.end(),
                           [](const Bintang& b) { return !b.aktif; }),
            koleksiBintang.end());

        // Bullet-Enemy collision
        for (auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            for (auto& musuh : musuhMusuh) {
              if (musuh.aktif &&
                  CheckCollisionRecs(peluru.kotak, musuh.kotak)) {
                musuh.kesehatan--;
                peluru.aktif = false;
                if (musuh.kesehatan <= 0) {
                  musuh.aktif = false;
                  skorSaatIni += 10;
                }
                break;
              }
            }
          }
        }

        // Background scrolling
        gulirBelakang -= 10.0f * deltaWaktu;
        gulirTengah -= 20.0f * deltaWaktu;
        gulirDepan -= 30.0f * deltaWaktu;
        if (gulirBelakang <= -teksturLatarBelakang.width) gulirBelakang = 0;
      } break;

      case GAME_OVER: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          statusGameSaatIni = LEADERBOARD;
          // When entering LEADERBOARD, show top scores by default
          UpdateLeaderboardDisplay(10);
          memset(searchBuffer, 0, sizeof(searchBuffer));  // Clear search buffer
          showingSearchResults = false;
        }
      } break;

      case LEADERBOARD: {
        // Handle search input
        int key = GetCharPressed();
        while (key > 0) {
          if ((key >= 32) && (key <= 125) && (strlen(searchBuffer) < 31)) {
            searchBuffer[strlen(searchBuffer)] = (char)key;
          }
          key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
          if (strlen(searchBuffer) > 0) {
            searchBuffer[strlen(searchBuffer) - 1] = '\0';
          }
        }

        // Perform search
        if (IsKeyPressed(KEY_ENTER)) {
          if (strlen(searchBuffer) > 0) {
            SearchAndDisplayLeaderboard(std::string(searchBuffer));
            showingSearchResults = true;
          } else {
            // If enter pressed with empty search, show top 10 again
            UpdateLeaderboardDisplay(10);
            showingSearchResults = false;
          }
        }

        // Go back to main menu
        if (IsKeyPressed(KEY_SPACE) ||
            IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
          statusGameSaatIni = LAYAR_JUDUL;
        }

      } break;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw background regardless of state
    DrawTextureEx(teksturLatarBelakang, Vector2{gulirBelakang, 0.0f}, 0.0f,
                  1.0f, WHITE);
    DrawTextureEx(teksturLatarBelakang,
                  Vector2{teksturLatarBelakang.width + gulirBelakang, 0.0f},
                  0.0f, 1.0f, WHITE);

    switch (statusGameSaatIni) {
      case LAYAR_JUDUL: {
        const char* teksJudul = "PESAWAT SEDERHANA 2D";
        const char* teksMulai = "Tekan ENTER atau KLIK untuk Memulai";
        const char* teksKeluar =
            "Tekan ESC untuk Keluar";  // Added instruction to exit
        int lebarJudul =
            MeasureTextEx(fontJudul, teksJudul, fontKustom.baseSize * 1.5, 2).x;
        int lebarMulai =
            MeasureTextEx(fontKustom, teksMulai, fontKustom.baseSize, 2).x;
        int lebarKeluar =
            MeasureTextEx(fontKustom, teksKeluar, fontKustom.baseSize, 2).x;

        DrawTextEx(fontJudul, teksJudul,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarJudul / 2,
                           (float)SCREEN_HEIGHT / 2 - 50},
                   fontKustom.baseSize * 1.5, 2, WHITE);
        DrawTextEx(fontKustom, teksMulai,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarMulai / 2,
                           (float)SCREEN_HEIGHT / 2 + 20},
                   fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, teksKeluar,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarKeluar / 2,
                           (float)SCREEN_HEIGHT / 2 + 40},
                   fontKustom.baseSize, 2, WHITE);
      } break;

      case MASUKAN_NAMA: {
        const char* teksPrompt = "MASUKKAN NAMA ANDA:";
        int lebarPrompt =
            MeasureTextEx(fontKustom, teksPrompt, fontKustom.baseSize, 2).x;
        int lebarBufferNama =
            MeasureTextEx(fontKustom, g_bufferNama, fontKustom.baseSize, 2).x;

        DrawTextEx(fontKustom, teksPrompt,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarPrompt / 2,
                           (float)SCREEN_HEIGHT / 2 - 50},
                   fontKustom.baseSize, 2, WHITE);
        DrawRectangleLines(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 40,
                           WHITE);
        DrawTextEx(fontKustom, g_bufferNama,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarBufferNama / 2,
                           (float)SCREEN_HEIGHT / 2 + 10},
                   fontKustom.baseSize, 2, WHITE);

        if (((hitunganBingkai / 30) % 2) == 0) {
          DrawTextEx(fontKustom, "_",
                     Vector2{(float)SCREEN_WIDTH / 2 - lebarBufferNama / 2 +
                                 lebarBufferNama,
                             (float)SCREEN_HEIGHT / 2 + 10},
                     fontKustom.baseSize, 2, WHITE);
        }
      } break;

      case PERMAINAN: {
        DrawTextureRec(teksturPemain,
                       Rectangle{0.0f, 0.0f, (float)teksturPemain.width,
                                 (float)teksturPemain.height},
                       Vector2{pemain.kotak.x, pemain.kotak.y}, WHITE);

        for (const auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            DrawTextureRec(teksturPeluru,
                           Rectangle{0.0f, 0.0f, (float)teksturPeluru.width,
                                     (float)teksturPeluru.height},
                           Vector2{peluru.kotak.x, peluru.kotak.y}, WHITE);
          }
        }

        for (const auto& musuh : musuhMusuh) {
          if (musuh.aktif) {
            if (musuh.indeksTekstur >= 0 &&
                musuh.indeksTekstur < teksturMusuhArray.size()) {
              DrawTextureRec(
                  teksturMusuhArray[musuh.indeksTekstur],
                  Rectangle{
                      0.0f, 0.0f,
                      (float)teksturMusuhArray[musuh.indeksTekstur].width,
                      (float)teksturMusuhArray[musuh.indeksTekstur].height},
                  Vector2{musuh.kotak.x, musuh.kotak.y}, WHITE);
            }
          }
        }

        for (const auto& bintang : koleksiBintang) {
          if (bintang.aktif) {
            DrawTextureRec(teksturBintang,
                           Rectangle{0.0f, 0.0f, (float)teksturBintang.width,
                                     (float)teksturBintang.height},
                           Vector2{bintang.kotak.x, bintang.kotak.y}, WHITE);
          }
        }

        DrawTextEx(fontKustom, TextFormat("Skor: %i", skorSaatIni),
                   Vector2{10, 10}, fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, TextFormat("Nyawa: %i", nyawaSaatIni),
                   Vector2{10, 40}, fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, TextFormat("Peluru: %i", jumlahPeluruSaatIni),
                   Vector2{10, 70}, fontKustom.baseSize, 2, WHITE);
        DrawFPS(SCREEN_WIDTH - 100, 10);
      } break;

      case GAME_OVER: {
        const char* teksGameOver = "GAME OVER!";
        const char* teksSkor = TextFormat("SKOR ANDA: %i", skorSaatIni);
        const char* teksLanjutkan =
            "Tekan ENTER atau KLIK untuk melihat Leaderboard";
        int lebarGameOver = MeasureTextEx(fontKustom, teksGameOver,
                                          fontKustom.baseSize * 1.5, 2)
                                .x;
        int lebarSkor =
            MeasureTextEx(fontKustom, teksSkor, fontKustom.baseSize, 2).x;
        int lebarLanjutkan = MeasureTextEx(fontKustom, teksLanjutkan,
                                           fontKustom.baseSize * 0.8, 2)
                                 .x;

        DrawTextEx(fontJudul, teksGameOver,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarGameOver / 2,
                           (float)SCREEN_HEIGHT / 2 - 70},
                   fontKustom.baseSize * 1.5, 2, WHITE);
        DrawTextEx(fontKustom, teksSkor,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarSkor / 2,
                           (float)SCREEN_HEIGHT / 2},
                   fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, teksLanjutkan,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarLanjutkan / 2,
                           (float)SCREEN_HEIGHT / 2 + 50},
                   fontKustom.baseSize * 0.8, 2, WHITE);
      } break;

      case LEADERBOARD: {
        const char* judulLeaderboard = "LEADERBOARD";
        int lebarJudul = MeasureTextEx(fontKustom, judulLeaderboard,
                                       fontKustom.baseSize * 1.5, 2)
                             .x;
        DrawTextEx(fontJudul, judulLeaderboard,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarJudul / 2, 30},
                   fontKustom.baseSize * 1.5, 2, WHITE);

        // Draw Leaderboard entries
        DrawLeaderboard(fontKustom, fontKustom.baseSize, WHITE, SCREEN_WIDTH,
                        SCREEN_HEIGHT);

        // Draw search input box and prompt
        const char* searchPrompt = "CARI NAMA (ENTER untuk mencari/TOP 10):";
        int searchPromptWidth = MeasureTextEx(fontKustom, searchPrompt,
                                              fontKustom.baseSize * 0.8, 2)
                                    .x;
        DrawTextEx(fontKustom, searchPrompt,
                   Vector2{(float)SCREEN_WIDTH / 2 - searchPromptWidth / 2,
                           (float)SCREEN_HEIGHT - 100},
                   fontKustom.baseSize * 0.8, 2, WHITE);
        DrawRectangleLines(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 70, 200, 30,
                           WHITE);
        DrawTextEx(fontKustom, searchBuffer,
                   Vector2{(float)SCREEN_WIDTH / 2 -
                               MeasureTextEx(fontKustom, searchBuffer,
                                             fontKustom.baseSize * 0.8, 2)
                                       .x /
                                   2,
                           (float)SCREEN_HEIGHT - 60},
                   fontKustom.baseSize * 0.8, 2, WHITE);

        // Blinking cursor for search input
        if (((searchFramesCounter / 30) % 2) == 0) {
          DrawTextEx(fontKustom, "_",
                     Vector2{(float)SCREEN_WIDTH / 2 -
                                 MeasureTextEx(fontKustom, searchBuffer,
                                               fontKustom.baseSize * 0.8, 2)
                                         .x /
                                     2 +
                                 MeasureTextEx(fontKustom, searchBuffer,
                                               fontKustom.baseSize * 0.8, 2)
                                     .x,
                             (float)SCREEN_HEIGHT - 60},
                     fontKustom.baseSize * 0.8, 2, WHITE);
        }

        const char* backText =
            "Tekan SPASI atau KLIK KANAN untuk ke Menu Utama";  // Updated text
        int backWidth =
            MeasureTextEx(fontKustom, backText, fontKustom.baseSize * 0.8, 2).x;
        DrawTextEx(fontKustom, backText,
                   Vector2{(float)SCREEN_WIDTH / 2 - backWidth / 2,
                           (float)SCREEN_HEIGHT - 20},
                   fontKustom.baseSize * 0.8, 2, WHITE);
      } break;
    }

    EndDrawing();
  }

  // De-Initialization
  UnloadTexture(teksturLatarBelakang);
  UnloadTexture(teksturPemain);
  UnloadTexture(teksturPeluru);
  UnloadTexture(teksturBintang);
  for (auto& tex : teksturMusuhArray) {
    UnloadTexture(tex);
  }
  UnloadFont(fontKustom);
  UnloadFont(fontJudul);
  CloseDatabase();  // BARU: Tutup koneksi database saat keluar aplikasi
  CloseWindow();
  return 0;
}

// --- Function Definitions for main.cpp ---
void ResetElemenPermainan() {
  skorSaatIni = 0;
  nyawaSaatIni = 1;
  jumlahPeluruSaatIni = 50;
  peluruPemain.clear();
  musuhMusuh.clear();
  koleksiBintang.clear();
  waktuSpawnMusuhTerakhir = GetTime();
  intervalSpawnMusuh = 2.0f;
  waktuSpawnBintangTerakhir = GetTime();
  intervalSpawnBintang = 5.0f;
}