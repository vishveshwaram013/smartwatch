#include "power.h"
#include <axp20x.h>
/**
 * TODO
 * WRITE YOUR CLASS FUNCTION IMPLEMENTATIONS HERE
 */
volatile bool powerButtonPressed = false; // Set a flag to indicate the power button was pressed
AXP20X_Class axp;  // Create an instance of the AXP20X_Class to manage power functions
bool setupPower() { //This function initializes the AXP202 power management chip 
    // It configures the necessary power outputs for the smartwatch
    powerButtonPressed=false;
   
    int result = axp.begin(Wire, AXP202_SLAVE_ADDRESS);
    if (result == AXP_PASS) {} 
    else {
        Serial.println("AXP202 Connection Failed.");
        return false;
    }

    // Enable power to the display (LDO2)
    axp.setPowerOutPut(AXP202_LDO2, AXP202_ON);
    
    //Enable power to the touch controller and GPS/Additional Peripherals (LDO3)
    axp.setPowerOutPut(AXP202_LDO3, AXP202_ON);
    
    // Set the voltage for LDO2 (typically 3.3V for displays)
    axp.setLDO2Voltage(3300); 

    // Enable power to the GPS/Additional Peripherals (LDO3)
    axp.setPowerOutPut(AXP202_LDO3, AXP202_ON);
    axp.setLDO3Voltage(3300);

    Serial.println("AXP202 Connection Established!");
    return true;
}

bool isUSBConnected() {
    return axp.isVBUSPlug();//check if USB power is connected
}

bool isCharging() {
    return  axp.isChargeing();//check if the battery is currently charging
}

void enableDisplayPower() {
    axp.setPowerOutPut(AXP202_LDO2, AXP202_ON);// Enable power to the display (LDO2)
}

void disableDisplayPower() {
    axp.setPowerOutPut(AXP202_LDO2, AXP202_OFF);// Disable power to the display (LDO2)
}       

void IRAM_ATTR powerButtonISR() { //Sets up a flag to indicate that the power button was pressed.
    //IRAM_ATTR is used to place the ISR in IRAM for faster execution. 
    powerButtonPressed = true; // Set the flag when the power button is pressed 
    // Handle power button press (if needed)
    // For example, you might want to toggle display power or enter a low-power mode
}

void initPowerButtonIRQ() {
    // 1. Enable Short Press IRQ in AXP202 registers [cite: 1302, 1304]
    axp.enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true); //[cite: 1302]
    
    // 2. Clear any pending interrupts to start fresh [cite: 1320]
    axp.clearIRQ(); //[cite: 1030]

    // 3. Attach the physical ESP32 pin (GPIO 35) to your ISR
    pinMode(35, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(35), powerButtonISR, FALLING);
}

void stopCharging() {
    // Function to stop charging the battery (if supported by the AXP202)
    // This is a placeholder; actual implementation depends on the AXP202 library's capabilities
    // axp.stopCharging();
    axp.enableChargeing(false);
}

void resumeCharging(){
        axp.enableChargeing(true);
}


int getBatteryPercentage() {
    // 1. Get the raw voltage in millivolts (e.g., 3700 for 3.7V)
    float voltage = axp.getBattVoltage();

    // 2. Safety bounds
    if (voltage >= 4200) return 100;
    if (voltage <= 3510) return 0;

    // 3. Map the voltage to a percentage
    // Formula: ((Voltage - Min) / (Max - Min)) * 100
    int percentage = (int)((voltage - 3510) / (4200 - 3510) * 100);

    return percentage;
}
/*
int getBatteryPercentage() {
    return axp.getBattPercentage();
}*/
