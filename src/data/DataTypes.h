#ifndef DATATYPES_H
#define DATATYPES_H

struct LogData {
  unsigned long timestamp;
  char gpsTime[12];
  float leanAngle;
  float pitchAngle;
  float speedKmh;
  float tireTempL;
  float tireTempR;
  float engineTemp;
  float ambientTemp;
  float ambientTempDHT;
  float humidity;
  float latitude;
  float longitude;
  int satellites;
};

struct MaxValues {
  float maxLeanLeft;
  float maxLeanRight;
  float maxPitch;
  float maxSpeed;
  float maxTireTempL;
  float maxTireTempR;
  float maxEngineTemp;
  unsigned long lastMaxUpdate;
  
  MaxValues() : 
    maxLeanLeft(0), maxLeanRight(0), maxPitch(0), maxSpeed(0), 
    maxTireTempL(0), maxTireTempR(0), maxEngineTemp(0),
    lastMaxUpdate(0) {}
};

#endif