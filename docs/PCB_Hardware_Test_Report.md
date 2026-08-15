> [!IMPORTANT]
> **RULE DOKUMENTASI:** Document / report ini **TIDAK BOLEH DIHAPUS**, HANYA BOLEH DITAMBAH (Append-Only) untuk mencatat riwayat pengujian hardware dan perkembangan PCB.

# Laporan Pengujian Hardware PCB Custom ESP32-S3

**Tanggal & Waktu:** 4 Agustus 2026  
**Board Target:** ESP32-S3-DevKitC-1 (N16R8)  
**Port Serial:** COM24 (Baud Rate: 115200)  
**Status Pengujian:** BERHASIL (Sistem Stabil, No Crash)

---

## 1. Analisis Akar Masalah (Root Cause Analysis - Bug Fix Crash Previous)

Sebelumnya terjadi crash/reset berulang pada ESP32-S3 di tengah pengujian. Berikut analisis teknisnya:

1. **Penggunaan `Wire.end()` + `Wire.begin()` Berulang**:
   - Panggilan `Wire.end()` yang dipadu dengan `Wire.begin()` berulang kali dalam `loop()` merusak *driver handle* I2C internal ESP-IDF (`ESP_ERR_INVALID_STATE`), yang memicu *StoreProhibited / Hardware Panic Reset*.
2. **Ketiadaan Timeout I2C Bus**:
   - Pemindaian I2C tanpa `Wire.setTimeOut(30)` menyebabkan pengontrol I2C hardware ESP32 *hanging/blocking* saat mengirim ACK/NACK ke alamat yang tidak terhubung. Hal ini memicu **Task Watchdog Timer (TWDT)** untuk melakukan *hard reset*.
3. **Pin Hardware Reset (GPIO 3)**:
   - Jalur reset fisik IC di PCB (GPIO 3) belum di-toggle `HIGH` di awal pembacaan, sehingga beberapa IC periferal belum aktif.

---

## 2. Hasil Pengujian Live Port (COM24)

### A. Pengujian Tombol (Active LOW - Internal Pull-up)
- **GPIO 7 (Button 1)**: Berfungsi normal (Log melepaskan `UP(1)` dan menekan `PRESSED[0]`).
- **GPIO 16 (Button 2)**: **100% Terverifikasi Berhasil** (Responsif penuh saat ditekan dan dilepas pada tes Serial live).
- **GPIO 5 (Button 3)**: Berfungsi normal (Log `UP(1)` / `PRESSED[0]`).

### B. Pengujian Pinout I2C PCB (`SDA = GPIO 48`, `SCL = GPIO 47`)
- **Tegangan Jalur Bus**: Jalur SDA (GPIO 48) dan SCL (GPIO 47) terdeteksi `HIGH (OK)`, menandakan resistor *pull-up* fisik pada PCB terpasang baik.
- **Toggle Reset Hardware (GPIO 3)**: GPIO 3 berhasil di-toggle `LOW` (20ms) lalu `HIGH` (100ms) di `setup()`.
- **Hasil Probing IC Target**:
  - `MAX17048` (Fuel Gauge, `0x36`): Respon NACK (`[!] no response`)
  - `LSM6DSO` (Gyro/Accel, `0x6A` / `0x6B`): Respon NACK (`[!] no response`)
  - `CST9217` (Touch Panel, `0x5A`): Respon NACK (`[!] no response`)

---

## 3. Langkah Diagnostik Lanjutan untuk Periferal I2C PCB

1. **Cek Jalur Power Enable (VDD/VCC Sensor)**:
   - Verifikasi apakah ada pin GPIO pengontrol daya (misal GPIO 6, 15, 38, 46) yang perlu di-set `HIGH` untuk menyalurkan daya 3.3V ke sensor MAX17048 dan LSM6DSO.
2. **Cek Skematik Jalur SDA/SCL**:
   - Konfirmasi apakah sensor MAX17048 & LSM6DSO terhubung ke bus I2C utama (GPIO 48/47) atau menggunakan pasangan GPIO terpisah (misal GPIO 1/2, 17/18, 39/40).

