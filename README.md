# Tugas Besar Alpro Lanjut

## Membuat Game Pesawat Sederhana
Proyek ini adalah implementasi game pesawat sederhana yang terinspirasi dari game "Platypus".

### Persiapan yang Dibutuhkan
- **GCC Compiler dari MinGW**
- **CMake**:  Alat untuk membantu proses "membangun" (build) program dari kode sumber.
- **Git dan Github**: Untuk mengelola dan mengambil kode program dari internet.
- **VS Code** dan Extensinya
  - **CMake Extension**: Mempermudah proses build langsung dari VS Code.
  - **C/C++ Extension by Microsoft**: Menyediakan fitur seperti intellisense (bantuan penulisan kode), debugging, dan navigasi kode.

### Standarisasi Kode
Gunakan **Google C++ Style Guide**
dengan Spaces atau Tab Display Size: 2

### Cara Build Program

1. Clone repositori.
```bash
git clone https://github.com/bokumentation/Tubes-PesawatSederhana2D.git
```
2. Masuk ke folder program
```bash
cd .\Tubes-PesawatSederhana2D\ 
```
3. Proses build dengan CMake
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```
4. Menjalankan program
```bash
.\build\pesawat2d.exe
```