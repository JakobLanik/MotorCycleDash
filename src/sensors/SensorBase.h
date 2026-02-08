#ifndef SENSORBASE_H
#define SENSORBASE_H

#include <Arduino.h>

class SensorBase {
public:
  virtual bool begin() = 0;
  virtual float readValue() = 0;
  virtual bool isAvailable() = 0;
  
protected:
  bool _initialized = false;
  float _lastValue = 0.0f;
  unsigned long _lastReadTime = 0;
};

#endif