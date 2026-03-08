#ifndef ADXL345SENSOR_H
#define ADXL345SENSOR_H

#include "SensorBase.h"
#include <SPI.h>

class ADXL345Sensor : public SensorBase {
public:
    ADXL345Sensor();
    bool begin() override;
    float readValue() override;
    bool isAvailable() override;
    float calculateRollAngle();
    float calculatePitchAngle();
    
    // Diese Deklarationen müssen vorhanden sein:
    void calibrate(); 
    void readAxes(float &x, float &y, float &z);

private:
    SPIClass _hspi;
    float _rollOffset;
    float _pitchOffset;
    bool _calibrated;

    // Interne Hilfsfunktionen
    void spiWrite8(uint8_t reg, uint8_t val);
    uint8_t spiRead8(uint8_t reg);
    void readAxesRaw(int16_t &x, int16_t &y, int16_t &z);
    
    // Diese beiden fehlten ebenfalls:
    void mapAxes(int16_t rx, int16_t ry, int16_t rz, float &ax, float &ay, float &az);
    float calculateRollDeg(float ay, float az);
    float calculatePitchDeg(float ax, float ay, float az);
};

#endif