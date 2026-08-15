---
name: display-driver
description: Implements display initialization, Canvas creation in PSRAM, and display task on Core 1 for the CO5300 AMOLED panel.
---

# Skill: display-driver

This skill is designed for the **Display Driver Agent**. It guides the integration of moononournation's Arduino GFX with the CO5300 panel driver and creates a Canvas buffer in PSRAM.

## Constraints
*   **Allowed files**: `src/display_driver.h`, `src/display_driver.cpp`.
*   **Prohibited files**: Do not open/edit touch driver or LVGL files.

## Instructions
1.  **Define Pins**:
    *   CS: 10, SCLK: 12, SIO0: 11, SIO1: 13, SIO2: 14, SIO3: 9
    *   RST: 3, Width: 410, Height: 502
2.  **Display Initialization**:
    *   Instantiate `Arduino_ESP32QSPI` bus.
    *   Instantiate `Arduino_CO5300` panel driver.
    *   Instantiate `Arduino_Canvas` of size 410x502 targeting the panel driver.
    *   Call `gfx->begin(20000000)` (20MHz QSPI clock).
3.  **Cyan Color Fix**:
    *   Implement color-mapping code that intercepts cyan `0x07FF` and rewrites it to `0x07FE` to prevent Arduino GFX/CO5300 firmware lockups.
4.  **FreeRTOS Render Loop**:
    *   Implement a FreeRTOS task `displayTask` pinned to Core 1, priority 2.
    *   The task calls `gfx->flush()` to draw the canvas onto the screen at ~30 FPS (approx 33ms interval).
    *   Expose wrapper functions: `display_init()`, `display_get_canvas()`, and `display_trigger_flush()`.
