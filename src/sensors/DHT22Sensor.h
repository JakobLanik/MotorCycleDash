#ifndef DHT22SENSOR_H
#define DHT22SENSOR_H

#include "SensorBase.h"
#include <DHT.h>

class DHT22Sensor : public SensorBase {
public:
  DHT22Sensor();
  bool begin() override;
  float readValue() override;
  bool isAvailable() override;
  
  float readTemperature();
  float readHumidity();
  
private:
  DHT _dht;
  float _lastTemperature;
  float _lastHumidity;
};

#endif