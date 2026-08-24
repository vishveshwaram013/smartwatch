#include "mytime.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include <string.h>

/**
 * TODO
 * WRITE YOUR FUNCTION IMPLEMENTATIONS HERE
 */
const char *timeZone = "IST-5:30"; // POSIX Timezone string for India Standard Time (UTC+5:30)   
uint8_t decToBcd(uint8_t val) 
{
    return ((val / 10) << 4) | (val % 10);
}

uint8_t bcdToDec(uint8_t val) 
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

/**
 * Internal helper to read a block of registers from the PCF8563
 */
static esp_err_t rtc_read_block(uint8_t reg_addr, uint8_t *data, size_t len) 
{
    // i2c_master_write_read_device is the standard ESP-IDF way to handle RTCs
    return i2c_master_write_read_device(I2C_NUM_0, PCF8563_ADDR, &reg_addr, 1, data, len, pdMS_TO_TICKS(1000));
}

/**
 * Internal helper to write a block of registers to the PCF8563
 */
static esp_err_t rtc_write_block(uint8_t reg_addr, uint8_t *data, size_t len) 
{
    uint8_t write_buf[len + 1];
    write_buf[0] = reg_addr; // First byte is the starting register address
    memcpy(&write_buf[1], data, len);
    
    return i2c_master_write_to_device(I2C_NUM_0, PCF8563_ADDR, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
}

esp_err_t rtc_set_time(struct tm *time_ptr) 
{
    uint8_t buf[7];

    // Map struct tm values to PCF8563 registers [cite: 1043]
    buf[0] = decToBcd(time_ptr->tm_sec);               // 0x02: Seconds
    buf[1] = decToBcd(time_ptr->tm_min);               // 0x03: Minutes
    buf[2] = decToBcd(time_ptr->tm_hour);              // 0x04: Hours
    buf[3] = decToBcd(time_ptr->tm_mday);              // 0x05: Days
    buf[4] = 0;                                        // 0x06: Weekdays (optional)
    buf[5] = decToBcd(time_ptr->tm_mon + 1);           // 0x07: Months (tm_mon is 0-11)
    buf[6] = decToBcd(time_ptr->tm_year % 100);        // 0x08: Years (last two digits)

    // Start writing from register 0x02
    return rtc_write_block(0x02, buf, 7);
}




esp_err_t rtc_get_time(struct tm *time_ptr) {
    uint8_t buf[7];
    
    esp_err_t ret = rtc_read_block(0x02, buf, 7);
    if (ret != ESP_OK) return ret;
    
    // Mask non-time bits (VL, Century, etc.) and convert to Decimal [cite: 1125, 1140]
    time_ptr->tm_sec  = bcdToDec(buf[0] & 0x7F);
    time_ptr->tm_min  = bcdToDec(buf[1] & 0x7F);
    time_ptr->tm_hour = bcdToDec(buf[2] & 0x3F);       // 24-hour mode mask
    time_ptr->tm_mday = bcdToDec(buf[3] & 0x3F);
    time_ptr->tm_mon  = bcdToDec(buf[5] & 0x1F) - 1;   // Convert 1-12 to 0-11
    time_ptr->tm_year = bcdToDec(buf[6]) + 100;        // Standard tm_year is years since 1900

    return ESP_OK;
}
