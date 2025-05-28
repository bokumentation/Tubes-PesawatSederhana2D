#include <raylib.h>  // Include the Raylib header

// You can define game constants here
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const char* GAME_TITLE = "Simple Plane Game";
const int PLAYER_SPEED = 5;  // Pixels per frame

// Define a simple structure for your player (the plane)
struct Player {
  Rectangle rect;  // Position and size
  Color color;     // Color of the player
};

int main() {
  // 1. Initialization
  // Initialize the window with specified dimensions and title
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);

  // Initialize your player
  Player player;
  player.rect = {(float)SCREEN_WIDTH / 2 - 25, (float)SCREEN_HEIGHT / 2 - 25,
                 50, 50};  // Centered 50x50 square
  player.color = BLUE;     // Blue plane

  // Set the target frames-per-second (FPS)
  SetTargetFPS(60);

  // 2. Game Loop
  // Loop as long as the window is not closed (or ESC is pressed)
  while (!WindowShouldClose())  // Detect window close button or ESC key
  {
    // 3. Update (Game Logic)
    // Move player based on arrow key input
    if (IsKeyDown(KEY_RIGHT)) {
      player.rect.x += PLAYER_SPEED;
    }
    if (IsKeyDown(KEY_LEFT)) {
      player.rect.x -= PLAYER_SPEED;
    }
    if (IsKeyDown(KEY_UP)) {
      player.rect.y -= PLAYER_SPEED;
    }
    if (IsKeyDown(KEY_DOWN)) {
      player.rect.y += PLAYER_SPEED;
    }

    // Keep player within screen bounds
    if (player.rect.x < 0) player.rect.x = 0;
    if (player.rect.x + player.rect.width > SCREEN_WIDTH)
      player.rect.x = SCREEN_WIDTH - player.rect.width;
    if (player.rect.y < 0) player.rect.y = 0;
    if (player.rect.y + player.rect.height > SCREEN_HEIGHT)
      player.rect.y = SCREEN_HEIGHT - player.rect.height;

    // 4. Drawing
    BeginDrawing();  // Start drawing operations

    ClearBackground(RAYWHITE);  // Clear background with a light white color

    // Draw the player (a rectangle for now)
    DrawRectangleRec(player.rect, player.color);

    // Draw some text for debugging or info
    DrawText("Move the blue square with arrow keys!", 10, 10, 20, DARKGRAY);
    DrawFPS(SCREEN_WIDTH - 100, 10);  // Display FPS

    EndDrawing();  // End drawing operations
  }

  // 5. De-Initialization
  // Close window and unload OpenGL context
  CloseWindow();

  return 0;
}