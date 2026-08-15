#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

// Custom Font definitions
LV_FONT_DECLARE(ui_font_montserrat_118);
LV_FONT_DECLARE(ui_font_montserrat_120);
LV_FONT_DECLARE(ui_font_montserrat_17);
LV_FONT_DECLARE(ui_font_montserrat_11);


// Image Asset declarations
LV_IMG_DECLARE(ui_koloro_1716820713807);

// Wallpaper Gallery Selector Array
extern const lv_img_dsc_t* ui_wallpapers[1];
#define UI_WALLPAPER_COUNT 1
#define UI_DEFAULT_WALLPAPER (&ui_koloro_1716820713807)


// Screen definitions
extern lv_obj_t *ui_screen_1784282057501;
void ui_screen_1784282057501_screen_init(void);


void ui_init(void);

#ifdef __cplusplus
}
#endif

#endif // UI_H
