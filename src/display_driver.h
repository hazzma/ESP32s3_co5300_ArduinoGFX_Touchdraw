#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

// Screen dimensions
#define LCD_W 410
#define LCD_H 502

// Workaround: Cyan 0x07FF is a sentinel in Arduino_GFX — use 0x07FE instead
#define COLOR_CYAN_FIX 0x07FE

// Exported functions
void display_init();
void display_fill_screen(uint16_t color);
void display_show_test_card(uint16_t bg_color, uint16_t text_color, const char *title, const char *hex_code);
void display_draw_bitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
void display_draw_circle(int16_t x, int16_t y, uint8_t radius, uint16_t color);
void display_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t thickness);

#endif // DISPLAY_DRIVER_H
