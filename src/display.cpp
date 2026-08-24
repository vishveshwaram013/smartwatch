#include "display.h"
#include "power.h"
/**
 * TODO
 * WRITE YOUR CLASS FUNCTION IMPLEMENTATIONS HERE
 */
TFT_eSPI tft;

void Display::setupDisplay()
{
    setupPower();
    enableDisplayPower();

    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
    /*
    this->drawString("Hello, World!", 180, 100, TFT_RED);
    this->drawWiFiSymbol(30, 30);
    this->drawHomeSymbol(30, 70);
    this->drawString("Battery: " + String(getBatteryPercentage()) + "%" + String(isCharging()), 30, 150, TFT_CYAN);
    this->drawBatterySymbol(30, 120, getBatteryPercentage());
    */
    }

void Display::clearDisplay()
{
    tft.fillScreen(TFT_BLACK);
}

void Display::drawString(String text, int x, int y, int color)
{
    tft.setTextColor(color, TFT_BLACK); // Set text color and background
    tft.drawString(text, x, y, 2);      // Use the internal library drawString with the specified font size
    //x an y are the coordinates of the top-left corner of the text
}

void Display::drawStringg(String text, int x, int y, int size)
{
    tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color and background
    tft.drawString(text, x, y, size);      // Use the internal library drawString with the specified font size
    //x an y are the coordinates of the top-left corner of the text
}

void Display::drawDateTimeLock(struct tm *timeinfo)
{
    char timeBuffer[6]; // HH:MM
    char dateBuffer[9]; // YYYY-MM-DD
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    //snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", timeinfo->tm_year + 1900 , timeinfo->tm_mon + 1, timeinfo->tm_mday);
    snprintf(dateBuffer, sizeof(dateBuffer), "%02d-%02d-%02d", timeinfo->tm_mday, timeinfo->tm_mon + 1,  timeinfo->tm_year - 100);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(timeBuffer), 60, 45, 6);
    tft.drawString(String(dateBuffer), 70, 85, 4);
}

void Display::drawCircle(int x, int y, int radius, uint16_t color)
{
    tft.drawCircle(x, y, radius, color);

    vTaskDelay(1000); // Optional: Add a small delay to visualize the drawing process
    //Now erase the crcle.
    //Remove the cicle, make it dissappear suddenly.
    //Draw a black circle over the previous circle to erase it.
    tft.drawCircle(x, y, radius, TFT_BLACK);
   // Serial.printf("Circle at X: %d, Y: %d, Radius: %d erased\n", x, y, radius);
}

//Predefined: existed before i could change any shit.
//inbuilt methods for drawing symbols on the display, you can modify these or add new ones as needed
void Display::drawWiFiSymbol(int x, int y, uint16_t color)
{
    int r1 = 8;
    int r2 = 16;
    int r3 = 24;
    tft.fillCircle(x, y, 2, color);
    tft.drawArc(x, y, r1, r1 - 2, 135, 225, color, TFT_BLACK);
    tft.drawArc(x, y, r2, r2 - 2, 135, 225, color, TFT_BLACK);
    tft.drawArc(x, y, r3, r3 - 2, 135, 225, color, TFT_BLACK);
}

void Display::drawHomeSymbol(int x, int y, uint16_t color)
{
    int width = 40;
    int height = 35;
    int roofHeight = 15;
    int x0 = x;
    int x1 = x + width / 2;
    int x2 = x + width;
    int y0 = y;
    int y1 = y - roofHeight;
    tft.fillTriangle(x0, y0, x1, y1, x2, y0, color);
    int bodyHeight = height - roofHeight;
    int bodyY = y;
    tft.fillRect(x0 + 8, bodyY, width - 16, bodyHeight, color);
    int doorW = 8;
    int doorH = 10;
    int doorX = x + (width / 2) - (doorW / 2);
    int doorY = y + bodyHeight - doorH;
    tft.fillRect(doorX, doorY, doorW, doorH, TFT_BLACK);
    tft.drawTriangle(x0, y0, x1, y1, x2, y0, TFT_WHITE);
    tft.drawRect(x0 + 8, bodyY, width - 16, bodyHeight, TFT_WHITE);
}

