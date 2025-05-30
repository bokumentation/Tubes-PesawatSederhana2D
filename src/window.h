#ifndef WINDOW_H
#define WINDOW_H

#include <raylib.h>

// Declare these as external constants
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const char* GAME_TITLE;
extern const int PLAYER_SPEED;

// Function to initialize the Raylib window (if you still want this)
void InisialisasiGameWindow();  // No parameters needed if using global
                                // constants

#endif  // WINDOW_H