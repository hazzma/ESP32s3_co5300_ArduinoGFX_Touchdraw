# Workspace Rules: Worker Agent Boundaries & Constraints

This workspace uses a multi-agent architectural model to build the ESP32S3 AMOLED display & touch integration. To prevent conflicts, code overwrites, and unnecessary file reads, all agents operating in this workspace must strictly adhere to the following rules.

---

## 1. Role Identification
Before starting any task, the active agent must identify which role it is playing. The roles are:
1.  **Device Config Agent** (PlatformIO project configuration)
2.  **Display Driver Agent** (AMOLED init, Canvas allocation, display task)
3.  **Touch Driver Agent** (CST9217 I2C touch reading, touch interrupt, sensor task)
4.  **UI Integration Agent** (LVGL config, WatchForge UI files copying, and glue integration)

---

## 2. Strict File Boundaries (Security & Integrity)
To prevent agents from modifying each other's work or reading unrelated context, access to files is strictly partitioned as follows:

| Agent Role | Allowed Write/Modify Paths | Allowed Read-Only Paths | Prohibited Paths (DO NOT OPEN) |
| :--- | :--- | :--- | :--- |
| **Device Config Agent** | `platformio.ini` | None | `src/*`, `docs/*` |
| **Display Driver Agent** | `src/display_driver.h`<br>`src/display_driver.cpp` | `docs/FSD_UI_Display_AMOLED_CO5300_Arduino.md` | `src/touch_driver.*`<br>`src/ui/*`<br>`src/images/*`<br>`src/lv_conf.h` |
| **Touch Driver Agent** | `src/touch_driver.h`<br>`src/touch_driver.cpp` | `docs/FSD_UI_Display_AMOLED_CO5300_Arduino.md` | `src/display_driver.*`<br>`src/ui/*`<br>`src/images/*`<br>`src/lv_conf.h` |
| **UI Integration Agent** | `src/lv_conf.h`<br>`src/ui/*`<br>`src/images/*`<br>`src/ui_integration.h`<br>`src/ui_integration.cpp`<br>`src/main.cpp` | `src/display_driver.h`<br>`src/touch_driver.h`<br>`docs/FSD_UI_Display_AMOLED_CO5300_Arduino.md` | `src/display_driver.cpp`<br>`src/touch_driver.cpp` |

> [!WARNING]
> **Enforcement:** Reading or writing files listed under "Prohibited Paths" for your active role is a direct violation of safety rules and will result in corrupted project state. Do not bypass this rule.

---

## 3. Specific Component Requirements

### Display Driver Agent:
*   Must use the QSPI bus `Arduino_ESP32QSPI` and panel driver `Arduino_CO5300` as specified.
*   Must use `Arduino_Canvas` allocated in PSRAM as the sole render target.
*   Must implement a blocking `.flush()` mechanism safely from a Core 1 FreeRTOS task at priority 2.
*   Must avoid rendering standard Cyan `0x07FF` (causes panel hang) and replace it with `0x07FE`.
*   Must set the QSPI clock to `20000000` (20MHz) for jumper-wire stability.

### Touch Driver Agent:
*   Must configure Wire I2C with SDA=47, SCL=48, and clock=100000 (100kHz).
*   Must read coordinates from address `0x5A` (CST9217) and parse X and Y registers accurately.
*   Must handle touch lift detection.
*   Must run the sampling loop inside a Core 0 FreeRTOS task at priority 3 (using interrupt or periodic polling).

### UI Integration Agent:
*   Must copy UI source and asset files from the download directory `watchforge_ui_library/src/` into the local `src/` directory.
*   Must configure LVGL v8 buffer to point to the `Arduino_Canvas` buffer from the Display Agent.
*   Must register the LVGL touch driver to read from the Touch Agent's coordinate queue/buffer.
*   Must run `lv_timer_handler()` periodically.
*   Must coordinate the initialization sequence in `src/main.cpp` in this order: Display -> Touch -> LVGL -> UI.
