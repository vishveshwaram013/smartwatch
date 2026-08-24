#pragma once

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>
#include <time.h>

#ifndef MYTIME_H
#define MYTIME_H

#ifdef __cplusplus
extern "C"
{
#endif

#define PCF8563_ADDR  0x51  //i2c address for PCF8563
#define FT6336U_ADDR  0x38


uint8_t decToBcd(uint8_t val);
uint8_t bcdToDec(uint8_t val);

//helper functions
esp_err_t rtc_get_time(struct tm *time_ptr);
esp_err_t rtc_set_time(struct tm *time_ptr);

#ifdef __cplusplus
}
#endif

#endif
