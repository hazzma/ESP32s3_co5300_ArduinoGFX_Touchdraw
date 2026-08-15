#include <Arduino.h>
#include <Wire.h>
#include "display_driver.h"
#include "touch_driver.h"

// Hardware Reset & Power Enable Pins for LCD & Peripherals on PCB
#define HARDWARE_RST_PIN 3
#define OLED_EN_PIN      8

// Buttons (Active LOW)
#define BTN_7  7
#define BTN_16 16
#define BTN_5  5

// PCB I2C Pin Mapping
#define I2C_SDA 48
#define I2C_SCL 47

#define MAX17048_ADDR 0x36
#define LSM6DSO_ADDR1 0x6A
#define LSM6DSO_ADDR2 0x6B

static bool max17048_found = false;
static bool lsm6dso_found = false;
static uint8_t lsm6dso_addr = 0;

struct TestCard {
    uint16_t bg;
    uint16_t text;
    const char *name;
    const char *code;
};

static const TestCard test_cards[] = {
    { 0xF800, 0xFFFF, "TEST 1: RED",     "RGB565: 0xF800" },
    { 0x07E0, 0x0000, "TEST 2: GREEN",   "RGB565: 0x07E0" },
    { 0x001F, 0xFFFF, "TEST 3: BLUE",    "RGB565: 0x001F" },
    { 0xFFE0, 0x0000, "TEST 4: YELLOW",  "RGB565: 0xFFE0" },
    { 0x07FE, 0x0000, "TEST 5: CYAN",    "RGB565: 0x07FE" },
    { 0x18C3, 0x07E0, "TEST 6: STATUS OK","AMOLED READY"  }
};
static const size_t num_cards = sizeof(test_cards) / sizeof(test_cards[0]);

static uint8_t readReg8(uint8_t addr, uint8_t reg) {
    if (i2c_bus_mutex) xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
        return 0;
    }
    Wire.requestFrom((uint8_t)addr, (uint8_t)1);
    uint8_t val = 0;
    if (Wire.available()) val = Wire.read();
    if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
    return val;
}

static uint16_t readReg16(uint8_t addr, uint8_t reg) {
    if (i2c_bus_mutex) xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
        if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
        return 0;
    }
    Wire.requestFrom((uint8_t)addr, (uint8_t)2);
    uint16_t val = 0;
    if (Wire.available() >= 2) {
        uint8_t msb = Wire.read();
        uint8_t lsb = Wire.read();
        val = ((uint16_t)msb << 8) | lsb;
    }
    if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
    return val;
}

static bool checkDevice(uint8_t addr) {
    if (i2c_bus_mutex) xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);
    Wire.beginTransmission(addr);
    bool ok = (Wire.endTransmission() == 0);
    if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
    return ok;
}

void scanI2CBusFull() {
    Serial.println("\n------------------------------------------------");
    Serial.printf("[I2C SCAN] Scanning all addresses on SDA=%d, SCL=%d...\n", I2C_SDA, I2C_SCL);
    uint8_t count = 0;

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  ==> FOUND I2C DEVICE AT 0x%02X", addr);
            if (addr == MAX17048_ADDR) {
                Serial.print(" (MAX17048 Fuel Gauge)");
                max17048_found = true;
            } else if (addr == LSM6DSO_ADDR1 || addr == LSM6DSO_ADDR2) {
                Serial.print(" (LSM6DSO Gyro/Accel)");
                lsm6dso_found = true;
                lsm6dso_addr = addr;
            } else if (addr == 0x5A || addr == 0x15 || addr == 0x1A || addr == 0x2E || addr == 0x38 || addr == 0x5D) {
                Serial.print(" (Touch Controller)");
            }
            Serial.println();
            count++;
        }
        delay(2);
    }
    if (count == 0) {
        Serial.println("  [!] No devices detected on I2C bus.");
    } else {
        Serial.printf("  [*] Total %d device(s) found on I2C bus.\n", count);
    }
    Serial.println("------------------------------------------------\n");
}

// Pen Colors & Brush Sizes for Touch Paint Canvas
static const uint16_t pen_colors[] = { COLOR_CYAN_FIX, 0xF800 /* Red */, 0x07E0 /* Green */, 0xFFE0 /* Yellow */, 0xFFFF /* White */, 0xF81F /* Magenta */ };
static const size_t num_pen_colors = sizeof(pen_colors) / sizeof(pen_colors[0]);

static const uint8_t brush_sizes[] = { 3, 6, 12, 18 };
static const size_t num_brush_sizes = sizeof(brush_sizes) / sizeof(brush_sizes[0]);

static size_t current_color_idx = 0;
static size_t current_size_idx = 1; // 6px default

static void reset_paint_canvas() {
    display_fill_screen(0x0000); // Black background
    display_draw_circle(190, 34, brush_sizes[current_size_idx], pen_colors[current_color_idx]);
    display_draw_fps(0.0f, 0.0f);
    if (Serial && Serial.availableForWrite() > 32) {
        Serial.println("[CANVAS] Screen cleared. Ready for finger drawing!");
    }
}

