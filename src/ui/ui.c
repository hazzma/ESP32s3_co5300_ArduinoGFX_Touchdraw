#include "ui.h"

// Font mappings


// Globals
lv_obj_t *ui_screen_1784282057501 = NULL;
lv_obj_t *ui_csplit_1784713731846_hour;
lv_obj_t *ui_csplit_1784713731846_min;
lv_obj_t *ui_widget_1784714134494;
lv_obj_t *ui_sbsplit_1784791335829_clock;
lv_obj_t *ui_sbsplit_1784791335829_wifi;
lv_obj_t *ui_sbsplit_1784791335829_battery;

void ui_screen_1784282057501_screen_init(void) {
    ui_screen_1784282057501 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_screen_1784282057501, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_screen_1784282057501, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_screen_1784282057501, LV_OPA_COVER, LV_PART_MAIN);

    lv_obj_set_style_bg_img_src(ui_screen_1784282057501, &ui_koloro_1716820713807, LV_PART_MAIN);

    lv_obj_t *parent = ui_screen_1784282057501;

    lv_obj_t *ui_screen_1784282057501_tileview = lv_tileview_create(ui_screen_1784282057501);
    lv_obj_set_size(ui_screen_1784282057501_tileview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(ui_screen_1784282057501_tileview, LV_OPA_TRANSP, LV_PART_MAIN);

    // Page: Main Page (undefined, undefined)
    lv_obj_t *ui_screen_1784282057501_tile_0_1 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 0, 1, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_0_1;
    ui_csplit_1784713731846_hour = lv_label_create(parent);
    lv_label_set_text(ui_csplit_1784713731846_hour, "10");
    lv_obj_set_style_text_color(ui_csplit_1784713731846_hour, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_csplit_1784713731846_hour, &ui_font_montserrat_118, LV_PART_MAIN);
    lv_obj_set_width(ui_csplit_1784713731846_hour, 190);
    lv_obj_set_height(ui_csplit_1784713731846_hour, 151);
    lv_obj_set_pos(ui_csplit_1784713731846_hour, 104, 111);
    ui_csplit_1784713731846_min = lv_label_create(parent);
    lv_label_set_text(ui_csplit_1784713731846_min, "09");
    lv_obj_set_style_text_color(ui_csplit_1784713731846_min, lv_color_hex(0xFF00D0), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_csplit_1784713731846_min, &ui_font_montserrat_120, LV_PART_MAIN);
    lv_obj_set_width(ui_csplit_1784713731846_min, 173);
    lv_obj_set_height(ui_csplit_1784713731846_min, 153);
    lv_obj_set_pos(ui_csplit_1784713731846_min, 116, 212);
    ui_widget_1784714134494 = lv_label_create(parent);
    lv_label_set_text(ui_widget_1784714134494, "Wednesday, 15 Jul");
    lv_obj_set_style_text_color(ui_widget_1784714134494, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_width(ui_widget_1784714134494, 171);
    lv_obj_set_height(ui_widget_1784714134494, 45);
    lv_obj_set_pos(ui_widget_1784714134494, 119, 319);
    ui_sbsplit_1784791335829_clock = lv_label_create(parent);
    lv_label_set_text(ui_sbsplit_1784791335829_clock, "10:09");
    lv_obj_set_style_text_color(ui_sbsplit_1784791335829_clock, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_sbsplit_1784791335829_clock, &ui_font_montserrat_11, LV_PART_MAIN);
    lv_obj_set_width(ui_sbsplit_1784791335829_clock, 50);
    lv_obj_set_height(ui_sbsplit_1784791335829_clock, 24);
    lv_obj_set_pos(ui_sbsplit_1784791335829_clock, 80, 4);
    ui_sbsplit_1784791335829_wifi = lv_obj_create(parent);
    lv_obj_clear_flag(ui_sbsplit_1784791335829_wifi, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_sbsplit_1784791335829_wifi, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // Style: modern
    lv_obj_set_width(ui_sbsplit_1784791335829_wifi, 34);
    lv_obj_set_height(ui_sbsplit_1784791335829_wifi, 43);
    lv_obj_set_pos(ui_sbsplit_1784791335829_wifi, 242, -6);
    ui_sbsplit_1784791335829_battery = lv_obj_create(parent);
    lv_obj_clear_flag(ui_sbsplit_1784791335829_battery, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_sbsplit_1784791335829_battery, lv_color_hex(0x10B981), LV_PART_MAIN);
    // Battery level: 80%, charging: no
    lv_obj_set_width(ui_sbsplit_1784791335829_battery, 50);
    lv_obj_set_height(ui_sbsplit_1784791335829_battery, 24);
    lv_obj_set_pos(ui_sbsplit_1784791335829_battery, 269, 4);
    // Page: Page (1, 0) (1, 0)
    lv_obj_t *ui_screen_1784282057501_tile_1_1 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 1, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_1;
    // Page: Page (2, 0) (2, 0)
    lv_obj_t *ui_screen_1784282057501_tile_2_1 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 2, 1, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_2_1;
    // Page: Page (1, 1) (1, 1)
    lv_obj_t *ui_screen_1784282057501_tile_1_2 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 2, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_2;
    // Page: Page (1, 2) (1, 2)
    lv_obj_t *ui_screen_1784282057501_tile_1_3 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 3, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_3;
    // Page: Page (1, 3) (1, 3)
    lv_obj_t *ui_screen_1784282057501_tile_1_4 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 4, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_4;
    // Page: Page (1, 4) (1, 4)
    lv_obj_t *ui_screen_1784282057501_tile_1_5 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 5, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_5;
    // Page: Page (1, 5) (1, 5)
    lv_obj_t *ui_screen_1784282057501_tile_1_6 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 6, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_6;
    // Page: Page (1, 6) (1, 6)
    lv_obj_t *ui_screen_1784282057501_tile_1_7 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 7, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_7;
    // Page: Page (1, 7) (1, 7)
    lv_obj_t *ui_screen_1784282057501_tile_1_8 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 1, 8, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_1_8;
    // Page: Page (0, -1) (0, -1)
    lv_obj_t *ui_screen_1784282057501_tile_0_0 = lv_tileview_add_tile(ui_screen_1784282057501_tileview, 0, 0, LV_DIR_LEFT | LV_DIR_RIGHT | LV_DIR_TOP | LV_DIR_BOTTOM);
    parent = ui_screen_1784282057501_tile_0_0;

    // FIX: Set active tile to (0, 1) where the clock widgets actually are
    lv_obj_set_tile_id(ui_screen_1784282057501_tileview, 0, 1, LV_ANIM_OFF);
}



// Wallpaper Gallery Definitions
const lv_img_dsc_t* ui_wallpapers[1] = {
    &ui_koloro_1716820713807,
};


// Transition animation helpers
void _ui_screen_change(lv_obj_t ** target, lv_scr_load_anim_t fademode, int spd, int delay, void (*target_init)(void)) {
    if(*target == NULL) {
        target_init();
    }
    lv_scr_load_anim(*target, fademode, spd, delay, false);
}

// Event callbacks


// Main initializer
void ui_init(void) {
    // Initialize first screen
    if (ui_screen_1784282057501 == NULL) {
        ui_screen_1784282057501_screen_init();
    }
    lv_disp_load_scr(ui_screen_1784282057501);
}
