#ifndef WINDOW_H
#define WINDOW_H

#include <raylib.h>

// Lebar Window
extern const int SCREEN_WIDTH;
// Tinggi Window
extern const int SCREEN_HEIGHT;
// Judul game di window
extern const char* GAME_TITLE;
// Player speed (you might move this to the Player struct as in the updated
// main.cpp)
extern const int PLAYER_SPEED;

// Fungsi inisialisasi Raylib window
void InisialisasiGameWindow();
#endif  // WINDOW_H