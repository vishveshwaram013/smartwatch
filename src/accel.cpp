#include <Wire.h>
#include <SPI.h>
#include <Arduino.h>
#include "accel.h"
#include "SensorBMA423.hpp"

uint32_t SENSOR_SDA = 21;
uint32_t SENSOR_SCL = 22;   
uint32_t SENSOR_IRQ = 39;
float filteredMag = 512.0;      // running filtered magnitude, init to 1g in raw counts
bool aboveThreshold = false;
unsigned long lastStepTime = 0;

SensorBMA423 accel;
uint32_t lastMillis;
uint32_t lastX, lastY, lastZ;



Accel::Accel() {}

Accel::~Accel() {}

void Accel::setupAccel()
{
    //Serial.begin(115200);
    //while (!Serial);


    pinMode(SENSOR_IRQ, INPUT);

    if (!accel.begin(Wire, BMA423_SLAVE_ADDRESS, SENSOR_SDA, SENSOR_SCL)) {
        Serial.println("Failed to find BMA423 - check your wiring!");
        return;
    }
    Serial.println("Init BAM423 Sensor success!");

    //Default 4G ,200HZ
    accel.configAccelerometer();
    accel.enableFeature(SensorBMA423::FEATURE_STEP_CNTR, 1);

    accel.enablePedometer(true);
    accel.enableAccelerometer();
}


void Accel::readAccelData(AccelData& data)
{
    accel.getAccelerometer(data.x, data.y, data.z);
    Serial.print("X:");
    Serial.print(data.x); Serial.print(" ");
    Serial.print("Y:");
    Serial.print(data.y); Serial.print(" ");
    Serial.print("Z:");
    Serial.print(data.z);
    Serial.println();
    lastX = data.x;
    lastY = data.y;
    lastZ = data.z;
    vTaskDelay(50);
}

uint32_t Accel::stepsCount()
{
    return accel.getPedometerCounter();
}
float Accel::getTemperature()
{
    float f = accel.getTemperature(SensorBMA423::TEMP_DEG);
    Serial.print("getTemperature:");
    Serial.print(f);
    Serial.println("*C");
    vTaskDelay(50);
    return f;
}
