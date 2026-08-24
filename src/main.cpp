#include <Arduino.h>
#include "display.h"
#include "power.h"
#include "mywifi.h"
#include <WiFi.h>
#include "mytime.h"
#include "accel.h"

#include "FS.h"
extern "C" {
  #include "touch.h"
}

uint32_t x = 60, y=105;

Display display;
MyWifi wifi;
Accel accl;
String WIFI_SSID = "itel P55 5G";
String WIFI_PASSWORD = "Vishwaa@25";

uint16_t laststate = -1;
boolean isStopwatchRunning = false;
uint64_t stopwatchStartTime = 0;
uint16_t state = 0;  //0->Home screen, 1->Wifi screen, 2->stop watch screen 3->Step counter screen
volatile bool displayOn = true; // Global variable to track display state
unsigned long lastUpdateTime = 0;
extern void syncTimeWithNTP();

// This function manually polls Pin 35 and updates the flag
void checkPowerButton() {
    // AXP202 IRQ pin is Active Low. 
    // If digitalRead returns LOW, the AXP202 is signaling an event. [cite: 521, 1032]
    if (digitalRead(35) == LOW) {
        powerButtonPressed = true; 
        vTaskDelay(200);

    }
}

void checkWiFi() {//remove comment from syncTimeWithNTP(); to fetch time from web and save to RTC
    WiFi.persistent(false); // Disable Wi-Fi persistence to save power
    WiFi.disconnect(true); // Ensure Wi-Fi is disconnected before starting
    WiFi.mode(WIFI_STA); // Set Wi-Fi to station mode
    vTaskDelay(100); // Short delay to ensure Wi-Fi hardware is ready
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Give it 10 seconds to connect
    int retry = 0;
    display.drawString("Connecting to Wi-Fi...", 55, 110);

    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        Serial.printf("Last Error: %d\n", WiFi.status());
        retry++;

    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        display.drawStringg("Connected!", 57, 130, 4);
    }
    else {
        Serial.println("Failed to connect to Wi-Fi");
        display.drawStringg("Failed to connect!", 20, 135, 4);
    }
    // Shut down Wi-Fi to save significant battery power
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void updateBatteryDisplay() {
    if(displayOn == false) {
        return; // If the display is off, skip updating
    }
    int batteryPercentage = getBatteryPercentage(); // Get the current battery percentage
    display.drawString(String(batteryPercentage) + "%", 155, 13, TFT_CYAN);
    display.drawBatterySymbol(205, 20, batteryPercentage);
}

void updateTimeDisplay(boolean imp = false) {
    struct tm now;//This is called once every minute
    if((displayOn == false || state != 0) && !imp) {
        return; // If the display is off, skip updating
    }
    unsigned long currentMillis = millis(); 
    if((not (lastUpdateTime ==0 || currentMillis - lastUpdateTime >= 45000)) && !imp) {
        return; // If it's not time to update yet, skip the rest of the loop
    }
    lastUpdateTime = currentMillis; // Update the last update time
    if (rtc_get_time(&now) == ESP_OK) {
        display.drawDateTimeLock(&now);
        Serial.println("Display Refreshed: " + String(now.tm_hour) + ":" + String(now.tm_min)); //Comment it out for faster performance
    }
    updateBatteryDisplay(); // Update battery display every time we update the time
}

//for the factorial of a number
int fact(int a)
{
    if(a<=-1)
        return -1;
    int fact = 1;
    for(int i=1; i<=a; i++)
        fact *= i;
    return fact;
}

void getHomeScreen(){
    updateTimeDisplay(true);
    display.drawWiFiSymbol(30, 35);
    display.drawClockSymbol(60, 180);
    display.drawFitnessSymbol(180, 185);
    display.drawString("Stop Watch", 30, 210);
    display.drawString("Step Counter", 140, 210);
    Serial.println("Home Screen Displayed");
}

void getStopwatchScreen(){
    display.clearDisplay();
    display.drawHomeSymbol(10, 20);
    updateBatteryDisplay();
    display.drawStringg("Stopwatch Screen", 15, 60, 4);
    display.drawStopWatchTime(0, 0, 0); // Display initial stopwatch time
    display.drawTestInRectable(" Start ", 20, 178, TFT_WHITE);
    display.drawTestInRectable(" Stop ", 95, 178, TFT_WHITE);
    display.drawTestInRectable(" Reset ", 160, 178, TFT_WHITE);
}


