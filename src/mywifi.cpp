#include "mywifi.h"
#include "mytime.h"
/**
 * TODO
 * WRITE YOUR CLASS FUNCTION IMPLEMENTATIONS HERE
 */
const char* TZ_INFO = "IST-5:30"; 


void syncTimeWithNTP() {
    // 1. Set offset (5.5 hours * 3600 seconds) and NTP server
    configTime(5.5 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    struct tm timeinfo;
    // 2. Try to get the local time from the internal ESP32 clock
    if (getLocalTime(&timeinfo)) {
        // 3. Write this valid time into your PCF8563 RTC hardware
        rtc_set_time(&timeinfo); 
        Serial.println("RTC synced with NTP");
    } else {
        Serial.println("NTP Sync Failed");
    }
}
