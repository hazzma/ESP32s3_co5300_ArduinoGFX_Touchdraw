#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t i2c_bus_mutex;

void touch_init();
bool touch_read(uint16_t *x, uint16_t *y);

#endif // TOUCH_DRIVER_H
