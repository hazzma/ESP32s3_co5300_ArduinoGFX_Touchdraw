# FSD: Render Custom UI Library on AMOLED CO5300AF-51
**Framework:** Arduino (arduino-esp32)
**Target MCU:** ESP32-S3-DevKitC-1 N16R8 (16MB Flash, 8MB OPI PSRAM)
**Status:** Ready for agent implementation

---

## 1. Objective

Render the developer's custom UI library onto the CO5300AF-51 AMOLED panel (410×502px) using the Arduino framework. The UI must display correctly with accurate colors, proper touch input forwarding, and stable refresh. This is a display integration task — not a UI design task. The agent must not redesign or modify the UI library logic itself.

---

## 2. Hardware

| Component | Detail |
| :--- | :--- |
| MCU | ESP32-S3-DevKitC-1, N16R8 variant (16MB Flash, 8MB OPI PSRAM) |
| Display IC | CO5300AF-51 |
| Touch IC | CST9217 |
| Resolution | 410 × 502 px |
| Display Interface | QSPI |
| Touch Interface | I2C, address `0x5A` |
| Backlight | None (AMOLED self-emissive) |

---

## 3. Pin Mapping (validated — do not modify)

| Subsystem | Function | GPIO | Note |
| :--- | :--- | :--- | :--- |
| LCD QSPI | CLK | 12 | Native FSPICLK |
| LCD QSPI | CS | 10 | Native FSPICS0 |
| LCD QSPI | SIO0 (D0) | 11 | Native FSPID |
| LCD QSPI | SIO1 (D1) | 13 | Native FSPIQ |
| LCD QSPI | SIO2 (D2) | 14 | Native FSPIWP |
| LCD QSPI | SIO3 (D3) | 9 | Native FSPIHD |
| LCD | TE | 21 | Tearing effect, optional for now |
| LCD + Touch | RST (shared) | 3 | Strapping pin — must not be held LOW externally at boot |
| LCD Power | OLED_EN | 8 | Active HIGH — enables power rail to AMOLED panel |
| Touch / I2C | SDA | 48 | Requires external pull-up if module does not provide one |
| Touch / I2C | SCK (SCL) | 47 | Same as above |
| Touch | INT | 4 | RTC_GPIO4, active low |

---

## 4. Software Stack

| Layer | Choice | Note |
| :--- | :--- | :--- |
| Framework | Arduino (arduino-esp32) | — |
| Display Library | GFX Library for Arduino by moononournation | Version: latest stable |
| Display Driver Class | `Arduino_CO5300` | Specific to this panel |
| Bus Driver Class | `Arduino_ESP32QSPI` | QSPI transport |
| Framebuffer | `Arduino_Canvas` | Full-frame canvas allocated in PSRAM |
| Touch Driver | Manual I2C via `Wire` | CST9217 has no built-in driver in the library |
| Color Format | RGB565 (16-bit) | `gfx->setColorDepth(16)` |

---

## 5. Proven Configuration (do not change without reason)

These settings are validated on the actual hardware. Agent must use these exact values.

### 5.1 Bus & Panel Init

```cpp
#include <Arduino_GFX_Library.h>

#define LCD_CS    10
#define LCD_SCLK  12
#define LCD_SDIO0 11
#define LCD_SDIO1 13
#define LCD_SDIO2 14
#define LCD_SDIO3 9
#define LCD_RST   3
#define LCD_W     410
#define LCD_H     502

Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);

Arduino_GFX *gfx_dev = new Arduino_CO5300(bus, LCD_RST,
    0 /* rotation */, false /* IPS */, LCD_W, LCD_H);

Arduino_Canvas *gfx = new Arduino_Canvas(LCD_W, LCD_H, gfx_dev);
```

### 5.2 Init Sequence in setup()

```cpp
Serial.begin(115200);
while (!Serial && millis() < 2000); // Required: Native USB CDC on ESP32-S3

if (!gfx->begin(20000000)) {        // 20MHz — validated stable on jumper wire
    Serial.println("[ERROR] Display init failed");
    while (1);
}
Serial.println("[OK] Display initialized");
```

> **Note on clock speed:** 20MHz is used because the current setup uses jumper wires.
> On a final PCB with short traces, this can be raised to 40MHz or 80MHz.

### 5.3 Color Definitions (RGB565)

```cpp
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0
#define COLOR_CYAN    0x07FE  // NOT 0x07FF — causes render bug in Arduino_GFX
```

> **Warning:** `0x07FF` (standard cyan) causes a known rendering stall in Arduino_GFX
> with CO5300. Always use `0x07FE` instead.

### 5.4 Draw + Flush Pattern

All drawing must follow this pattern:

```cpp
// 1. Draw to canvas (in RAM)
gfx->fillScreen(COLOR_BLACK);
gfx->setTextColor(COLOR_WHITE);
// ... draw UI elements ...

// 2. Push canvas to display (blocking SPI transfer)
gfx->flush();
```