void Display::drawRefreshSymbol(int x, int y, uint16_t color)
{
    int radius = 18;
    int thickness = 3;
    int start_angle = 45;
    int end_angle = 315;
    tft.drawArc(x, y, radius, radius - thickness, start_angle, end_angle, color, TFT_BLACK);
    float arrow_angle = (end_angle + 8) * 0.0174533;
    int arrow_len = 8;
    int offset_y = 4;
    int tip_x = x + radius * cos(arrow_angle);
    int tip_y = y - radius * sin(arrow_angle) + offset_y;
    int base1_x = tip_x - arrow_len * cos(arrow_angle - 0.5);
    int base1_y = tip_y + arrow_len * sin(arrow_angle - 0.5);
    int base2_x = tip_x - arrow_len * cos(arrow_angle + 0.5);
    int base2_y = tip_y + arrow_len * sin(arrow_angle + 0.5);
    tft.fillTriangle(tip_x, tip_y, base1_x, base1_y, base2_x, base2_y, color);
}

void Display::drawClockSymbol(int x, int y, uint16_t color)
{
    int r = 16;
    tft.drawCircle(x, y, r, color);
    tft.fillCircle(x, y, 2, color);
    int hour_len = r - 6;
    int hour_x = x - hour_len * cos(30 * 0.0174533);
    int hour_y = y - hour_len * sin(30 * 0.0174533);
    tft.drawLine(x, y, hour_x, hour_y, color);
    int min_len = r - 3;
    int min_x = x + min_len * cos(60 * 0.0174533);
    int min_y = y - min_len * sin(60 * 0.0174533);
    tft.drawLine(x, y, min_x, min_y, color);
}

void Display::drawStopWatchTime(int seconds, int minutes, int hours)
{
    //Serial.printf("Stopwatch Time: %02d:%02d:%02d\n", hours, minutes, seconds);
    char timeBuffer[9]; // HH:MM:SS
    snprintf(timeBuffer, sizeof(timeBuffer), "%02d:%02d:%02d ", hours, minutes, seconds);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(timeBuffer), 20, 95, 6);

}

void Display::drawFitnessSymbol(int x, int y, uint16_t color)
{
    int box_w = 40;
    int box_h = 24;
    int corner_r = 4;
    tft.drawRoundRect(x - box_w / 2, y - box_h / 2, box_w, box_h, corner_r, color);
    int w = box_w - 8;
    int h = box_h / 2;
    int cx = x;
    int cy = y;
    tft.drawLine(cx - w / 2, cy, cx - w / 4, cy, color);
    tft.drawLine(cx - w / 4, cy, cx - w / 8, cy - h / 2, color);
    tft.drawLine(cx - w / 8, cy - h / 2, cx, cy + h / 2, color);
    tft.drawLine(cx, cy + h / 2, cx + w / 8, cy, color);
    tft.drawLine(cx + w / 8, cy , cx + w / 2, cy, color);
    //
}

void Display::drawTestInRectable(String s, int x, int y, uint16_t color)
{//x and y are the top right coordinates of the rectangle
   // the width of the box should be as per the length of the string
   // the height  of the box is fixed for font size 2, which is 16 pixels. So the height of the box is 16 pixels.
    tft.setTextColor(color, TFT_BLACK); // Set text color and background
    int size = 2; int width = 7; int height = 20; // Font size
    if(color == TFT_LIGHTGREY)
    {  size = 4;
        width = 12;
        height = 28;
    }
    tft.drawString(s, x + 2, y + 2, size); // Draw the string with a small offset for padding
    tft.drawRoundRect(x, y, s.length() * width, height, 4, color);
}

void Display::drawBatterySymbol(int x, int y, int battery, uint16_t color)
{
    int box_w = 40;
    int box_h = 19;
    int corner_r = 4;
    int terminal_w = 4;
    tft.drawRoundRect(x - box_w / 2, y - box_h / 2, box_w, box_h, corner_r, color);
    int term_x = x + box_w / 2;
    int term_y = y - box_h / 4;
    tft.drawRect(term_x, term_y, terminal_w, box_h / 2, color);
    int fill_margin = 4;
    if (battery > 100)
    {
        battery = 100;
    }
    int fill_width = (box_w - 2 * fill_margin) * ((float)battery / 100);
    int fill_height = box_h - 2 * fill_margin;
    int fill_x = x - box_w / 2 + fill_margin;
    int fill_y = y - fill_height / 2;
    tft.fillRect(fill_x, fill_y, fill_width, fill_height, TFT_GREEN);
    String percent = String(battery) + "%";
}
