#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>
#include <stdlib.h>       /* for malloc/free/realloc */
#include <esp_heap_caps.h> /* for heap_caps_malloc/free/realloc */

#if 1 /* Set to 1 to enable content */

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH     16
#define LV_COLOR_16_SWAP   0

/*====================
   MEMORY SETTINGS
 *====================*/
/* Use custom allocator to route ALL LVGL allocations to PSRAM */
#define LV_MEM_CUSTOM       1
#define LV_MEM_CUSTOM_INCLUDE   <esp_heap_caps.h>
#define LV_MEM_CUSTOM_ALLOC(size)   heap_caps_malloc(size, MALLOC_CAP_SPIRAM)
#define LV_MEM_CUSTOM_FREE(p)       heap_caps_free(p)
#define LV_MEM_CUSTOM_REALLOC(p, new_size)  heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM)

/*====================
   HAL SETTINGS
 *====================*/
#define LV_TICK_CUSTOM     1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/*====================
   FONT USAGE
 *====================*/
#define LV_FONT_MONTSERRAT_8     0
#define LV_FONT_MONTSERRAT_10    1
#define LV_FONT_MONTSERRAT_12    1
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_48    1
#define LV_FONT_DEFAULT          &lv_font_montserrat_14

/*====================
   DEBUG SETTINGS
 *====================*/
#define LV_USE_LOG         1
#if LV_USE_LOG
    #define LV_LOG_LEVEL   LV_LOG_LEVEL_WARN
#endif

#define LV_USE_ASSERT_NULL      1
#define LV_USE_ASSERT_MEM       1
#define LV_USE_ASSERT_MEM_ALIGN 1
#define LV_USE_ASSERT_OBJ       1
#define LV_USE_ASSERT_STYLE     1

#define LV_USE_USER_DATA   1

/*====================
   THEME SETTINGS
 *====================*/
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1

/*====================
   WIDGET USAGE
 *====================*/
#define LV_USE_ARC          1
#define LV_USE_BAR          1
#define LV_USE_BTN          1
#define LV_USE_BTNMATRIX    1
#define LV_USE_CANVAS       1
#define LV_USE_CHECKBOX     1
#define LV_USE_DROPDOWN     1
#define LV_USE_IMG          1
#define LV_USE_LABEL        1
#define LV_USE_LINE         1
#define LV_USE_ROLLER       1
#define LV_USE_SLIDER       1
#define LV_USE_SWITCH       1
#define LV_USE_TABLE        1
#define LV_USE_TEXTAREA     1
#define LV_USE_TILEVIEW     1

#endif /* 1 */

#endif /* LV_CONF_H */
