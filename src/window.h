#ifndef WINDOW_H
#define WINDOW_H

#include <raylib.h>
/**
 * @file window.h
 * @brief Deklarasi konstanta dan fungsi untuk manajemen jendela game.
 *
 * File header ini mendefinisikan lebar, tinggi, dan judul jendela game,
 * serta mendeklarasikan fungsi untuk inisialisasi jendela Raylib.
 */

extern const int SCREEN_WIDTH;   // Lebar jendela game dalam piksel.
extern const int SCREEN_HEIGHT;  // Tinggi jendela game dalam piksel.
extern const char* JUDUL_GAME;   // String konstanta untuk judul jendela game.
extern const int KECEPATAN_PEMAIN;  // Kecepatan awal pemain (mungkin tidak
                                    // relevan jika dikelola di struct Pemain).

/**
 * @brief Menginisialisasi jendela game menggunakan Raylib.
 *
 * Fungsi ini mengatur bendera konfigurasi Raylib (misalnya, FLAG_MSAA_4X_HINT
 * untuk anti-aliasing) dan kemudian membuat jendela game dengan lebar,
 * tinggi, dan judul yang telah ditentukan.
 */
void InisialisasiGameWindow();
#endif  // WINDOW_H