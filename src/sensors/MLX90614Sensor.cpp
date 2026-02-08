#include "MLX90614Sensor.h"

MLX90614Sensor::MLX90614Sensor(uint8_t address) 
    : _address(address), _sensorOK(false), _wireBus(&Wire) {}

// Standard-begin ruft die Bus-Version mit dem Standard-Wire auf
bool MLX90614Sensor::begin() {
    return begin(&Wire);
}

// Die eigentliche Initialisierung
bool MLX90614Sensor::begin(TwoWire* wireBus) {
    _wireBus = wireBus;
    _sensorOK = _mlx.begin(_address, _wireBus);
    _initialized = _sensorOK;
    return _sensorOK;
}

float MLX90614Sensor::readValue() {
    return readObjectTemp();
}

bool MLX90614Sensor::isAvailable() {
    return _sensorOK;
}

float MLX90614Sensor::readAmbientTemp() {
    if (!_sensorOK) return NAN;
    return _mlx.readAmbientTempC();
}

float MLX90614Sensor::readObjectTemp() {
    if (!_sensorOK) return NAN;
    float temp = _mlx.readObjectTempC();
    if (!isnan(temp)) {
        _lastValue = temp;
        _lastReadTime = millis();
    }
    return temp;
}