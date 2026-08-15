---
name: device-config
description: Configures the PlatformIO project settings, dependencies, and build flags for ESP32S3 AMOLED panel integration.
---

# Skill: device-config

This skill is designed for the **Device Config Agent**. It guides the setup of `platformio.ini` for the ESP32-S3 board with OPI PSRAM and native USB CDC.

## Instructions
1. Create or overwrite [platformio.ini](file:///c:/Users/hanse/Documents/PlatformIO/Projects/ESP32S3-Amoled-V1-ArduinoGFX/platformio.ini) in the root of the PlatformIO project.
2. Define the configuration for the `esp32-s3-devkitc-1` development board.
3. Configure the following build flags:
   - `-DARDUINO_USB_CDC_ON_BOOT=1` (redirects Serial to native USB CDC)
   - `-DBOARD_HAS_PSRAM` (enables PSRAM support)
   - `-mfix-esp32-psram-cache-issue` (compiler workaround if needed)
4. Add the following library dependencies:
   - `moononournation/GFX Library for Arduino` (latest stable version)
   - `lvgl/lvgl@^8.3.11` (explicitly use LVGL v8)
5. Set `board_build.arduino.memory_type = qio_opi` or appropriate memory layout for the N16R8 variant to ensure OPI PSRAM is enabled at compile time.
6. Verify configurations are set correctly by compiling (do not write any application code in this step).
