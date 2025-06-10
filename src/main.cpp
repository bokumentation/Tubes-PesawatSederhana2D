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

#include "player_data.h"  // BARU: Sertakan data pemain dan header leaderboard kita
#include "window.h"       // Header untuk InisialisasiGameWindow()

// --- Status Game ---
enum GameState { LAYAR_JUDUL, MASUKAN_NAMA, PERMAINAN, GAME_OVER, LEADERBOARD };

// --- Struktur Pemain ---
struct Pemain {
  Rectangle kotak;
  Color warna;
};

// --- Struktur Peluru ---
struct Peluru {
  Rectangle kotak;
  float kecepatan;
  Color warna;
  bool aktif;
};

// --- Struktur Musuh ---
struct Musuh {
  Rectangle kotak;
  Color warna;
  float kecepatan;
  int kesehatan;
  bool aktif;
};

// -- Struktur Bintang --
struct Bintang {
  Rectangle kotak;
  Color warna;
  float kecepatan;
  bool aktif;
};

// --- Variabel Status Game Global ---
GameState statusGameSaatIni = LAYAR_JUDUL;
int skorSaatIni =
    0;  // Diubah namanya agar tidak bentrok dengan g_score di player_data
int nyawaSaatIni = 1;          // Diubah namanya agar lebih jelas
int jumlahPeluruSaatIni = 50;  // BARU: Jumlah peluru awal

// Untuk kursor berkedip (tetap di main untuk kesederhanaan karena ini spesifik
// UI)
int hitunganBingkai = 0;

// --- Variabel khusus permainan (bukan bagian dari player_data) ---
std::vector<Peluru> peluruPemain;
std::vector<Musuh> musuhMusuh;
std::vector<Bintang> koleksiBintang;  // BARU: Vektor untuk bintang

float gulirBelakang = 0.0f;
float gulirTengah = 0.0f;
float gulirDepan = 0.0f;

Texture2D teksturLatarBelakang;  // Dideklarasikan di sini, dimuat di main

// Pembangkitan angka acak untuk spawn musuh
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> spawnYMusuh(0, SCREEN_HEIGHT - 50);
std::uniform_real_distribution<> kecepatanMusuh(100.0f, 250.0f);

// BARU: Pembangkitan angka acak untuk spawn Bintang
std::uniform_int_distribution<> spawnYBintang(
    0, SCREEN_HEIGHT - 30);  // Rentang lebih kecil untuk bintang
std::uniform_real_distribution<> kecepatanBintang(
    80.0f, 180.0f);  // Lebih lambat dari musuh

// Timer untuk spawn musuh
double waktuSpawnMusuhTerakhir = 0.0;
float intervalSpawnMusuh = 2.0f;

// BARU: Timer untuk spawn Bintang
double waktuSpawnBintangTerakhir = 0.0;
float intervalSpawnBintang = 5.0f;  // Spawn bintang setiap 5 detik

// -- Variabel Baru untuk laju tembak pemain

// Waktu sejak peluru terakhir ditembakkan
double waktuTembakTerakhir = 0.0;
// Detik antara tembakan (misal: 0.15s untuk tembakan cepat)
float lajuTembakPemain = 0.15f;

// --- Prototipe Fungsi (untuk tanggung jawab main.cpp) ---
void ResetElemenPermainan();  // Mengatur ulang hanya elemen khusus game

