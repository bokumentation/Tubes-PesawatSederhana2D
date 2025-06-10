#include "window.h"  // Sertakan header window.h kustom Anda

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const char* JUDUL_GAME =
    "PesawatSederhana 2D (tekan ESC untuk menutup jendela)";  // GAME_TITLE ->
                                                              // JUDUL_GAME
const int KECEPATAN_PEMAIN = 5;  // Konstanta ini kurang relevan sekarang karena
                                 // kecepatan ada di struct Pemain

void InisialisasiGameWindow() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, JUDUL_GAME);
}