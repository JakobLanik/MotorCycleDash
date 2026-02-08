#ifndef GPSSENSOR_H
#define GPSSENSOR_H

#include "SensorBase.h"
#include <TinyGPSPlus.h>

class GPSSensor : public SensorBase {
public:
  GPSSensor();
  bool begin() override;
  float readValue() override;
  bool isAvailable() override;
  
  float getSpeed();
  void getTime(int &hour, int &minute);
  void getLocation(float &lat, float &lng);
  int getSatellites();
  void update();
  
private:
  TinyGPSPlus _gps;
  HardwareSerial _serial;
};

#endif