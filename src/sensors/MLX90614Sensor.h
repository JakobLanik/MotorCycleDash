#ifndef MLX90614SENSOR_H
#define MLX90614SENSOR_H

#include "SensorBase.h"
#include <Adafruit_MLX90614.h>
#include <Wire.h>

class MLX90614Sensor : public SensorBase {
public:
  MLX90614Sensor(uint8_t address);
  
  // Diese beiden sind wichtig:
  bool begin() override;               // Erfüllt SensorBase
  bool begin(TwoWire* wireBus);        // Erlaubt Bus-Wahl

  float readValue() override;
  bool isAvailable() override;
  float readAmbientTemp();
  float readObjectTemp();
  
private:
  Adafruit_MLX90614 _mlx;
  uint8_t _address;
  bool _sensorOK;
  TwoWire* _wireBus; // Merkt sich den gewählten Bus
};

#endif