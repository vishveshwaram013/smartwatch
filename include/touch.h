#ifndef TOUCH_H
#define TOUCH_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"


#include "driver/gpio.h"
#include "driver/i2c.h"
/*
FT6336 SDA	23
FT6336 SCL	32
FT6336 RST	14
*/
#define FT6236_ADDR         0x38
#define TOUCH_INT_PIN       38
//Touch_int pin is the 
// Note: Some LilyGo models require GPIO 13 to be HIGH to enable the touch IC
#define TOUCH_RST_PIN       14

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t points;
} touch_data_t;

// Initializes I2C, GPIO Interrupts, and Resets the Touch IC
esp_err_t touch_init2(void);

// Reads coordinates from the FT6236U
esp_err_t touch_read_coords(touch_data_t *data);

// brief Helper to check if a touch event is pending
bool touch_has_event(void);

#endif
