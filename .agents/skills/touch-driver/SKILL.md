---
name: touch-driver
description: Implements I2C driver for the CST9217 touch panel and runs the touch sampling task on Core 0.
---

# Skill: touch-driver

This skill is designed for the **Touch Driver Agent**. It guides the implementation of the CST9217 touch panel driver over I2C (Wire) using GPIO 47 (SDA), GPIO 48 (SCL), and GPIO 4 (INT).

## Constraints
*   **Allowed files**: `src/touch_driver.h`, `src/touch_driver.cpp`.
*   **Prohibited files**: Do not open/edit display driver or LVGL files.

## Instructions
1.  **I2C Settings**:
    *   SDA: 47, SCL: 48, INT: 4
    *   Address: `0x5A`
    *   Clock: 100kHz (`Wire.setClock(100000)`)
2.  **CST9217 Read Logic**:
    *   Start transmission to `0x5A`, write register address `0x00`, and request 6 bytes.
    *   Read 6 bytes into a buffer:
        *   `buf[0]`: status/touch count
        *   `buf[1]` & `buf[2]`: X coordinate `((buf[1] & 0x0F) << 8) | buf[2]`
        *   `buf[3]` & `buf[4]`: Y coordinate `((buf[3] & 0x0F) << 8) | buf[4]`
        *   `buf[5]`: additional flags
    *   Check for touch lift: returns false if `(buf[1] >> 6) == 1`.
3.  **Core 0 Task Execution**:
    *   Pin `sensorTask` to Core 0, priority 3.
    *   Wait for interrupt on GPIO 4 (falling edge) or poll every 10–20ms.
    *   Expose thread-safe functions: `touch_init()`, `touch_has_input()`, and `touch_get_coords(uint16_t *x, uint16_t *y)`.
