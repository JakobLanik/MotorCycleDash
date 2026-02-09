#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <Arduino.h>
#include "DataTypes.h"
#include "../storage/SDCardManager.h"

class DataLogger {
public:
  DataLogger(SDCardManager* sdManager);
  
  void init();
  void logCurrentData();
  void updateMaxValues();
  
  // Setter für aktuelle Daten
  void setLeanAngle(float angle);
  void setPitchAngle(float pitch);
  void setSpeedKmh(float speed);
  void setTireTempL(float temp);
  void setTireTempR(float temp);
  void setEngineTemp(float temp);
  void setAmbientTemp(float temp);
  void setHumidity(float humidity);
  void setGPSTime(const char* timeStr); // Neu: Für die Uhrzeit vom GPS
  void setGPSData(float lat, float lng, int satellites);
  void setGPSInvalid(int sats); // Neu: Für ungültige GPS-Daten (kein Fix)
  
  // Getter
  LogData getCurrentData() const;
  MaxValues getMaxValues() const;
  
  // Max-Werte zurücksetzen
  void resetMaxValues();
  
private:
  SDCardManager* _sdManager;
  LogData _currentData;
  MaxValues _maxValues;
  unsigned long _lastLogTime;
   char _latStr[16];
    char _lonStr[16];
    int _sats;

  private:
   

  void writeDataToSD();
  void updateMaxLean(float angle);
  void updateMaxPitch(float pitch);
  void updateMaxSpeed(float speed);
  void updateMaxTireTempL(float temp);
  void updateMaxTireTempR(float temp);
  void updateMaxEngineTemp(float temp);
  
  // Zeitstempel für Max-Werte Updates
  unsigned long _lastMaxUpdate;
};

#endif