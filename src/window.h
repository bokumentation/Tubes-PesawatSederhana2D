#ifndef WINDOW_H
#define WINDOW_H

#include <raylib.h>

// Lebar Window
extern const int SCREEN_WIDTH;
// Tinggi Window
extern const int SCREEN_HEIGHT;
// Judul game di window
extern const char* JUDUL_GAME;  // GAME_TITLE -> JUDUL_GAME
// Kecepatan pemain (Anda mungkin memindahkan ini ke struct Pemain seperti di
// main.cpp yang diperbarui)
extern const int KECEPATAN_PEMAIN;  // PLAYER_SPEED -> KECEPATAN_PEMAIN

// Fungsi inisialisasi Raylib window
void InisialisasiGameWindow();
#endif  // WINDOW_H