void updateStopwatchDisplay() {
    if (isStopwatchRunning) {
        uint64_t elapsedMillis = millis() - stopwatchStartTime;
        uint32_t totalSeconds = elapsedMillis / 1000;
        uint32_t hours = totalSeconds / 3600;
        uint32_t minutes = (totalSeconds % 3600) / 60;
        uint32_t seconds = totalSeconds % 60;

        display.drawStopWatchTime(seconds, minutes, hours);
    }
}

void getWiFiScreen(){
    display.clearDisplay();
    updateBatteryDisplay();
    display.drawHomeSymbol(10, 20);
    display.drawStringg("Wi-Fi", 90, 40, 4);
    display.drawWiFiSymbol(120, 100);
    checkWiFi();

    display.drawRefreshSymbol(60, 190);
    display.drawTestInRectable(" Sinc Time ", 143, 178, TFT_WHITE);
    vTaskDelay(500); // Short delay to ensure the display updates before proceeding
}

void getStepCounterScreen(){
    display.clearDisplay();
    updateBatteryDisplay();
    display.drawHomeSymbol(10, 20);    
    display.drawStringg("Step-Count", 55, 65, 4);
    display.drawTestInRectable(" Steps: " + String(accl.stepsCount()), 65, 105, TFT_LIGHTGREY);
    display.drawRefreshSymbol(120, 180);
}

void updateStepCount()
{
    display.drawTestInRectable(" Steps: " + String(accl.stepsCount()), 65, 105, TFT_LIGHTGREY);
}

void getCorrectState(){
    
    if(state == 0) {
        display.clearDisplay();
        getHomeScreen();
    }
    else if(state == 1) {
        display.clearDisplay();
        getWiFiScreen();
    }
    else if(state == 2) {
        display.clearDisplay();
        getStopwatchScreen();   
    }
    else if(state == 3) {
        display.clearDisplay();
        getStepCounterScreen();
    }
    laststate = state; // Update laststate to the current state after handling
}

void updatePowerButtonState() {
    if (powerButtonPressed) {
        powerButtonPressed = false; // Reset flag

        // Read the AXP202 status to identify the event [cite: 1030]
        axp.readIRQ(); //[cite: 1030]

        // Check if the event was specifically a Short Press 
        // This is necessary because the AXP202 can generate multiple types of interrupts (long press, double press, etc.) 
        // It generates interrupts for various events, so we need to check which one occurred. [cite: 1302]
        if (axp.isPEKShortPressIRQ()) { //[cite: 1302]
            displayOn = !displayOn; // Toggle state

            if (displayOn) {
                enableDisplayPower();
                updateBatteryDisplay(); // Update battery info immediately when turned on
                updateTimeDisplay(); // Refresh the display immediately when turned on  
                axp.clearIRQ(); 
                return; // Skip the rest of the loop to avoid unnecessary updates while turning on
            } else {
                disableDisplayPower(); // Turn off LDO2
            }
        }

        // Clear IRQ status in the PMIC to allow future interrupts [cite: 1320]
        axp.clearIRQ(); //[cite: 1030]
    }

}

