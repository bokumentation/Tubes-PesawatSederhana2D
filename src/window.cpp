#include "window.h"  // Sertakan header window.h kustom Anda
/**
 * @file window.cpp
 * @brief Implementasi fungsi dan definisi konstanta untuk manajemen jendela
 * game.
 *
 * File ini berisi definisi konstanta SCREEN_WIDTH, SCREEN_HEIGHT, JUDUL_GAME,
 * dan KECEPATAN_PEMAIN, serta implementasi fungsi InisialisasiGameWindow().
 */

/**
 * @brief Konstanta lebar jendela game.
 *
 * Ditetapkan sebesar 800 piksel.
 */
const int SCREEN_WIDTH = 800;

/**
 * @brief Konstanta tinggi jendela game.
 *
 * Ditetapkan sebesar 450 piksel.
 */
const int SCREEN_HEIGHT = 450;

/**
 * @brief Konstanta judul jendela game.
 *
 * Akan ditampilkan di bagian atas jendela game.
 */
const char* JUDUL_GAME =
    "PesawatSederhana 2D (tekan ESC untuk menutup jendela)";  // GAME_TITLE ->
                                                              // JUDUL_GAME

/**
 * @brief Konstanta kecepatan pemain.
 *
 * Konstanta ini mungkin kurang relevan sekarang karena kecepatan pemain
 * dikelola di dalam struct Pemain di main.cpp.
 */
const int KECEPATAN_PEMAIN = 5;  // Konstanta ini kurang relevan sekarang karena
                                 // kecepatan ada di struct Pemain

/**
 * @brief Menginisialisasi jendela game menggunakan Raylib.
 *
 * Fungsi ini mengatur bendera konfigurasi untuk Raylib, seperti
 * FLAG_MSAA_4X_HINT untuk mengaktifkan 4x Multi-Sample Anti-Aliasing,
 * dan kemudian memanggil InitWindow() untuk membuat jendela dengan
 * dimensi dan judul yang telah ditentukan.
 */
void InisialisasiGameWindow() {
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, JUDUL_GAME);
}