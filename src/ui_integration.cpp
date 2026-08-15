#include "ui_integration.h"
#include "display_driver.h"
#include "touch_driver.h"
#include "ui/ui.h"
#include <lvgl.h>
#include <esp_heap_caps.h>

// LVGL draw buffer: 40 lines × 410px × 2 bytes = ~32KB per buffer
#define LVGL_BUF_LINES 40

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_color_t *buf1 = nullptr;
static lv_color_t *buf2 = nullptr;

// -----------------------------------------------------------------------
// Display flush callback: called by LVGL when a region is ready to paint.
// We push it directly to the AMOLED hardware via draw_bitmap.
// -----------------------------------------------------------------------
static void my_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    int32_t w = (area->x2 - area->x1 + 1);
    int32_t h = (area->y2 - area->y1 + 1);

    static uint32_t flush_cnt = 0;
    if (flush_cnt++ % 50 == 0) {
        Serial.printf("[LVGL] Flush #%u: x=%d y=%d w=%d h=%d\n", flush_cnt, area->x1, area->y1, w, h);
    }

    // Push rendered tile directly to hardware — no intermediate canvas needed
    display_draw_bitmap(area->x1, area->y1, (uint16_t *)color_p, w, h);

    // Tell LVGL the flush is done so it can render the next tile
    lv_disp_flush_ready(drv);
}

// -----------------------------------------------------------------------
// Touch read callback: called by LVGL every ~30ms
// -----------------------------------------------------------------------
static void my_touch_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    uint16_t tx = 0, ty = 0;
    if (touch_read(&tx, &ty)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = (lv_coord_t)tx;
        data->point.y = (lv_coord_t)ty;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

// -----------------------------------------------------------------------
// Initialization
// -----------------------------------------------------------------------
void ui_integration_init() {
    // 1. Initialize LVGL core
    lv_init();

    // 2. Allocate LVGL draw buffer in PSRAM (all LVGL memory lives in PSRAM now)
    size_t buf_size = LCD_W * LVGL_BUF_LINES;
    buf1 = (lv_color_t *)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    buf2 = nullptr;  // Single buffer — prevents DMA race conditions

    if (!buf1) {
        Serial.println("[ERROR] Failed to allocate LVGL draw buffer in PSRAM");
        return;
    }
    Serial.printf("[INFO] LVGL draw buffer: %u bytes (PSRAM, Single Buffer)\n", buf_size * sizeof(lv_color_t));
    Serial.printf("[INFO] Free PSRAM: %u bytes | Free Internal: %u bytes\n",
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    lv_disp_draw_buf_init(&draw_buf, buf1, nullptr, buf_size);

    // 3. Register Display Driver — partial refresh, direct push to hardware
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res    = LCD_W;
    disp_drv.ver_res    = LCD_H;
    disp_drv.flush_cb   = my_disp_flush;
    disp_drv.draw_buf   = &draw_buf;
    disp_drv.full_refresh = 0;  // Partial refresh — LVGL sends dirty tiles only
    lv_disp_drv_register(&disp_drv);

    // 4. Register Input (Touch) Driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touch_read_cb;
    lv_indev_drv_register(&indev_drv);

    // 5. Run WatchForge generated UI initializer
    ui_init();

    // 6. Force first render immediately so the display is not blank on boot
    lv_timer_handler();

    Serial.println("[OK] LVGL and UI Integration Initialized");
}

// -----------------------------------------------------------------------
// Main loop tick — call every 5ms from loop()
// -----------------------------------------------------------------------
void ui_integration_loop() {
    lv_timer_handler();
}
