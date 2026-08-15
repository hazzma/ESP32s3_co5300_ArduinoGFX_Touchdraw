#include "touch_driver.h"
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#define TOUCH_SDA  48
#define TOUCH_SCL  47
#define TOUCH_INT  4

SemaphoreHandle_t i2c_bus_mutex = nullptr;
static uint8_t touch_detected_addr = 0x5A;
static bool touch_chip_found = false;

static uint16_t last_x = 0;
static uint16_t last_y = 0;
static bool touch_pressed = false;

static SemaphoreHandle_t touch_mutex = nullptr;
static TaskHandle_t touch_task_handle = nullptr;

static bool check_i2c_addr(uint8_t addr) {
    if (i2c_bus_mutex) xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);
    Wire.beginTransmission(addr);
    bool ok = (Wire.endTransmission() == 0);
    if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
    return ok;
}

static bool touch_read_internal(uint16_t *x, uint16_t *y) {
    uint8_t buf[12] = {0};
    uint8_t addr = touch_detected_addr ? touch_detected_addr : 0x5A;

    if (i2c_bus_mutex) xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);

    // 1. Write 16-bit register 0xD000 to CST9217
    Wire.beginTransmission(addr);
    Wire.write(0xD0);
    Wire.write(0x00);
    int err = Wire.endTransmission(true);

    // 2. Short 1ms delay for CST9217 packet prepare
    delayMicroseconds(1000);

    // 3. Read 10 bytes from CST9217 (CST9217_DATA_LENGTH = 10)
    int read_len = 0;
    if (err == 0) {
        read_len = Wire.requestFrom((uint8_t)addr, (uint8_t)10);
        if (read_len >= 7) {
            for (int i = 0; i < read_len && Wire.available(); i++) {
                buf[i] = Wire.read();
            }
        }
    }

    if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);

    if (err != 0 || read_len < 7) return false;

    // 4. Validate ACK byte (buf[6] == 0xAB per official CST9217 driver)
    bool ack_ok = (buf[6] == 0xAB);

    // 5. Parse CST9217 touch point data:
    // status = buf[0] & 0x0F (0x06 = touch active/down, 0x01 = contact)
    // X = (buf[1] << 4) | (buf[3] >> 4)
    // Y = (buf[2] << 4) | (buf[3] & 0x0F)
    uint8_t status = buf[0] & 0x0F;
    uint16_t touch_x = ((uint16_t)buf[1] << 4) | (buf[3] >> 4);
    uint16_t touch_y = ((uint16_t)buf[2] << 4) | (buf[3] & 0x0F);

    bool is_pressed = (ack_ok && (status == 0x06 || status == 0x01) && (touch_x > 0 || touch_y > 0));

    if (is_pressed && touch_x <= 410 && touch_y <= 502) {
        *x = touch_x;
        *y = touch_y;
        return true;
    }

    return false;
}

static void touch_task(void *pvParameters) {
    bool was_pressed = false;
    uint16_t prev_x = 0, prev_y = 0;

    while (1) {
        uint16_t tx = 0, ty = 0;
        
        // Always query touch controller to guarantee 100% detection on every touch-down!
        bool pressed = touch_read_internal(&tx, &ty);

        if (touch_mutex) {
            xSemaphoreTake(touch_mutex, portMAX_DELAY);
            if (pressed) {
                last_x = tx;
                last_y = ty;
                touch_pressed = true;
            } else {
                touch_pressed = false;
            }
            xSemaphoreGive(touch_mutex);
        }

        if (pressed) {
            if (!was_pressed || abs((int)tx - (int)prev_x) > 1 || abs((int)ty - (int)prev_y) > 1) {
                if (Serial && Serial.availableForWrite() > 32) {
                    Serial.printf("[TOUCH] X: %u, Y: %u\n", tx, ty);
                }
                prev_x = tx;
                prev_y = ty;
            }
            was_pressed = true;
        } else if (was_pressed) {
            if (Serial && Serial.availableForWrite() > 32) {
                Serial.println("[TOUCH] RELEASED");
            }
            was_pressed = false;
        }

        // Fast 8ms polling rate (125Hz) for instant multi-touch responsiveness
        vTaskDelay(pdMS_TO_TICKS(8));
    }
}

void touch_init() {
    Serial.println("\n[TOUCH] Initializing Touch Controller...");

    if (!i2c_bus_mutex) {
        i2c_bus_mutex = xSemaphoreCreateMutex();
    }

    pinMode(TOUCH_INT, INPUT_PULLUP);
    delay(100);

    // Scan common touch controller I2C addresses
    const uint8_t candidate_addrs[] = { 0x5A, 0x15, 0x1A, 0x2E, 0x38, 0x5D, 0x14 };
    touch_chip_found = false;

    for (uint8_t addr : candidate_addrs) {
        if (check_i2c_addr(addr)) {
            touch_detected_addr = addr;
            touch_chip_found = true;
            Serial.printf("[TOUCH] ==> Found Touch IC at I2C address 0x%02X!\n", addr);
            break;
        }
    }

    if (!touch_chip_found) {
        Serial.println("[TOUCH] Notice: Touch IC did not respond at standard addresses. Defaulting to 0x5A.");
        touch_detected_addr = 0x5A;
    }

    // Configure CST9217 Command Mode (write 0x01 to 0xD101 per fuzzybear62/esphome-cst9217 fix)
    if (i2c_bus_mutex) xSemaphoreTake(i2c_bus_mutex, portMAX_DELAY);
    Wire.beginTransmission(touch_detected_addr);
    Wire.write(0xD1);
    Wire.write(0x01);
    Wire.write(0x01);
    int mode_err = Wire.endTransmission(true);
    if (i2c_bus_mutex) xSemaphoreGive(i2c_bus_mutex);
    delay(10);

    if (mode_err == 0) {
        Serial.println("[TOUCH] CST9217 Active Mode Initialized (0xD101 -> 0x01).");
    }

    if (!touch_mutex) {
        touch_mutex = xSemaphoreCreateMutex();
    }

    if (!touch_task_handle) {
        xTaskCreatePinnedToCore(
            touch_task,
            "sensorTask",
            4096,
            nullptr,
            3,
            &touch_task_handle,
            0 // Core 0
        );
        Serial.println("[TOUCH] Sensor task created on Core 0.");
    }
}

bool touch_read(uint16_t *x, uint16_t *y) {
    if (!touch_mutex) return false;
    
    bool pressed;
    xSemaphoreTake(touch_mutex, portMAX_DELAY);
    *x = last_x;
    *y = last_y;
    pressed = touch_pressed;
    xSemaphoreGive(touch_mutex);
    
    return pressed;
}

