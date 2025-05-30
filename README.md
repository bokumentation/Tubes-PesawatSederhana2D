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
Gunakan **[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)**
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


### Cara Memperbarui Folder Proyek dari Repo yang Sudah *Diupdate*
**Terdapat 2 pilihan** untuk memperbarui folder proyek yang sudah diclone.
- Pilihan 1: Jika tidak ngapa-ngapain
  ```bash
  git stash
  git pull origin main
  ```
- Pilihan 2: Jika sudah menambahkan kode
  ```bash
  git add src/main.cpp
  git commit -m "ini perubahan dari lokal saya"
  git pull origin main
  ```
  > Git will attempt to merge your committed local changes with the incoming changes. If there are conflicts, Git will guide you through resolving them.

Jika sudah diupdate, maka perlu **re-run CMake build commands**.
```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```