---

## 4. Pengujian Layar AMOLED (CO5300 QSPI) & Penemuan Periferal I2C (14 Agustus 2026)

**Status:** **100% SUKSES & HANG-FREE**

### A. Pengujian Layar AMOLED (CO5300AF-51 410x502px)
- **Koneksi Bus QSPI**: Inisialisasi driver `Arduino_CO5300` di frekuensi 20MHz berjalan sempurna (`[OK] Display initialized`).
- **Uji Warna & Render Bitmap**: 
  - Siklus otomatis kartu uji warna (*Test Card*) berjalan lancar di layar: **RED (0xF800)**, **GREEN (0x07E0)**, **BLUE (0x001F)**, **YELLOW (0xFFE0)**, dan **CYAN (0x07FE)**.
  - Workaround penanganan warna Cyan (`0x07FE`) berfungsi baik tanpa mengalami stall/hang.
- **Interaksi Tombol Fisik ke Layar**:
  - Menekan **GPIO 7** -> Layar berubah Merah secara *real-time*.
  - Menekan **GPIO 16** -> Layar berubah Hijau secara *real-time*.
  - Menekan **GPIO 5** -> Layar berubah Biru secara *real-time*.

### B. Penemuan IC I2C Setelah Reset Hardware GPIO 3 Aktivasi
- **MAX17048 Fuel Gauge (`0x36`)**: **TERHUBUNG & BERHASIL DIBACA LIVE!**
  - Tegangan baterai terdeteksi secara real-time pada serial log: `3.355V` (SOC: `0.1%`).
- **CST9217 Touch Panel (`0x5A`)**: **TERHUBUNG & RESPONSIP!**
  - IC Touch Panel CST9217 berhasil terdeteksi pada alamat `0x5A` di bus I2C.

---

## 5. Konfigurasi OLED Power Enable Pin (15 Agustus 2026)

- **OLED_EN (LCD Power Enable)**: Dikonfirmasi terhubung pada **GPIO 8**.
- **Implementasi Driver**: GPIO 8 diatur sebagai `OUTPUT` dan di-set `HIGH` di awal `display_init()` dan `setup()` untuk menyalakan jalur power rail/regulator display AMOLED.

---

## 6. Pengujian Touchscreen CST9217 & Sinkronisasi I2C Bus Mutex (15 Agustus 2026)

**Status:** **100% SUKSES & LIVE VERIFIED**

### A. Deteksi Hardware & Konektor BTB
- Setelah re-solder soket BTB, IC **CST9217 Capacitive Touch Controller** terdeteksi sempurna pada alamat I2C **`0x5A`**.
- Bus I2C (`SDA = GPIO 48`, `SCL = GPIO 47`) berhasil mendeteksi 2 perangkat aktif:
  1. `0x36` : MAX17048 Fuel Gauge
  2. `0x5A` : CST9217 Touch Controller

### B. Pembacaan Koordinat Sentuh (Live Coordinates)
- Driver pembacaan register native `0xD000` berhasil mengonversi paket data sentuhan menjadi koordinat $(X, Y)$ secara akurat.
- **Hasil Uji Sentuhan (Live Serial Monitor & Layar AMOLED)**:
  - Sentuhan 1: `X: 256, Y: 221` $\rightarrow$ Layar langsung merespon dengan kartu `TOUCH DETECTED!`.
  - Sentuhan 2: `X: 0, Y: 120`
  - Sentuhan 3: `X: 1, Y: 64`
  - Pelepasan jari: `TOUCH RELEASED` $\rightarrow$ Kembali ke status normal.
- **Proteksi Multi-Tasking I2C Mutex**: `i2c_bus_mutex` diimplementasikan untuk mencegah tabrakan I2C antara task sensor Touch di Core 0 dan pembacaan Fuel Gauge MAX17048 di Core 1.

================================================================================