int main() {
  //                                //
  // 1. INISIALISASI                //
  //                                //

  InisialisasiGameWindow();

  Font fontKustom =
      LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 20, 0, 0);
  SetTextureFilter(fontKustom.texture, TEXTURE_FILTER_BILINEAR);

  Font fontJudul =
      LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 30, 0, 0);
  SetTextureFilter(fontJudul.texture, TEXTURE_FILTER_BILINEAR);

  // Muat tekstur (lebih baik dilakukan sekali)
  teksturLatarBelakang = LoadTexture("assets/background.png");

  // Inisialisasi pemain (status awal, akan diatur ulang saat game dimulai)
  Pemain pemain;  // Objek pemain tetap di main atau manajer game
  pemain.kotak = {(float)SCREEN_WIDTH / 2 - 25, (float)SCREEN_HEIGHT / 2 - 25,
                  50, 50};
  pemain.warna = BLUE;

  // Inisialisasi data pemain dan muat leaderboard
  InitPlayerData();  // Panggil fungsi dari player_data.cpp

  HideCursor();
  SetTargetFPS(60);

  //                                //
  // 2. GAME LOOP                   //
  //                                //

  while (!WindowShouldClose()) {
    float deltaWaktu = GetFrameTime();
    hitunganBingkai++;

    // --- Logika Berdasarkan Status ---
    switch (statusGameSaatIni) {
      case LAYAR_JUDUL: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          statusGameSaatIni = MASUKAN_NAMA;
          g_namaPemain = "";  // Bersihkan nama untuk masukan baru
          memset(g_bufferNama, 0, sizeof(g_bufferNama));  // Bersihkan buffer
        }
      } break;

      case MASUKAN_NAMA: {
        int kunci = GetCharPressed();
        while (kunci > 0) {
          if ((kunci >= 32) && (kunci <= 125) && (strlen(g_bufferNama) < 31)) {
            g_bufferNama[strlen(g_bufferNama)] =
                (char)kunci;  // Tambahkan karakter ke buffer
          }
          kunci = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
          if (strlen(g_bufferNama) > 0) {
            g_bufferNama[strlen(g_bufferNama) - 1] =
                '\0';  // Hapus karakter terakhir
          }
        }

        if (IsKeyPressed(KEY_ENTER)) {
          if (strlen(g_bufferNama) > 0) {
            g_namaPemain = std::string(g_bufferNama);
            ResetElemenPermainan();  // Atur ulang elemen khusus permainan
            statusGameSaatIni = PERMAINAN;
          }
        }
      } break;

      case PERMAINAN: {
        Vector2 posisiMouse = GetMousePosition();
        pemain.kotak.x = posisiMouse.x - pemain.kotak.width / 2;
        pemain.kotak.y = posisiMouse.y - pemain.kotak.height / 2;

        if (pemain.kotak.x < 0) {
          pemain.kotak.x = 0;
        };
        if (pemain.kotak.x + pemain.kotak.width > SCREEN_WIDTH) {
          pemain.kotak.x = SCREEN_WIDTH - pemain.kotak.width;
        };
        if (pemain.kotak.y < 0) {
          pemain.kotak.y = 0;
        };
        if (pemain.kotak.y + pemain.kotak.height > SCREEN_HEIGHT) {
          pemain.kotak.y = SCREEN_HEIGHT - pemain.kotak.height;
        };

        // BARU: Penembakan peluru dengan pemeriksaan jumlah peluru
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) &&
            (GetTime() - waktuTembakTerakhir >= lajuTembakPemain) &&
            jumlahPeluruSaatIni > 0) {  // Periksa apakah peluru tersedia
          Peluru peluruBaru;
          peluruBaru.kotak = {pemain.kotak.x + pemain.kotak.width,
                              pemain.kotak.y + pemain.kotak.height / 2 - 5, 20,
                              10};
          peluruBaru.kecepatan = 1000;
          peluruBaru.warna = RED;
          peluruBaru.aktif = true;
          peluruPemain.push_back(peluruBaru);

          jumlahPeluruSaatIni--;  // Kurangi jumlah peluru
          waktuTembakTerakhir = GetTime();
        }

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

        if (GetTime() - waktuSpawnMusuhTerakhir > intervalSpawnMusuh) {
          Musuh musuhBaru;
          musuhBaru.kotak = {(float)SCREEN_WIDTH, (float)spawnYMusuh(gen), 50,
                             50};
          musuhBaru.warna = LIME;
          musuhBaru.kecepatan = kecepatanMusuh(gen);
          musuhBaru.kesehatan = 1;
          musuhBaru.aktif = true;
          musuhMusuh.push_back(musuhBaru);
          waktuSpawnMusuhTerakhir = GetTime();
          intervalSpawnMusuh = GetRandomValue(50, 150) / 100.0f;
        }

        // BARU: Logika Spawn Bintang
        if (GetTime() - waktuSpawnBintangTerakhir > intervalSpawnBintang) {
          Bintang bintangBaru;
          bintangBaru.kotak = {(float)SCREEN_WIDTH, (float)spawnYBintang(gen),
                               20,  // Ukuran lebih kecil untuk bintang
                               20};
          bintangBaru.warna = YELLOW;  // Jadikan kuning
          bintangBaru.kecepatan = kecepatanBintang(gen);
          bintangBaru.aktif = true;
          koleksiBintang.push_back(bintangBaru);
          waktuSpawnBintangTerakhir = GetTime();
          intervalSpawnBintang =
              GetRandomValue(400, 700) / 100.0f;  // Acak interval
        }

        for (auto& musuh : musuhMusuh) {
          if (musuh.aktif) {
            musuh.kotak.x -= musuh.kecepatan * deltaWaktu;
            if (musuh.kotak.x + musuh.kotak.width < 0) {
              musuh.aktif = false;
            }

            if (CheckCollisionRecs(pemain.kotak, musuh.kotak)) {
              nyawaSaatIni--;  // Gunakan nyawaSaatIni
              musuh.aktif = false;
              if (nyawaSaatIni <= 0) {  // Periksa nyawaSaatIni
                AddScoreToLeaderboard(
                    g_namaPemain,
                    skorSaatIni);  // Gunakan g_namaPemain dan skorSaatIni
                statusGameSaatIni = GAME_OVER;
              }
            }
          }
        }
        musuhMusuh.erase(
            std::remove_if(musuhMusuh.begin(), musuhMusuh.end(),
                           [](const Musuh& e) { return !e.aktif; }),
            musuhMusuh.end());

        // BARU: Pembaruan Bintang dan Tabrakan Pemain
        for (auto& bintang : koleksiBintang) {
          if (bintang.aktif) {
            bintang.kotak.x -= bintang.kecepatan * deltaWaktu;
            if (bintang.kotak.x + bintang.kotak.width < 0) {
              bintang.aktif = false;  // Nonaktifkan jika di luar layar
            }

            // Periksa tabrakan dengan pemain
            if (CheckCollisionRecs(pemain.kotak, bintang.kotak)) {
              jumlahPeluruSaatIni += 5;  // Dapatkan 5 peluru per bintang
              bintang.aktif = false;     // Nonaktifkan bintang yang terkumpul
            }
          }
        }
        koleksiBintang.erase(
            std::remove_if(koleksiBintang.begin(), koleksiBintang.end(),
                           [](const Bintang& b) { return !b.aktif; }),
            koleksiBintang.end());

        for (auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            for (auto& musuh : musuhMusuh) {
              if (musuh.aktif &&
                  CheckCollisionRecs(peluru.kotak, musuh.kotak)) {
                musuh.kesehatan--;
                peluru.aktif = false;
                if (musuh.kesehatan <= 0) {
                  musuh.aktif = false;
                  skorSaatIni += 10;  // Gunakan skorSaatIni
                }
                break;
              }
            }
          }
        }

        gulirBelakang -= 10.0f * deltaWaktu;
        gulirTengah -= 20.0f * deltaWaktu;
        gulirDepan -= 30.0f * deltaWaktu;

        if (gulirBelakang <= -teksturLatarBelakang.width) gulirBelakang = 0;
      } break;

      case GAME_OVER: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          statusGameSaatIni = LEADERBOARD;
        }
      } break;

      case LEADERBOARD: {
        if (IsKeyPressed(KEY_ENTER) ||
            IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
          statusGameSaatIni = LAYAR_JUDUL;
        }
      } break;
    }

    // --- Menggambar ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Gambar latar belakang terlepas dari status
    DrawTextureEx(teksturLatarBelakang, (Vector2){gulirBelakang, 0}, 0.0f, 1.0f,
                  WHITE);
    DrawTextureEx(teksturLatarBelakang,
                  (Vector2){teksturLatarBelakang.width + gulirBelakang, 0},
                  0.0f, 1.0f, WHITE);

    switch (statusGameSaatIni) {
      case LAYAR_JUDUL: {
        const char* teksJudul = "PESAWAT SEDERHANA 2D";
        const char* teksMulai = "Tekan ENTER atau KLIK untuk Memulai";
        int lebarJudul =
            MeasureTextEx(fontJudul, teksJudul, fontKustom.baseSize * 1.5, 2).x;
        int lebarMulai =
            MeasureTextEx(fontKustom, teksMulai, fontKustom.baseSize, 2).x;

        DrawTextEx(fontJudul, teksJudul,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarJudul / 2,
                           (float)SCREEN_HEIGHT / 2 - 50},
                   fontKustom.baseSize * 1.5, 2, WHITE);
        DrawTextEx(fontKustom, teksMulai,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarMulai / 2,
                           (float)SCREEN_HEIGHT / 2 + 20},
                   fontKustom.baseSize, 2, WHITE);
      } break;

      case MASUKAN_NAMA: {
        const char* teksPrompt = "MASUKKAN NAMA ANDA:";
        int lebarPrompt =
            MeasureTextEx(fontKustom, teksPrompt, fontKustom.baseSize, 2).x;
        int lebarBufferNama =
            MeasureTextEx(fontKustom, g_bufferNama, fontKustom.baseSize, 2)
                .x;  // Gunakan g_bufferNama

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
        DrawRectangleRec(pemain.kotak, pemain.warna);

        for (const auto& peluru : peluruPemain) {
          if (peluru.aktif) {
            DrawRectangleRec(peluru.kotak, peluru.warna);
          }
        }

        for (const auto& musuh : musuhMusuh) {
          if (musuh.aktif) {
            DrawRectangleRec(musuh.kotak, musuh.warna);
          }
        }

        // BARU: Gambar koleksi Bintang
        for (const auto& bintang : koleksiBintang) {
          if (bintang.aktif) {
            // Anda bisa menggambar bentuk atau tekstur berbeda untuk bintang
            // Untuk saat ini, kita gambar persegi panjang kecil berwarna kuning
            DrawRectangleRec(bintang.kotak, bintang.warna);
          }
        }

        DrawTextEx(fontKustom, TextFormat("Skor: %i", skorSaatIni),
                   Vector2{10, 10},  // Gunakan skorSaatIni
                   fontKustom.baseSize, 2, WHITE);
        DrawTextEx(fontKustom, TextFormat("Nyawa: %i", nyawaSaatIni),
                   Vector2{10, 40},  // Gunakan nyawaSaatIni
                   fontKustom.baseSize, 2, WHITE);
        DrawTextEx(
            fontKustom,
            TextFormat("Peluru: %i",
                       jumlahPeluruSaatIni),  // BARU: Tampilkan jumlah peluru
            Vector2{10, 70}, fontKustom.baseSize, 2, WHITE);
        DrawFPS(SCREEN_WIDTH - 100, 10);
      } break;

      case GAME_OVER: {
        const char* teksGameOver = "GAME OVER!";
        const char* teksSkor =
            TextFormat("SKOR ANDA: %i", skorSaatIni);  // Gunakan skorSaatIni
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

        DrawLeaderboard(fontKustom, fontKustom.baseSize, WHITE, SCREEN_WIDTH,
                        SCREEN_HEIGHT);  // Lewatkan dimensi layar

        const char* teksKembali = "Tekan ENTER atau KLIK untuk ke Menu Utama";
        int lebarKembali =
            MeasureTextEx(fontKustom, teksKembali, fontKustom.baseSize * 0.8, 2)
                .x;
        DrawTextEx(fontKustom, teksKembali,
                   Vector2{(float)SCREEN_WIDTH / 2 - lebarKembali / 2,
                           (float)SCREEN_HEIGHT - 50},
                   fontKustom.baseSize * 0.8, 2, WHITE);
      } break;
    }

    EndDrawing();
  }

  //                                //
  // 5. De-Inisialisasi             //
  //                                //

  UnloadTexture(teksturLatarBelakang);
  UnloadFont(fontKustom);
  UnloadFont(fontJudul);
  CloseWindow();
  return 0;
}

// --- Definisi Fungsi untuk main.cpp ---
void ResetElemenPermainan() {
  skorSaatIni = 0;
  nyawaSaatIni = 1;
  jumlahPeluruSaatIni = 50;  // BARU: Atur ulang peluru
  peluruPemain.clear();
  musuhMusuh.clear();
  koleksiBintang.clear();  // BARU: Bersihkan bintang saat reset
  // Atur ulang timer spawn musuh juga untuk permulaan yang baru
  waktuSpawnMusuhTerakhir = GetTime();
  intervalSpawnMusuh = 2.0f;
  waktuSpawnBintangTerakhir =
      GetTime();                // BARU: Atur ulang timer spawn bintang
  intervalSpawnBintang = 5.0f;  // BARU: Atur ulang interval spawn bintang
}