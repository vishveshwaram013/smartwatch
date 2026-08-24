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

void syncTimeWithNTP();
uint8_t decToBcd(uint8_t val);
uint8_t bcdToDec(uint8_t val);


#ifdef __cplusplus
}
#endif

#endif
