---
name: ui-integration
description: Ports the WatchForge LVGL UI library into the project, registers Display/Touch drivers into LVGL, and runs the main loop.
---

# Skill: ui-integration

This skill is designed for the **UI Integration Agent**. It guides copying files from `watchforge_ui_library/src/` to the PlatformIO project, setting up `lv_conf.h`, registering LVGL display/input callbacks, and initializing all components.

## Constraints
*   **Allowed files**: `src/lv_conf.h`, `src/ui/*`, `src/images/*`, `src/ui_integration.h`, `src/ui_integration.cpp`, `src/main.cpp`.
*   **Allowed Read-Only**: `src/display_driver.h`, `src/touch_driver.h`.
*   **Prohibited files**: Do not modify `src/display_driver.cpp` or `src/touch_driver.cpp`.

## Instructions
1.  **Copy Assets & Code**:
    *   Copy the folders `ui/` and `images/` from `c:\Users\hanse\Downloads\watchforge_ui_library\src` into `c:\Users\hanse\Documents\PlatformIO\Projects\ESP32S3-Amoled-V1-ArduinoGFX\src`.
2.  **LVGL Configuration**:
    *   Create a valid `src/lv_conf.h` derived from `lv_conf_template.h`.
    *   Configure resolution (410x502) and set color depth to 16 bits (`LV_COLOR_DEPTH 16`).
3.  **LVGL Display Driver Hook**:
    *   Retrieve the canvas buffer address from `display_get_canvas()`.
    *   Initialize `lv_disp_draw_buf_t` and `lv_disp_drv_t`.
    *   In the display flush callback `my_disp_flush()`, copy/push the LVGL draw buffer directly to the display canvas and trigger a canvas flush.
4.  **LVGL Input Driver Hook**:
    *   Initialize `lv_indev_drv_t`.
    *   In the input read callback `my_touchpad_read()`, query the Touch Driver Agent's thread-safe coordinates (`touch_get_coords()`) and update the `lv_indev_data_t` structure.
5.  **Initalization & Main Loop**:
    *   Write `src/main.cpp`.
    *   Initialize systems sequentially: Display driver -> Touch driver -> LVGL (`lv_init()`) -> Register Display/Touch drivers in LVGL -> WatchForge UI (`ui_init()`).
    *   Call `lv_timer_handler()` in the main loop or inside a FreeRTOS task with a short delay (e.g. 5ms).
6.  **Handle Missing Fonts (Stubs)**:
    *   If custom fonts like `ui_font_montserrat_118.c`, etc., are missing and not resolved, create mock definitions in `src/ui/ui_font_stubs.cpp` mapping them to standard LVGL fonts (e.g., `&lv_font_montserrat_14` or other available fonts) to ensure compilation passes.
    *   Stub declarations:
        ```cpp
        #include "lvgl.h"
        const lv_font_t ui_font_montserrat_118 = lv_font_montserrat_48; // or appropriate built-in fallback
        const lv_font_t ui_font_montserrat_120 = lv_font_montserrat_48;
        const lv_font_t ui_font_montserrat_17  = lv_font_montserrat_16;
        const lv_font_t ui_font_montserrat_11  = lv_font_montserrat_12;
        ```
