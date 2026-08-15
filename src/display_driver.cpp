#include "display_driver.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp32-hal-psram.h>
#include <rom/cache.h>  // Cache_WriteBack_Addr for PSRAM→DMA coherency

// QSPI pin mapping (per FSD + PCB custom layout)
#define LCD_CS    10
#define LCD_SCLK  12
#define LCD_SDIO0 11
#define LCD_SDIO1 13
#define LCD_SDIO2 14
#define LCD_SDIO3 9
#define LCD_RST   3
#define LCD_EN    8

static Arduino_DataBus *bus    = nullptr;
static Arduino_GFX     *gfx   = nullptr;

// ---------------------------------------------------------------------------
// display_init: Initialize OLED power, QSPI bus and CO5300 panel at 20MHz
// ---------------------------------------------------------------------------
void display_init() {
    // 1. Enable OLED Power (GPIO 8)
    pinMode(LCD_EN, OUTPUT);
    digitalWrite(LCD_EN, HIGH);
    delay(20);

    bus = new Arduino_ESP32QSPI(LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);
    // Note: Pass GFX_NOT_DEFINED (-1) for reset pin so Arduino_CO5300 doesn't re-pulse
    // the shared GPIO 3 reset line during QSPI init, protecting the CST9217 boot state.
    gfx = new Arduino_CO5300(bus, GFX_NOT_DEFINED /* rst */, 0 /* rotation */, false /* IPS */, LCD_W, LCD_H,
                              22 /* col_offset1 */, 0 /* row_offset1 */,
                              22 /* col_offset2 */, 0 /* row_offset2 */);

    if (!gfx->begin(80000000)) { // 80MHz — Maximum speed for custom PCB
        Serial.println("[ERROR] Display init failed");
        while (1);
    }
    Serial.println("[OK] Display initialized");
}

void display_fill_screen(uint16_t color) {
    if (!gfx) return;
    // Cyan sentinel workaround
    if (color == 0x07FF) color = COLOR_CYAN_FIX;
    gfx->fillScreen(color);
}

// ---------------------------------------------------------------------------
// display_show_test_card: Render color test card with text labels
// ---------------------------------------------------------------------------
void display_show_test_card(uint16_t bg_color, uint16_t text_color, const char *title, const char *hex_code) {
    if (!gfx) return;
    if (bg_color == 0x07FF) bg_color = COLOR_CYAN_FIX;
    if (text_color == 0x07FF) text_color = COLOR_CYAN_FIX;

    gfx->fillScreen(bg_color);
    gfx->setTextColor(text_color);
    gfx->setTextSize(3);
    gfx->setCursor(40, 200);
    gfx->println(title);

    gfx->setTextSize(2);
    gfx->setCursor(40, 260);
    gfx->println(hex_code);
}

// Check if pointer is in RAM (SRAM or PSRAM)
static inline bool is_ram_ptr(const void *ptr) {
    uint32_t addr = (uint32_t)ptr;
    // ESP32-S3 RAM addresses (Internal SRAM: ~0x3FC00000+, PSRAM: ~0x3C000000)
    return (addr >= 0x3C000000 && addr <= 0x3FFFFFFF);
}

// ---------------------------------------------------------------------------
// display_draw_bitmap: Push a tile (partial area) directly to the AMOLED panel.
// Called from LVGL flush_cb for each rendered dirty region or directly for images.
// Safely checks if buffer is in RAM before doing in-place Cyan fix / Cache flush
// to avoid StoreProhibited CPU crashes on Flash (const) arrays.
// ---------------------------------------------------------------------------
void display_draw_bitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h) {
    if (!gfx || !bitmap) return;

    if (is_ram_ptr(bitmap)) {
        int32_t len = (int32_t)w * h;
        for (int32_t i = 0; i < len; i++) {
            if (bitmap[i] == 0x07FF) {
                bitmap[i] = COLOR_CYAN_FIX;
            }
        }
        uint32_t byte_len = (uint32_t)len * sizeof(uint16_t);
        Cache_WriteBack_Addr((uint32_t)bitmap, byte_len);
        gfx->draw16bitRGBBitmap(x, y, bitmap, w, h);
    } else {
        // Flash buffer (read-only const array): Copy line-by-line into an SRAM buffer
        // to sanitize 0x07FF -> 0x07FE (Cyan fix) and prevent CO5300 display IC hardware hang!
        static uint16_t line_buf[LCD_W];
        for (int16_t row = 0; row < h; row++) {
            const uint16_t *src_row = bitmap + ((int32_t)row * w);
            for (int16_t col = 0; col < w; col++) {
                uint16_t pixel = src_row[col];
                if (pixel == 0x07FF) {
                    pixel = COLOR_CYAN_FIX;
                }
                line_buf[col] = pixel;
            }
            gfx->draw16bitRGBBitmap(x, y + row, line_buf, w, 1);
        }
    }
}

void display_draw_circle(int16_t x, int16_t y, uint8_t radius, uint16_t color) {
    if (!gfx) return;
    if (color == 0x07FF) color = COLOR_CYAN_FIX;
    gfx->fillCircle(x, y, radius, color);
}

void display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t thickness) {
    if (!gfx) return;
    if (color == 0x07FF) color = COLOR_CYAN_FIX;

    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t steps = (dx > dy) ? dx : dy;

    if (steps == 0) {
        gfx->fillCircle(x0, y0, thickness, color);
        return;
    }

    // Step-by-step interpolation ensures 100% gapless smooth strokes
    float xInc = (float)(x1 - x0) / steps;
    float yInc = (float)(y1 - y0) / steps;

    float x = x0;
    float y = y0;

    // Draw circles along the path for ultra-smooth rounded brush strokes
    for (int i = 0; i <= steps; i++) {
        gfx->fillCircle((int16_t)x, (int16_t)y, thickness, color);
        x += xInc;
        y += yInc;
    }
}

void display_draw_fps(float fps) {
    if (!gfx) return;
    // Draw background overlay box at top-left
    gfx->fillRect(5, 5, 120, 26, 0x0000);
    gfx->drawRect(5, 5, 120, 26, COLOR_CYAN_FIX);
    gfx->setTextColor(0x07E0); // Bright Green
    gfx->setTextSize(2);
    gfx->setCursor(12, 10);
    gfx->printf("%.1f FPS", fps);
}
