#pragma once
#include <Arduino.h>
#include <SensorBMA423.hpp>

class Accel
{
public:
    struct AccelData {
        int16_t x;
        int16_t y;
        int16_t z;
    };
    

    Accel();
    ~Accel();

    void setupAccel();
    void readAccelData(AccelData& data);
    float getTemperature();
    uint32_t stepsCount();
private:
    /**
     * TODO
     * WRITE YOUR PRIVATE MEMBERS AND FUNCTIONS HEADERS HERE
     */
};
