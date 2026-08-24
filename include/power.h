#pragma once

#include <axp20x.h>

extern AXP20X_Class axp;
extern volatile bool powerButtonPressed; // Flag to indicate if the power button was pressed
bool setupPower();

void enableDisplayPower();
void disableDisplayPower();

void initPowerButtonIRQ();   

void powerButtonISR(); // Interrupt Service Routine for the power button (if needed)
void stopCharging(); // Function to stop charging the battery (if supported by the AXP202)
void resumeCharging();
bool isCharging(); // Function to check if the battery is currently charging (if supported by the AXP202)
bool isUSBConnected(); // Function to check if USB power is connected (if supported by the AXP202)
extern int getBatteryPercentage();
