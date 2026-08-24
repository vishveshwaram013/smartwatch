#pragma once
#include <cmath>
#include <TFT_eSPI.h>
#include <time.h>

class Display
{
public:
    void setupDisplay();
    void clearDisplay();
    void drawCircle(int x, int y, int radius, uint16_t color);
    void drawWiFiSymbol(int x, int y, uint16_t color = TFT_WHITE);
    void drawHomeSymbol(int x, int y, uint16_t color = TFT_WHITE);
    void drawRefreshSymbol(int x, int y, uint16_t color = TFT_WHITE);
    void drawClockSymbol(int x, int y, uint16_t color = TFT_WHITE);
    void drawFitnessSymbol(int x, int y, uint16_t color = TFT_WHITE);
    void drawBatterySymbol(int x, int y, int battery, uint16_t color = TFT_WHITE);

    void drawString(String text, int x, int y, int color = TFT_WHITE);
    void drawStringg(String text, int x, int y, int size);
    void drawDateTimeLock(struct tm *timeinfo);
    void drawTestInRectable(String s, int x, int y, uint16_t color=TFT_WHITE);
    void drawStopWatchTime(int seconds, int minutes, int hours);

private:
    TFT_eSPI tft;
    /**
     * TODO
     * WRITE YOUR PRIVATE MEMBERS AND FUNCTIONS HEADERS HERE
     */
};
