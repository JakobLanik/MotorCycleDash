#ifndef ENGINETHERMISTOR_H
#define ENGINETHERMISTOR_H

#include "SensorBase.h"

class EngineThermistor : public SensorBase {
public:
  EngineThermistor();
  bool begin() override;
  float readValue() override;
  bool isAvailable() override;
  
  float readRaw();
  float readSmoothed();
  uint16_t getColorForTemperature(float temp);
  
private:
  double adcToRntc(uint16_t adc);
  double rToTempC(double rntc);
  double readRawTemperature();
  
  double _emaValue;
  bool _emaInitialized;
};

#endif