void setup() {
    // 1. Instantly Power on OLED and set GPIOs
    pinMode(4, INPUT_PULLUP);
    pinMode(OLED_EN_PIN, OUTPUT);
    digitalWrite(OLED_EN_PIN, HIGH);
    
    pinMode(HARDWARE_RST_PIN, OUTPUT);
    digitalWrite(HARDWARE_RST_PIN, LOW);
    delay(30);
    digitalWrite(HARDWARE_RST_PIN, HIGH);
    delay(150);

    // 2. Init Display
    display_init();
    reset_paint_canvas();

    // 3. Init Buttons
    pinMode(BTN_7, INPUT_PULLUP);
    pinMode(BTN_16, INPUT_PULLUP);
    pinMode(BTN_5, INPUT_PULLUP);

    // 4. Init I2C & Touch with Fast Mode 400kHz Clock
    if (!i2c_bus_mutex) {
        i2c_bus_mutex = xSemaphoreCreateMutex();
    }
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setTimeOut(30);
    Wire.setClock(400000); // 400kHz Fast I2C Speed for zero-lag touch reads!

    touch_init();

    // 5. Non-blocking Serial setup
    Serial.setTxTimeoutMs(0);
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);

    Serial.println("\n==============================================");
    Serial.println("   AMOLED TOUCH PAINT CANVAS READY");
    Serial.println("   Draw on screen with finger!");
    Serial.println("   BTN 7 : Clear Screen");
    Serial.println("   BTN 16: Change Color");
    Serial.println("   BTN 5 : Change Brush Size");
    Serial.println("==============================================\n");
}

extern "C" const uint8_t ui_koloro_1716820713807_map[];

enum AppMode { MODE_DRAW = 0, MODE_IMAGE = 1 };
static AppMode current_mode = MODE_DRAW;

void loop() {
    static int last_b7 = HIGH;
    static int last_b16 = HIGH;
    static int last_b5 = HIGH;

    static bool was_drawing = false;
    static uint16_t prev_touch_x = 0, prev_touch_y = 0;

    bool render_occurred = false;

    // 1. Mode Execution
    if (current_mode == MODE_DRAW) {
        // Read Touch Screen Input for Drawing
        uint16_t tx = 0, ty = 0;
        bool touched = touch_read(&tx, &ty);

        if (touched) {
            if (was_drawing) {
                display_draw_line(prev_touch_x, prev_touch_y, tx, ty, pen_colors[current_color_idx], brush_sizes[current_size_idx]);
            } else {
                display_draw_circle(tx, ty, brush_sizes[current_size_idx], pen_colors[current_color_idx]);
            }
            prev_touch_x = tx;
            prev_touch_y = ty;
            was_drawing = true;
            render_occurred = true;
        } else {
            was_drawing = false;
        }
    } else {
        // MODE_IMAGE: Continuously render full-screen 410x502 RGB565 bitmap image
        display_draw_bitmap(0, 0, (uint16_t*)ui_koloro_1716820713807_map, 410, 502);
        render_occurred = true;
    }

    // 2. Read Button Inputs
    int b7  = digitalRead(BTN_7);
    int b16 = digitalRead(BTN_16);
    int b5  = digitalRead(BTN_5);

    // Button 7 -> Clear Screen (in DRAW mode) / Reset
    if (b7 == LOW && last_b7 == HIGH) {
        if (Serial && Serial.availableForWrite() > 32) Serial.println(">>> BTN 7 PRESSED -> CLEARING CANVAS <<<");
        if (current_mode == MODE_DRAW) {
            reset_paint_canvas();
        } else {
            display_draw_bitmap(0, 0, (uint16_t*)ui_koloro_1716820713807_map, 410, 502);
        }
        render_occurred = true;
    }

    // Button 16 -> Change Color
    if (b16 == LOW && last_b16 == HIGH) {
        current_color_idx = (current_color_idx + 1) % num_pen_colors;
        if (Serial && Serial.availableForWrite() > 32) Serial.printf(">>> BTN 16 PRESSED -> CHANGED COLOR TO INDEX %u <<<\n", current_color_idx);
        if (current_mode == MODE_DRAW) {
            display_draw_circle(190, 34, brush_sizes[current_size_idx], pen_colors[current_color_idx]);
        }
        render_occurred = true;
    }

    // Button 5 -> Toggle Mode (Mode 0: Touch Draw Canvas | Mode 1: Full Image Benchmark)
    if (b5 == LOW && last_b5 == HIGH) {
        current_mode = (current_mode == MODE_DRAW) ? MODE_IMAGE : MODE_DRAW;
        if (Serial && Serial.availableForWrite() > 32) Serial.printf(">>> BTN 5 PRESSED -> SWITCHED MODE TO %s <<<\n", current_mode == MODE_DRAW ? "TOUCH DRAW" : "FULL IMAGE BENCHMARK");
        if (current_mode == MODE_DRAW) {
            reset_paint_canvas();
        } else {
            display_draw_bitmap(0, 0, (uint16_t*)ui_koloro_1716820713807_map, 410, 502);
        }
        render_occurred = true;
    }

    last_b7 = b7;
    last_b16 = b16;
    last_b5 = b5;

    // 3. Calculate and Render Dual Benchmark (Updated every 500ms)
    // R: True Screen Render FPS (Green Box)
    // L: CPU Loop Iteration Frequency / Processing Hz (Yellow Box)
    static uint32_t last_fps_time = millis();
    static uint32_t render_frame_count = 0;
    static uint32_t loop_frame_count = 0;

    loop_frame_count++;
    if (render_occurred) {
        render_frame_count++;
    }

    uint32_t now = millis();
    if (now - last_fps_time >= 500) {
        uint32_t duration = now - last_fps_time;
        float true_render_fps = (float)render_frame_count * 1000.0f / (float)duration;
        float loop_fps = (float)loop_frame_count * 1000.0f / (float)duration;
        
        display_draw_fps(true_render_fps, loop_fps);
        
        render_frame_count = 0;
        loop_frame_count = 0;
        last_fps_time = now;
    }

    delay(2);
}












