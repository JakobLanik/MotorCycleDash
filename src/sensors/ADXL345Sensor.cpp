#include "ADXL345Sensor.h"
#include "config/PinConfig.h"
#include "config/Constants.h"
#include <math.h>

//ADXL345Sensor::ADXL345Sensor() : _hspi(HSPI), _rollOffset(0.0f), _calibrated(false) {}
ADXL345Sensor::ADXL345Sensor() : _hspi(HSPI), _rollOffset(0.0f), _pitchOffset(0.0f), _calibrated(false) {}

bool ADXL345Sensor::begin() {
    pinMode(ADXL_CS, OUTPUT);
    digitalWrite(ADXL_CS, HIGH);
    
    _hspi.begin(ADXL_SCK, ADXL_MISO, ADXL_MOSI, ADXL_CS);
    
    // Check device ID
    uint8_t id = spiRead8(ADXL_REG_DEVID);
    if (id != 0xE5) {
        _initialized = false;
        return false;
    }
    
    // Configure sensor
    spiWrite8(ADXL_REG_BW_RATE, 0x0A);      // ~100 Hz
    spiWrite8(ADXL_REG_DATA_FORMAT, 0x0B);  // FULL_RES=1, ±16g
    spiWrite8(ADXL_REG_POWER_CTL, 0x08);    // Measure mode
    
    _initialized = true;
    return true;
}

float ADXL345Sensor::readValue() {
    return calculateRollAngle();
}

bool ADXL345Sensor::isAvailable() {
    return _initialized;
}

void ADXL345Sensor::readAxes(float &x, float &y, float &z) {
    int16_t rx, ry, rz;
    readAxesRaw(rx, ry, rz);
    mapAxes(rx, ry, rz, x, y, z);
}

float ADXL345Sensor::calculateRollAngle() {
    float x, y, z;
    readAxes(x, y, z);
    
    float rollDeg = calculateRollDeg(y, z);
    float rollZeroed = rollDeg - _rollOffset;
    
    return rollZeroed;
}

float ADXL345Sensor::calculatePitchAngle() {
    float x, y, z;
    readAxes(x, y, z);
    float pitchDeg = calculatePitchDeg(x, y, z);
    return pitchDeg - _pitchOffset; // Offset abziehen
}

void ADXL345Sensor::calibrate() {
    const uint32_t CAL_MS = 2000;
    uint32_t tStart = millis();
    int calCount = 0;
    double rollSum = 0.0;
    double pitchSum = 0.0;
    
    while (millis() - tStart < CAL_MS) {
        float x, y, z;
        readAxes(x, y, z);
        rollSum += calculateRollDeg(y, z);
        pitchSum += calculatePitchDeg(x, y, z);
        calCount++;
        delay(10);
    }
    
    if (calCount > 0) {
        _rollOffset = (float)(rollSum / calCount);
        _pitchOffset = (float)(pitchSum / calCount); // Pitch ebenfalls nullen
        _calibrated = true;
    }
}

// Private methods
uint8_t ADXL345Sensor::spiRead8(uint8_t reg) {
    uint8_t cmd = 0x80 | (reg & 0x3F);
    _hspi.beginTransaction(SPISettings(ADXL_SPI_HZ, MSBFIRST, SPI_MODE3));
    digitalWrite(ADXL_CS, LOW);
    _hspi.transfer(cmd);
    uint8_t val = _hspi.transfer(0x00);
    digitalWrite(ADXL_CS, HIGH);
    _hspi.endTransaction();
    return val;
}

void ADXL345Sensor::spiWrite8(uint8_t reg, uint8_t val) {
    uint8_t cmd = (reg & 0x3F);
    _hspi.beginTransaction(SPISettings(ADXL_SPI_HZ, MSBFIRST, SPI_MODE3));
    digitalWrite(ADXL_CS, LOW);
    _hspi.transfer(cmd);
    _hspi.transfer(val);
    digitalWrite(ADXL_CS, HIGH);
    _hspi.endTransaction();
}

void ADXL345Sensor::readAxesRaw(int16_t &x, int16_t &y, int16_t &z) {
    uint8_t cmd = 0xC0 | (ADXL_REG_DATAX0 & 0x3F);
    uint8_t buf[6];
    
    _hspi.beginTransaction(SPISettings(ADXL_SPI_HZ, MSBFIRST, SPI_MODE3));
    digitalWrite(ADXL_CS, LOW);
    _hspi.transfer(cmd);
    
    for (int i = 0; i < 6; i++) {
        buf[i] = _hspi.transfer(0x00);
    }
    
    digitalWrite(ADXL_CS, HIGH);
    _hspi.endTransaction();
    
    x = (int16_t)((buf[1] << 8) | buf[0]);
    y = (int16_t)((buf[3] << 8) | buf[2]);
    z = (int16_t)((buf[5] << 8) | buf[4]);
}

void ADXL345Sensor::mapAxes(int16_t rx, int16_t ry, int16_t rz, float &ax, float &ay, float &az) {
    const float s = 0.0039f; // FULL_RES ≈3.9 mg/LSB
    ax =  ry * s;  // forward/back
    ay = -rx * s;  // left/right (inverted for motorcycle mounting)
    az =  -rz * s;  // up/down
}

float ADXL345Sensor::calculateRollDeg(float ay, float az) {
    return atan2f(ay, az) * 180.0f / M_PI;
}

float ADXL345Sensor::calculatePitchDeg(float ax, float ay, float az) {
    // Wir nutzen ax und az für eine saubere Vor-Zurück-Bewegung
    return atan2f(ax, az) * 180.0f / M_PI;
}