> `flush()` is blocking. Do not call it from a high-priority FreeRTOS task
> that would starve other tasks. Recommended: call from a dedicated display task
> at Core 1 with lower priority than sensor tasks.

---

## 6. Touch Integration

### 6.1 Init

```cpp
#include <Wire.h>

#define TOUCH_SDA  48
#define TOUCH_SCL  47
#define TOUCH_INT  4
#define TOUCH_ADDR 0x5A

void touchInit() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(100000); // 100kHz — validated, sufficient for all I2C devices on bus
    pinMode(TOUCH_INT, INPUT);
}
```

### 6.2 Read Touch Coordinates

```cpp
bool touchRead(uint16_t *x, uint16_t *y) {
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(0x00);
    Wire.endTransmission(false);
    Wire.requestFrom(TOUCH_ADDR, (uint8_t)6);
    if (Wire.available() < 6) return false;

    uint8_t buf[6];
    for (int i = 0; i < 6; i++) buf[i] = Wire.read();

    // Parsing per CST9217 register map — verify against datasheet if coordinates are wrong
    *x = ((buf[1] & 0x0F) << 8) | buf[2];
    *y = ((buf[3] & 0x0F) << 8) | buf[4];
    return (buf[1] >> 6) != 1; // return false if touch lifted
}
```

> **Agent note:** The register map above is based on common CST-series convention.
> If touch coordinates are inverted or offset, check the CST9217 datasheet for
> the exact register layout and correct the parsing accordingly.

---

## 7. UI Library Integration Requirements

The agent's task is to integrate the developer's existing UI library into this display stack. The following rules apply:

### 7.1 Canvas as the Only Render Target

The UI library must render **only to the `Arduino_Canvas *gfx` object**, not directly to `gfx_dev`. Direct writes to `gfx_dev` bypass the canvas and produce tearing or corruption.

### 7.2 Flush Trigger

The UI library must expose or accept a flush callback/trigger point. The agent must ensure `gfx->flush()` is called **after each complete frame is drawn**, not after each individual draw call (which would cause flickering and waste bandwidth).

Recommended pattern:

```cpp
void renderFrame() {
    gfx->startWrite();       // optional: batch SPI transactions
    ui_library_draw(gfx);    // UI library draws everything to canvas
    gfx->endWrite();
    gfx->flush();            // single flush per frame
}
```

### 7.3 Touch Input Forwarding

If the UI library has an input/event system, touch coordinates from `touchRead()` must be forwarded to it on every touch event. The agent must map the forwarding to the UI library's input API.

```cpp
uint16_t tx, ty;
if (touchRead(&tx, &ty)) {
    ui_library_on_touch(tx, ty); // adapt to the UI library's actual input API
}
```

### 7.4 Coordinate System

Panel origin (0,0) is top-left. X increases right, Y increases down. Panel width = 410, height = 502. If the UI library uses a different coordinate convention, the agent must apply the appropriate transform before passing coordinates.

### 7.5 FreeRTOS Task Structure (Recommended)

```
Core 0:
└── sensorTask       — read touch, IMU, fuel gauge (priority 3)
    └── Feeds input events to UI library input queue

Core 1:
└── displayTask      — call renderFrame() at ~30fps (priority 2)
    └── Calls gfx->flush() after each frame
```

---

## 8. Arduino IDE Board Settings

| Setting | Value |
| :--- | :--- |
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| PSRAM | OPI PSRAM |
| Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
| USB Mode | Hardware CDC and JTAG |
| Upload Speed | 921600 |

> **Critical:** PSRAM must be set to **OPI PSRAM**. Without this, `Arduino_Canvas`
> allocation will fail at runtime (411KB framebuffer does not fit in internal SRAM).

---

## 9. Known Issues & Workarounds

| Issue | Root Cause | Workaround |
| :--- | :--- | :--- |
| Blank screen / freeze at init | Signal integrity at >20MHz on jumper wires | Use `gfx->begin(20000000)` |
| Cyan renders as yellow / stall | `0x07FF` internal conflict in Arduino_GFX | Use `0x07FE` instead |
| Serial Monitor empty | Native USB CDC not enabled | Add `-DARDUINO_USB_CDC_ON_BOOT=1` and `while (!Serial && millis() < 2000)` |
| Canvas alloc fails / crash at begin() | PSRAM not configured in board settings | Set PSRAM to OPI PSRAM in Arduino IDE board settings |

---

## 10. Out of Scope

The following are explicitly out of scope for this FSD:

- UI design or layout decisions (agent must not change UI library design)
- Power management / deep sleep (separate FSD)
- WiFi / BLE integration
- Font or asset management beyond what the UI library already provides
- Migration to ESP-IDF framework