void updateTouchSensor() {
    if(displayOn)
    {
        touch_data_t touchData;

        // Only read if ISR actually fired!
        if (touch_has_event()) {
            esp_err_t ret = touch_read_coords(&touchData);
            uint16_t xx=touchData.x;
            uint16_t yy=touchData.y;
            if(xx == 0 && yy ==00)
                return;
            vTaskDelay(200);
            Serial.printf("Touch detected at: (%d, %d)\n", xx, yy);

            if(state == 0)
            {
                if(xx <=60 && yy<=70)
                    state = 1;
                //   Serial.println("WiFi Symbol Touched");
                else if((xx >= 35 && xx <= 85) && (yy >= 135 && yy <= 205))
                        state =2; // Stopwatch
                //    Serial.println("Stopwatch Symbol Touched");
                else if(xx>=180 && yy>=130)  
                   state = 3; // Step Counter
                //    Serial.println("Step Counter Symbol Touched");
            }
            else if(state == 1)
            {
                if(xx <=60 && yy<=60)
                {
                    state = 0;
                    Serial.println("Home Symbol Touched");
                }
                else if(xx >= 35 && xx <= 95 && yy >= 190-35 && yy <= 190+35)
                {
                    getCorrectState();
                    Serial.println("Refresh Symbol Touched");
                    return;
                }
                else if(xx >= 140 && xx <= 210 && yy >= 180 && yy <= 205)
                {
                    if(WiFi.status() == WL_CONNECTED)
                    {
                        syncTimeWithNTP();
                        return;
                    }
                }
            }      
            else if(state == 2)
            {
                if(xx <=60 && yy<=60)
                {
                    state = 0;
                    Serial.println("Home Symbol Touched");
                }
                else if(xx >= 15 && xx <= 75 && yy >= 175 && yy <= 200)
                {   
                    if(!isStopwatchRunning)
                    {
                        isStopwatchRunning = true;
                        stopwatchStartTime = millis() - stopwatchStartTime;
                        Serial.println("Start Button Touched");
                    }
                }
                else if(xx >= 90 && xx <= 140 && yy >= 178 && yy <= 198)
                {
                    if(isStopwatchRunning)
                    {
                        isStopwatchRunning = false;
                        stopwatchStartTime = millis() - stopwatchStartTime;
                    }    
                    Serial.println("Stop Button Touched");
                }
                else if(xx >= 155 && xx <= 205 && yy >= 178 && yy <= 198)
                {   
                    if(isStopwatchRunning)
                    {
                        isStopwatchRunning = false;
                        stopwatchStartTime = 0;
                    }
                    else
                    {
                        stopwatchStartTime = 0;
                    }
                    display.drawStopWatchTime(0,0,0);
                    Serial.println("Reset Button Touched");
                }
            }
            else if(state == 3)
            {
                if(xx <=60 && yy<=60)
                {
                    state = 0;
                    Serial.println("Home Symbol Touched");
                }
                else if(xx >= 100 && xx <= 140 && yy >= 180 && yy <= 200)
                {
                    updateStepCount();
                    Serial.println("Refresh Symbol Touched");
                }
            }
        }
    }
}

void dummyloop() { //This is a dummy loop called in loop only while debugging.

    display.clearDisplay();
    display.drawString("Steps: " + String(accl.stepsCount()), 30, 200, TFT_YELLOW); // Display step count
    vTaskDelay(1000); // Short delay to avoid overwhelming the loop
    // The actual logic is handled in the main loop above.
        
}

void updateAll()
{
    if(state == 0)
    {
        updateTimeDisplay();
    }
    else if(state == 1)
    {    }
    else if(state == 2)
    {
        updateStopwatchDisplay();
    }
    else if(state == 4)
    {
        updateStepCount();
    }
}

void setup() {
    //Serial.begin(115200);
    Wire.begin(21, 22);
    setupPower();
    enableDisplayPower();

    
    display.setupDisplay();
    display.clearDisplay();
    Serial.println("Display Initialized");
    //Serial is however disabled to save power, I shall enable when needed for debugging

    
    touch_init2();
    Serial.println("Touch Initialized");

    accl.setupAccel();
    
}

void getmovement() {
    touch_data_t touchData;

        // Only read if ISR actually fired!
        if (touch_has_event()) {
            esp_err_t ret = touch_read_coords(&touchData);
            uint16_t xx=touchData.x;
            uint16_t yy=touchData.y;

            Serial.printf("Touch detected at: (%d, %d)\n", xx, yy);
            if(xx<80 && yy<80) 
                x=x+5;
            else if(xx>120 && yy>120)
                y=y+5;  
            else if(xx<80 && yy>120)
                x=x-5;
            else if(xx>120 && yy<80)
                y=y-5;
        }

    
}

void loop() { //remove commented 
    // Now you can display this on your TFT

    //updateBatteryDisplay(); 

    checkPowerButton();
    updatePowerButtonState();

    if(!displayOn)
        return;
    updateTimeDisplay();
    //getmovement(); // Read accelerometer data and update step count
    updateTouchSensor();
    //getCorrectState(); // Ensure the correct screen is displayed based on the current state
    if(laststate != state) {
        laststate = state;
        display.clearDisplay();
        getCorrectState();
    }
    updateAll();
    //Serial.printf("   %d, %d\n", x, y);
}
