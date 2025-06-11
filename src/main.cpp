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
#include <cstring>    //
#include <random>     // Untuk spawn musuh acak
#include <vector>     // Untuk menyimpan peluru dan musuh

#include "database.h"  // Sertakan header database untuk menutup koneksi dan enum
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
int jumlahPeluruSaatIni = 3;  // Initial bullets

// Untuk kursor berkedip (tetap di main untuk kesederhanaan karena ini spesifik
// UI)
int hitunganBingkai = 0;

// Leaderboard search/sort variables
bool showingSearchResults = false;
char searchBuffer[32] = "\0";
int searchFramesCounter = 0;

// BARU: Variabel untuk menyimpan urutan sorting saat ini
LeaderboardSortOrder currentSortOrder = SORT_BY_SCORE_DESC;

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

// --- Prototipe Fungsi (untuk tanggung jawab main.cpp) ---
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

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    float deltaWaktu = GetFrameTime();
    hitunganBingkai++;
    searchFramesCounter++;

    switch (statusGameSaatIni) {
      case LAYAR_JUDUL: {
        // Define the START button rectangle
        Rectangle tombolMulai = {(float)SCREEN_WIDTH / 2 - 70,
                                 (float)SCREEN_HEIGHT / 2 + 80, 140, 40};

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          // Check if START button is clicked
          if (CheckCollisionPointRec(GetMousePosition(), tombolMulai)) {
            statusGameSaatIni = MASUKAN_NAMA;
            g_namaPemain = "";
            memset(g_bufferNama, 0, sizeof(g_bufferNama));
          }
        }
        // Original key press for backward compatibility or alternative input
        if (IsKeyPressed(KEY_ENTER)) {  // Keep existing key input
          statusGameSaatIni = MASUKAN_NAMA;
          g_namaPemain = "";
          memset(g_bufferNama, 0, sizeof(g_bufferNama));
        }
      } break;

      case MASUKAN_NAMA: {
        // Tangani input karakter untuk nama pemain.
        int kunci = GetCharPressed();
        while (kunci > 0) {
          if ((kunci >= 32) && (kunci <= 125) && (strlen(g_bufferNama) < 31)) {
            g_bufferNama[strlen(g_bufferNama)] = (char)kunci;
          }
          kunci = GetCharPressed();
        }

        // Tangani tombol BACKSPACE untuk menghapus karakter.
        if (IsKeyPressed(KEY_BACKSPACE)) {
          if (strlen(g_bufferNama) > 0) {
            g_bufferNama[strlen(g_bufferNama) - 1] = '\0';
          }
        }

        // Jika ENTER ditekan dan nama tidak kosong, mulai permainan.
        if (IsKeyPressed(KEY_ENTER)) {
          if (strlen(g_bufferNama) > 0) {
            g_namaPemain = std::string(g_bufferNama);
            ResetElemenPermainan();         // Reset elemen game.
            statusGameSaatIni = PERMAINAN;  // Ubah status ke PERMAINAN.
            HideCursor();
          }
        }
      } break;

      case PERMAINAN: {
        // Perbarui posisi pemain berdasarkan posisi mouse.
        Vector2 posisiMouse = GetMousePosition();
        pemain.kotak.x = posisiMouse.x - pemain.kotak.width / 2;
        pemain.kotak.y = posisiMouse.y - pemain.kotak.height / 2;

        // Batasi posisi pemain agar tidak keluar layar.
        if (pemain.kotak.x < 0) pemain.kotak.x = 0;
        if (pemain.kotak.x + pemain.kotak.width > SCREEN_WIDTH)
          pemain.kotak.x = SCREEN_WIDTH - pemain.kotak.width;
        if (pemain.kotak.y < 0) pemain.kotak.y = 0;
        if (pemain.kotak.y + pemain.kotak.height > SCREEN_HEIGHT)
          pemain.kotak.y = SCREEN_HEIGHT - pemain.kotak.height;

        // Logika penembakan peluru pemain.
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
            (GetTime() - waktuTembakTerakhir >= lajuTembakPemain) &&
            jumlahPeluruSaatIni > 0) {
          Peluru peluruBaru;
          peluruBaru.kotak = Rectangle{
              pemain.kotak.x + pemain.kotak.width, pemain.kotak.y + 25.0f,
              (float)teksturPeluru.width, (float)teksturPeluru.height};
          peluruBaru.kecepatan = 1000;
          peluruBaru.aktif = true;
          peluruPemain.push_back(
              peluruBaru);  // Tambahkan peluru baru ke vektor.

          jumlahPeluruSaatIni--;            // Kurangi jumlah peluru.
          waktuTembakTerakhir = GetTime();  // Reset timer tembak.
        }

        // Perbarui posisi peluru dan hapus peluru yang tidak aktif.
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

        // Logika spawn musuh.
        if (GetTime() - waktuSpawnMusuhTerakhir > intervalSpawnMusuh) {
          Musuh musuhBaru;
          musuhBaru.indeksTekstur =
              indeksTeksturMusuh(gen);  // Pilih tekstur musuh secara acak.
          Texture2D teksturMusuhSaatIni =
              teksturMusuhArray[musuhBaru.indeksTekstur];
          musuhBaru.kotak =
              Rectangle{(float)SCREEN_WIDTH, (float)spawnYMusuh(gen),
                        (float)teksturMusuhSaatIni.width,
                        (float)teksturMusuhSaatIni.height};
          musuhBaru.kecepatan = kecepatanMusuh(gen);
          musuhBaru.kesehatan = 1;
          musuhBaru.aktif = true;
          musuhMusuh.push_back(musuhBaru);  // Tambahkan musuh baru.
          waktuSpawnMusuhTerakhir = GetTime();
          intervalSpawnMusuh =
              GetRandomValue(50, 150) / 100.0f;  // Interval spawn acak.
        }

        // Logika spawn Bintang (item bonus).
        if (GetTime() - waktuSpawnBintangTerakhir > intervalSpawnBintang) {
          Bintang bintangBaru;
          bintangBaru.kotak = Rectangle{
              (float)SCREEN_WIDTH, (float)spawnYBintang(gen),
              (float)teksturBintang.width, (float)teksturBintang.height};
          bintangBaru.kecepatan = kecepatanBintang(gen);
          bintangBaru.aktif = true;
          koleksiBintang.push_back(bintangBaru);  // Tambahkan bintang baru.
          waktuSpawnBintangTerakhir = GetTime();
          intervalSpawnBintang =
              GetRandomValue(400, 700) / 100.0f;  // Interval spawn acak.
        }

        // Perbarui posisi musuh & deteksi tabrakan Pemain-Musuh.
        for (auto& musuh : musuhMusuh) {
          if (musuh.aktif) {
            musuh.kotak.x -= musuh.kecepatan * deltaWaktu;
            if (musuh.kotak.x + musuh.kotak.width < 0) {
              musuh.aktif = false;  // Nonaktifkan musuh jika keluar layar.
            }
            if (CheckCollisionRecs(pemain.kotak, musuh.kotak)) {
              nyawaSaatIni--;       // Kurangi nyawa pemain.
              musuh.aktif = false;  // Nonaktifkan musuh yang bertabrakan.
              if (nyawaSaatIni <= 0) {
                AddScoreToLeaderboard(
                    g_namaPemain,
                    skorSaatIni);  // Tambahkan skor ke leaderboard.
                statusGameSaatIni = GAME_OVER;  // Ubah status ke GAME_OVER.
              }
            }
          }
        }
        // Hapus musuh yang tidak aktif dari vektor.
        musuhMusuh.erase(
            std::remove_if(musuhMusuh.begin(), musuhMusuh.end(),
                           [](const Musuh& e) { return !e.aktif; }),
            musuhMusuh.end());

        // Perbarui posisi Bintang & deteksi tabrakan Pemain-Bintang.
        for (auto& bintang : koleksiBintang) {
          if (bintang.aktif) {
            bintang.kotak.x -= bintang.kecepatan * deltaWaktu;
            if (bintang.kotak.x + bintang.kotak.width < 0) {
              bintang.aktif = false;  // Nonaktifkan bintang jika keluar layar.
            }
            if (CheckCollisionRecs(pemain.kotak, bintang.kotak)) {
              jumlahPeluruSaatIni += 2;  // Pemain mendapatkan 5 peluru.
              bintang.aktif = false;     // Nonaktifkan bintang yang terkumpul.
            }
          }
        }
        // Hapus bintang yang tidak aktif dari vektor.
        koleksiBintang.erase(
            std::remove_if(koleksiBintang.begin(), koleksiBintang.end(),
                           [](const Bintang& b) { return !b.aktif; }),
            koleksiBintang.end());

        // Deteksi tabrakan Peluru-Musuh.
        for (auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            for (auto& musuh : musuhMusuh) {
              if (musuh.aktif &&
                  CheckCollisionRecs(peluru.kotak, musuh.kotak)) {
                musuh.kesehatan--;  // Kurangi kesehatan musuh.
                peluru.aktif =
                    false;  // Nonaktifkan peluru setelah mengenai musuh.
                if (musuh.kesehatan <= 0) {
                  musuh.aktif =
                      false;  // Nonaktifkan musuh jika kesehatannya nol.
                  skorSaatIni += 10;  // Tambahkan skor.
                }
                break;  // Hentikan iterasi musuh setelah tabrakan.
              }
            }
          }
        }

        // Efek gulir latar belakang (paralaks).
        gulirBelakang -= 10.0f * deltaWaktu;
        gulirTengah -= 20.0f * deltaWaktu;
        gulirDepan -= 30.0f * deltaWaktu;
        // Reset posisi gulir jika keluar batas.
        if (gulirBelakang <= -teksturLatarBelakang.width) gulirBelakang = 0;
      } break;

      case GAME_OVER: {
        // Jika ENTER atau KLIK KIRI ditekan, ubah status ke LEADERBOARD.
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          statusGameSaatIni = LEADERBOARD;
          // Saat masuk LEADERBOARD, tampilkan skor teratas secara default
          // (diurutkan berdasarkan skor).
          currentSortOrder =
              SORT_BY_SCORE_DESC;  // Atur ulang urutan pengurutan.
          UpdateLeaderboardDisplay(
              10, currentSortOrder);  // Perbarui tampilan leaderboard.
          memset(searchBuffer, 0,
                 sizeof(searchBuffer));  // Bersihkan buffer pencarian.
          showingSearchResults =
              false;  // Atur bahwa tidak ada hasil pencarian yang ditampilkan.
          ShowCursor();  // Show cursor when entering LEADERBOARD
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

        // Perform search or refresh display
        if (IsKeyPressed(KEY_ENTER)) {
          if (strlen(searchBuffer) > 0) {
            SearchAndDisplayLeaderboard(std::string(searchBuffer),
                                        currentSortOrder);
            showingSearchResults = true;
          } else {
            // If enter pressed with empty search, show top 10 (or current sort
            // order)
            UpdateLeaderboardDisplay(10, currentSortOrder);
            showingSearchResults = false;
          }
        }

        // Handle button clicks for sorting (defined here for click detection)
        Rectangle tombolUrutSkor = {(float)SCREEN_WIDTH / 2 + 245.0f,
                                    (float)SCREEN_HEIGHT - 430.0f, 140, 30};
        Rectangle tombolUrutNama = {(float)SCREEN_WIDTH / 2 + 245.0f,
                                    (float)SCREEN_HEIGHT - 390.0f, 140, 30};

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          if (CheckCollisionPointRec(GetMousePosition(), tombolUrutSkor)) {
            currentSortOrder = SORT_BY_SCORE_DESC;
            if (showingSearchResults) {
              SearchAndDisplayLeaderboard(std::string(searchBuffer),
                                          currentSortOrder);
            } else {
              UpdateLeaderboardDisplay(10, currentSortOrder);
            }
          } else if (CheckCollisionPointRec(GetMousePosition(),
                                            tombolUrutNama)) {
            currentSortOrder = SORT_BY_NAME_ASC;
            if (showingSearchResults) {
              SearchAndDisplayLeaderboard(std::string(searchBuffer),
                                          currentSortOrder);
            } else {
              UpdateLeaderboardDisplay(10, currentSortOrder);
            }
          }
        }

        // KEMBALI KE MENU button logic
        Rectangle tombolKembaliKeMenu = {(float)SCREEN_WIDTH / 2 - 380,
                                         (float)SCREEN_HEIGHT - 40, 200,
                                         25};  // Defining for click detection
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          if (CheckCollisionPointRec(GetMousePosition(), tombolKembaliKeMenu)) {
            statusGameSaatIni = LAYAR_JUDUL;
            ShowCursor();  // Hide cursor when leaving LEADERBOARD
          }
        }
        // Original key press for returning (SPACE or Right-Click) - keep as
        // alternative
        if (IsKeyPressed(KEY_SPACE) ||
            IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
          statusGameSaatIni = LAYAR_JUDUL;
          HideCursor();
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
        const char* teksKeluar = "Tekan ESC untuk Keluar";

        int lebarJudul =
            MeasureTextEx(fontJudul, teksJudul, fontKustom.baseSize * 1.5f, 2)
                .x;
        int lebarKeluar =
            MeasureTextEx(fontKustom, teksKeluar, fontKustom.baseSize, 2).x;

        // Draw Judul
        DrawTextEx(fontJudul, teksJudul,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarJudul / 2,
                           (float)SCREEN_HEIGHT / 2 - 50},
                   fontKustom.baseSize * 1.5f, 2, WHITE);

        // Define the START button rectangle for drawing
        Rectangle tombolMulai = {(float)SCREEN_WIDTH / 2 - 70,
                                 (float)SCREEN_HEIGHT / 2 + 80, 140, 40};

        // Draw START Button
        Color warnaTombolMulai =
            (CheckCollisionPointRec(GetMousePosition(), tombolMulai))
                ? LIGHTGRAY
                : GRAY;
        DrawRectangleRec(tombolMulai, warnaTombolMulai);
        DrawRectangleLinesEx(tombolMulai, 2, WHITE);
        const char* teksMulai = "MULAI";
        int lebarTeksMulai =
            MeasureTextEx(fontKustom, teksMulai, fontKustom.baseSize * 1.2f, 2)
                .x;
        DrawTextEx(fontKustom, teksMulai,
                   Vector2{tombolMulai.x + tombolMulai.width / 2.0f -
                               lebarTeksMulai / 2.0f,
                           tombolMulai.y + tombolMulai.height / 2.0f -
                               fontKustom.baseSize * 0.6f},
                   fontKustom.baseSize * 1.2f, 2, WHITE);

        // Draw Exit instruction
        DrawTextEx(fontKustom, teksKeluar,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarKeluar / 2,
                           (float)SCREEN_HEIGHT - 30},
                   fontKustom.baseSize, 2, WHITE);
      } break;

      case MASUKAN_NAMA: {
        const char* teksPrompt = "MASUKKAN NAMA ANDA:";
        int lebarPrompt =
            MeasureTextEx(fontKustom, teksPrompt, fontKustom.baseSize, 2).x;
        int lebarBufferNama =
            MeasureTextEx(fontKustom, g_bufferNama, fontKustom.baseSize, 2).x;

        // Gambar prompt dan kotak input nama.
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

        // Gambar kursor berkedip.
        if (((hitunganBingkai / 30) % 2) == 0) {
          DrawTextEx(fontKustom, "_",
                     Vector2{(float)SCREEN_WIDTH / 2 - lebarBufferNama / 2 +
                                 lebarBufferNama,
                             (float)SCREEN_HEIGHT / 2 + 10},
                     fontKustom.baseSize, 2, WHITE);
        }
      } break;

      case PERMAINAN: {
        // Gambar pemain.
        DrawTextureRec(teksturPemain,
                       Rectangle{0.0f, 0.0f, (float)teksturPemain.width,
                                 (float)teksturPemain.height},
                       Vector2{pemain.kotak.x, pemain.kotak.y}, WHITE);

        // Gambar semua peluru aktif.
        for (const auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            DrawTextureRec(teksturPeluru,
                           Rectangle{0.0f, 0.0f, (float)teksturPeluru.width,
                                     (float)teksturPeluru.height},
                           Vector2{peluru.kotak.x, peluru.kotak.y}, WHITE);
          }
        }

        // Gambar semua musuh aktif.
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

        // Gambar semua bintang aktif.
        for (const auto& bintang : koleksiBintang) {
          if (bintang.aktif) {
            DrawTextureRec(teksturBintang,
                           Rectangle{0.0f, 0.0f, (float)teksturBintang.width,
                                     (float)teksturBintang.height},
                           Vector2{bintang.kotak.x, bintang.kotak.y}, WHITE);
          }
        }

        // Gambar informasi skor, nyawa, dan peluru.
        DrawTextEx(fontKustom, TextFormat("Skor: %i", skorSaatIni),
                   Vector2{10, 10}, fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, TextFormat("Nyawa: %i", nyawaSaatIni),
                   Vector2{10, 40}, fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, TextFormat("Peluru: %i", jumlahPeluruSaatIni),
                   Vector2{10, 70}, fontKustom.baseSize, 2, WHITE);
        DrawFPS(SCREEN_WIDTH - 100, 10);  // Gambar FPS.
      } break;

      case GAME_OVER: {
        const char* teksGameOver = "GAME OVER!";
        const char* teksSkor = TextFormat("SKOR ANDA: %i", skorSaatIni);
        const char* teksLanjutkan =
            "Tekan ENTER atau KLIK untuk melihat Leaderboard";
        int lebarGameOver = MeasureTextEx(fontKustom, teksGameOver,
                                          fontKustom.baseSize * 1.5f, 2)
                                .x;
        int lebarSkor =
            MeasureTextEx(fontKustom, teksSkor, fontKustom.baseSize, 2).x;
        int lebarLanjutkan = MeasureTextEx(fontKustom, teksLanjutkan,
                                           fontKustom.baseSize * 0.8f, 2)
                                 .x;

        // Gambar teks "GAME OVER", skor, dan instruksi untuk melihat
        // leaderboard.
        DrawTextEx(fontJudul, teksGameOver,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarGameOver / 2,
                           (float)SCREEN_HEIGHT / 2 - 70},
                   fontKustom.baseSize * 1.5f, 2, WHITE);
        DrawTextEx(fontKustom, teksSkor,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarSkor / 2,
                           (float)SCREEN_HEIGHT / 2},
                   fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, teksLanjutkan,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarLanjutkan / 2,
                           (float)SCREEN_HEIGHT / 2 + 50},
                   fontKustom.baseSize * 0.8f, 2, WHITE);
      } break;

      case LEADERBOARD: {
        // Define Rectangles for the sort buttons
        Rectangle tombolUrutSkor = {(float)SCREEN_WIDTH / 2 + 245.0f,
                                    (float)SCREEN_HEIGHT - 430.0f, 140, 30};
        Rectangle tombolUrutNama = {(float)SCREEN_WIDTH / 2 + 245.0f,
                                    (float)SCREEN_HEIGHT - 390.0f, 140, 30};

        // Draw Sort by Score Button
        Color warnaTombolSkor =
            (CheckCollisionPointRec(GetMousePosition(), tombolUrutSkor))
                ? LIGHTGRAY
                : GRAY;
        DrawRectangleRec(tombolUrutSkor, warnaTombolSkor);
        DrawRectangleLinesEx(tombolUrutSkor, 2, WHITE);
        DrawTextEx(fontKustom, "Urutkan Skor",
                   Vector2{tombolUrutSkor.x + tombolUrutSkor.width / 2.0f -
                               MeasureTextEx(fontKustom, "Urutkan Skor",
                                             fontKustom.baseSize * 0.8f, 2)
                                       .x /
                                   2.0f,
                           tombolUrutSkor.y + tombolUrutSkor.height / 2.0f -
                               fontKustom.baseSize * 0.4f},
                   fontKustom.baseSize * 0.8f, 2, WHITE);

        // Draw Sort by Name Button
        Color warnaTombolNama =
            (CheckCollisionPointRec(GetMousePosition(), tombolUrutNama))
                ? LIGHTGRAY
                : GRAY;
        DrawRectangleRec(tombolUrutNama, warnaTombolNama);
        DrawRectangleLinesEx(tombolUrutNama, 2, WHITE);
        DrawTextEx(fontKustom, "Urutkan Nama",
                   Vector2{tombolUrutNama.x + tombolUrutNama.width / 2.0f -
                               MeasureTextEx(fontKustom, "Urutkan Nama",
                                             fontKustom.baseSize * 0.8f, 2)
                                       .x /
                                   2.0f,
                           tombolUrutNama.y + tombolUrutNama.height / 2.0f -
                               fontKustom.baseSize * 0.4f},
                   fontKustom.baseSize * 0.8f, 2, WHITE);

        const char* judulLeaderboard = "LEADERBOARD";
        int lebarJudul = MeasureTextEx(fontKustom, judulLeaderboard,
                                       fontKustom.baseSize * 1.5f, 2)
                             .x;
        // Gambar judul leaderboard.
        DrawTextEx(fontJudul, judulLeaderboard,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarJudul / 2, 30},
                   fontKustom.baseSize * 1.5f, 2, WHITE);

        // Gambar entri leaderboard.
        DrawLeaderboard(fontKustom, fontKustom.baseSize, WHITE, SCREEN_WIDTH,
                        SCREEN_HEIGHT);

        // Draw search input box and prompt
        const char* searchPrompt = "CARI NAMA (ENTER untuk mencari/TOP 10):";
        int searchPromptWidth = MeasureTextEx(fontKustom, searchPrompt,
                                              fontKustom.baseSize * 0.8f, 2)
                                    .x;
        DrawTextEx(fontKustom, searchPrompt,
                   Vector2{(float)SCREEN_WIDTH / 2 - searchPromptWidth / 2,
                           (float)SCREEN_HEIGHT - 380},
                   fontKustom.baseSize * 0.8f, 2, WHITE);
        // You had DrawRectangleLines commented out, I'm assuming you want to
        // keep it that way for the input box visuals.
        // DrawRectangleLines(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 40, 200,
        // 30, WHITE);
        DrawTextEx(fontKustom, searchBuffer,
                   Vector2{(float)SCREEN_WIDTH / 2 -
                               MeasureTextEx(fontKustom, searchBuffer,
                                             fontKustom.baseSize * 0.8f, 2)
                                       .x /
                                   2.0f,
                           (float)SCREEN_HEIGHT -
                               40},  // Adjusted Y to align better if no box
                                     // drawn or smaller box
                   fontKustom.baseSize * 0.8f, 2, WHITE);

        // Blinking cursor for search input
        if (((searchFramesCounter / 30) % 2) == 0) {
          DrawTextEx(
              fontKustom, "_",
              Vector2{(float)SCREEN_WIDTH / 2 -
                          MeasureTextEx(fontKustom, searchBuffer,
                                        fontKustom.baseSize * 0.8f, 2)
                                  .x /
                              2.0f +
                          MeasureTextEx(fontKustom, searchBuffer,
                                        fontKustom.baseSize * 0.8f, 2)
                              .x,
                      (float)SCREEN_HEIGHT - 40},  // Adjusted Y to match text
              fontKustom.baseSize * 0.8f, 2, WHITE);
        }

        // KEMBALI KE MENU button drawing
        Rectangle tombolKembaliKeMenu = {
            (float)SCREEN_WIDTH / 2 - 380, (float)SCREEN_HEIGHT - 40, 200,
            25};  // Adjusted Y position to be lower
        Color warnaTombolKembali =
            (CheckCollisionPointRec(GetMousePosition(), tombolKembaliKeMenu))
                ? LIGHTGRAY
                : GRAY;
        DrawRectangleRec(tombolKembaliKeMenu, warnaTombolKembali);
        DrawRectangleLinesEx(tombolKembaliKeMenu, 2, WHITE);
        const char* teksKembali = "KEMBALI KE MENU";
        int lebarTeksKembali = MeasureTextEx(fontKustom, teksKembali,
                                             fontKustom.baseSize * 0.8f, 2)
                                   .x;
        DrawTextEx(
            fontKustom, teksKembali,
            Vector2{tombolKembaliKeMenu.x + tombolKembaliKeMenu.width / 2.0f -
                        lebarTeksKembali / 2.0f,
                    tombolKembaliKeMenu.y + tombolKembaliKeMenu.height / 2.0f -
                        fontKustom.baseSize * 0.4f},
            fontKustom.baseSize * 0.8f, 2, WHITE);

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
  CloseDatabase();  // Tutup koneksi database saat keluar aplikasi
  CloseWindow();
  return 0;
}

// --- Definisi Fungsi untuk main.cpp ---
void ResetElemenPermainan() {
  skorSaatIni = 0;
  nyawaSaatIni = 1;
  jumlahPeluruSaatIni = 3;
  peluruPemain.clear();
  musuhMusuh.clear();
  koleksiBintang.clear();
  waktuSpawnMusuhTerakhir = GetTime();
  intervalSpawnMusuh = 2.0f;
  waktuSpawnBintangTerakhir = GetTime();
  intervalSpawnBintang = 5.0f;
}