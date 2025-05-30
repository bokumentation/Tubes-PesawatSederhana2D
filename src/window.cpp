#include "window.h"  // Include your custom window.h header

// Define and initialize the constants here.
// These are the actual definitions for the extern declarations in window.h
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const char* GAME_TITLE = "PesawatSederhana 2D (press ESC to close window)";
const int PLAYER_SPEED = 5;

// Function to initialize the window
void InisialisasiGameWindow() {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
}
