# ⚠️ CRITICAL HARDWARE & DRIVER WARNINGS (ESP32-S3 + CO5300 + CST9217)

Dokumen ini mencatat ringkasan **root cause, bug fixes, dan panduan wajib** untuk integrasi hardware **ESP32-S3 AMOLED CO5300 + Touch IC CST9217** agar kesalahan yang sama tidak terulang di masa depan.

---

## 1. 🛑 USB CDC Serial Blocking (MCU Hang Saat Serial Monitor Ditutup)

### Masalah
Saat kabel USB tercolok tetapi **Serial Monitor tidak dibuka / ditutup**, ESP32-S3 mengalami *lockup / freeze* (layar berhenti merespons sentuhan atau fungsi clear macet).

### Penyebab
ESP32-S3 menggunakan native USB CDC (`HWCDC`). Jika tidak ada host (komputer) yang aktif mengonsumsi data serial, buffer TX USB CDC akan penuh. Panggilan `Serial.println()` atau `Serial.printf()` secara default akan **memblokir eksekusi CPU (blocking)** sampai buffer kosong.

### Solusi & Pencegahan Wajib
1. Di `setup()`, panggil:
   ```cpp
   Serial.setTxTimeoutMs(0);
   Serial.begin(115200);
   Serial.setTxTimeoutMs(0);
   ```
2. **JANGAN PERNAH** memanggil `Serial.println()` secara telanjang di `loop()` atau FreeRTOS task.
3. Selalu bungkus dengan proteksi buffer:
   ```cpp
   if (Serial && Serial.availableForWrite() > 32) {
       Serial.println("Log message...");
   }
   ```
4. *Catatan:* Jangan gunakan `Serial.dtr()` karena tidak didukung pada HWCDC ESP32-S3.

---

## 2. 🎨 Image Color Distortion (Warna Biru Berubah Menjadi Hijau Terang)

### Masalah
Saat me-render gambar full-screen (misal 410x502 RGB565) dari array C/Flash hasil export UI Generator (SquareLine Studio / WatchForge / LVGL), warna **Biru / Ungu Neon berubah menjadi Hijau Terang**.

### Penyebab (Endianness Mismatch & Double Byte-Swap)
- Array gambar hasil export LVGL/SquareLine tersimpan dalam format **Big-Endian RGB565** (`[Byte0 = MSB, Byte1 = LSB]`).
- Fungsi `gfx->draw16bitRGBBitmap()` mengasumsikan input berupa Little-Endian dan melakukan byte-swap otomatis sebelum dikirim ke bus SPI.
- Akibatnya terjadi **double byte-swap**, menyebabkan 5-bit Biru (bit 0–4) bergeser masuk ke 6-bit Hijau (bit 5–10).

### Solusi & Pencegahan Wajib
Gunakan fungsi **`draw16bitBeRGBBitmap`** bawaan `Arduino_GFX`:
```cpp
// BENAR (Direct Single-Burst Hardware DMA Big-Endian):
gfx->draw16bitBeRGBBitmap(x, y, (uint16_t *)bitmap_map, w, h);

// SALAH (Menyebabkan double swap & warna rusak):
gfx->draw16bitRGBBitmap(x, y, (uint16_t *)bitmap_map, w, h);
```

---

## 3. 👆 CST9217 Low-Level I2C Driver & Multi-Stroke Stability

### Masalah
Sentuhan jari sering tidak terdeteksi, macet setelah jari diangkat (*lift detection fail*), atau membutuhkan library eksternal yang lambat.

### Solusi & Register Map Wajib
Gunakan native driver direct I2C (`Wire`) tanpa library pihak ketiga:
1. **I2C Config**: Address `0x5A`, Fast Mode clock `400000` (400kHz), Timeout `30ms`.
2. **Inisialisasi Command Mode**: Tulis `0x01` ke register `0xD101`:
   ```cpp
   Wire.beginTransmission(0x5A);
   Wire.write(0xD1);
   Wire.write(0x01);
   Wire.write(0x01);
   Wire.endTransmission(true);
   ```
3. **Membaca Koordinat Touch Point**:
   - Tulis register 16-bit `0xD000`.
   - Beri delay `1000us` (1ms).
   - Request 10 byte.
   - Validasi byte status: `status = buf[0] & 0x0F` (`0x06` = active touch, `0x01` = contact).
   - Validasi ACK byte: `buf[6] == 0xAB`.
   - Parsing koordinat:
     $$\text{X} = (\text{buf}[1] \ll 4) \mid (\text{buf}[3] \gg 4)$$
     $$\text{Y} = (\text{buf}[2] \ll 4) \mid (\text{buf}[3] \ \& \ \text{0x0F})$$
4. **Polling Rate**: Jalankan di FreeRTOS Task (Core 0, Priority 3) dengan interval `vTaskDelay(pdMS_TO_TICKS(8))` (125 Hz). Lindungi akses I2C dengan `i2c_bus_mutex`.

---

## 4. ✏️ DDA Line Interpolation (Mencegah Coretan Kuas Berlubang/Putus)

### Masalah
Saat menggeser jari dengan cepat pada kuas berukuran kecil (3px - 6px), garis yang digambar terputus-putus atau memiliki celah/gap.

### Solusi & Pencegahan Wajib
Jangan hanya menggambar lingkaran di titik $(X, Y)$ terkini. Gunakan algoritma interpolasi garis **DDA (Digital Differential Analyzer)** yang menghubungkan $(X_{prev}, Y_{prev})$ ke $(X_{curr}, Y_{curr})$:
```cpp
void display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t thickness) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t steps = (dx > dy) ? dx : dy;
    if (steps == 0) {
        gfx->fillCircle(x0, y0, thickness, color);
        return;
    }
    float xInc = (float)(x1 - x0) / steps;
    float yInc = (float)(y1 - y0) / steps;
    float x = x0, y = y0;
    for (int i = 0; i <= steps; i++) {
        gfx->fillCircle((int16_t)x, (int16_t)y, thickness, color);
        x += xInc;
        y += yInc;
    }
}
```

---

## 5. ⚡ QSPI Clock Speed: Jumper Wires vs Custom PCB

- **Jumper Wire / Breadboard**: Gunakan `20000000` (20MHz).
- **Custom PCB (Final Hardware)**: Gunakan `80000000` (80MHz). Jalur PCB mampu menangani 80MHz SPI DMA tanpa noise, menghasilkan kecepatan refresh hingga **300+ FPS** untuk partial render dan **30 FPS** untuk full-screen image frame refresh.

---

## 6. 🚫 Cyan 0x07FF Sentinel Hang Bug

Di `Arduino_GFX`, nilai warna `0x07FF` (Cyan murni) digunakan sebagai *sentinel value* internal yang memicu hang pada driver CO5300.
**Selalu ganti warna Cyan menjadi `0x07FE`:**
```cpp
#define COLOR_CYAN_FIX 0x07FE
if (color == 0x07FF) color = COLOR_CYAN_FIX;
```
