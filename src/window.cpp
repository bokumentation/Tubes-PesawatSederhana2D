#include "window.h"  // Include your custom window.h header

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const char* GAME_TITLE = "PesawatSederhana 2D (press ESC to close window)";
const int PLAYER_SPEED = 5;

void InisialisasiGameWindow() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
}
