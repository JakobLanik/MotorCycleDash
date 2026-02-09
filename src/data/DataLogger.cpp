#include "DataLogger.h"
#include "config/Constants.h"
#include <string.h>

DataLogger::DataLogger(SDCardManager* sdManager) 
  : _sdManager(sdManager), _lastLogTime(0), _lastMaxUpdate(0) {
  memset(&_currentData, 0, sizeof(_currentData));
}

void DataLogger::init() {
  _currentData.timestamp = millis();
  _lastLogTime = 0;
  _lastMaxUpdate = 0;
}

void DataLogger::logCurrentData() {
  unsigned long now = millis();
  
  // Update timestamp
  _currentData.timestamp = now;
  
  // Update max values
  updateMaxValues();
  
  // Log to SD card every LOG_INTERVAL ms
  if (_sdManager && _sdManager->isPresent() && (now - _lastLogTime >= LOG_INTERVAL)) {
    writeDataToSD();
    _lastLogTime = now;
  }
}

void DataLogger::updateMaxValues() {
  unsigned long now = millis();
  
  // Update max lean (separate left/right)
  updateMaxLean(_currentData.leanAngle);
  updateMaxPitch(_currentData.pitchAngle);
  // Update max speed
  updateMaxSpeed(_currentData.speedKmh);
  
  // Update max tire temperatures
  updateMaxTireTempL(_currentData.tireTempL);
  updateMaxTireTempR(_currentData.tireTempR);
  
  // Update max engine temperature
  updateMaxEngineTemp(_currentData.engineTemp);
}

void DataLogger::setLeanAngle(float angle) {
  _currentData.leanAngle = angle;
}

void DataLogger::setPitchAngle(float angle) {
  _currentData.pitchAngle = angle;
}

void DataLogger::setSpeedKmh(float speed) {
  _currentData.speedKmh = speed;
}

void DataLogger::setTireTempL(float temp) {
  _currentData.tireTempL = temp;
}

void DataLogger::setTireTempR(float temp) {
  _currentData.tireTempR = temp;
}

void DataLogger::setEngineTemp(float temp) {
  _currentData.engineTemp = temp;
}

void DataLogger::setAmbientTemp(float temp) {
  _currentData.ambientTempDHT = temp;
}

void DataLogger::setHumidity(float humidity) {
  _currentData.humidity = humidity;
}

void DataLogger::setGPSData(float lat, float lng, int satellites) {
  _currentData.latitude = lat;
  _currentData.longitude = lng;
  _currentData.satellites = satellites;
}

void DataLogger::setGPSTime(const char* timeStr) {
    if (timeStr) {
        // Wir kopieren den String in unser Daten-Objekt
        strncpy(_currentData.gpsTime, timeStr, sizeof(_currentData.gpsTime) - 1);
    }
}

LogData DataLogger::getCurrentData() const {
  return _currentData;
}

MaxValues DataLogger::getMaxValues() const {
  return _maxValues;
}

void DataLogger::resetMaxValues() {
  memset(&_maxValues, 0, sizeof(_maxValues));
  _lastMaxUpdate = millis();
}

// Private methods
void DataLogger::writeDataToSD() {
    if (!_sdManager->isPresent()) return;
    
    char buffer[512];
    
    // Reihenfolge passend zum CSV-Header:
    // 1.timestamp, 2.gps_time (s), 3.lean, 4.pitch, 5.speed, 6.tireL, 7.tireR, 
    // 8.engine, 9.amb_temp, 10.humidity, 11.lat, 12.lon, 13.sats
    
    snprintf(buffer, sizeof(buffer),
        "%lu,%s,%.2f,%.2f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.6f,%.6f,%d,",
        _currentData.timestamp,      // %lu
        _currentData.gpsTime,        // %s (String!)
        _currentData.leanAngle,      // %.2f
        _currentData.pitchAngle,     // %.2f
        _currentData.speedKmh,       // %.1f
        _currentData.tireTempL,      // %.1f
        _currentData.tireTempR,      // %.1f
        _currentData.engineTemp,     // %.1f
        _currentData.ambientTempDHT, // %.1f
        _currentData.humidity,       // %.1f
        _currentData.latitude,       // %.6f
        _currentData.longitude,      // %.6f
        _currentData.satellites      // %d
    );
    
    // Max-Werte (7 Stück)
    char maxBuffer[128];
    snprintf(maxBuffer, sizeof(maxBuffer), "%.2f,%.2f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
    _maxValues.maxLeanLeft,    // maxLeanL
    _maxValues.maxLeanRight,   // maxLeanR
    _maxValues.maxSpeed,       // maxSpeed
    _maxValues.maxTireTempL,   // maxTireL
    _maxValues.maxTireTempR,   // maxTireR
    _maxValues.maxEngineTemp,  // maxEng
    _maxValues.maxPitch);      // maxPitch
  
  strcat(buffer, maxBuffer);
  _sdManager->writeLogLine(buffer);
}

void DataLogger::updateMaxLean(float angle) {
  unsigned long now = millis();
  
  if (angle < 0) {
    float leftLean = -angle;
    if (leftLean > _maxValues.maxLeanLeft) {
      _maxValues.maxLeanLeft = leftLean;
      _maxValues.lastMaxUpdate = now;
    }
  } else {
    if (angle > _maxValues.maxLeanRight) {
      _maxValues.maxLeanRight = angle;
      _maxValues.lastMaxUpdate = now;
    }
  }
}

void DataLogger::updateMaxPitch(float pitch) {
  unsigned long now = millis();
  float absPitch = fabsf(pitch);
  
  if (absPitch > fabsf(_maxValues.maxPitch)) {
    _maxValues.maxPitch = pitch;
    _maxValues.lastMaxUpdate = now;
  }
}

void DataLogger::updateMaxSpeed(float speed) {
  unsigned long now = millis();
  
  if (speed > _maxValues.maxSpeed) {
    _maxValues.maxSpeed = speed;
    _maxValues.lastMaxUpdate = now;
  }
}

void DataLogger::updateMaxTireTempL(float temp) {
  unsigned long now = millis();
  
  if (temp > _maxValues.maxTireTempL) {
    _maxValues.maxTireTempL = temp;
    _maxValues.lastMaxUpdate = now;
  }
}

void DataLogger::updateMaxTireTempR(float temp) {
  unsigned long now = millis();
  
  if (temp > _maxValues.maxTireTempR) {
    _maxValues.maxTireTempR = temp;
    _maxValues.lastMaxUpdate = now;
  }
}

void DataLogger::updateMaxEngineTemp(float temp) {
  unsigned long now = millis();
  
  if (temp > _maxValues.maxEngineTemp) {
    _maxValues.maxEngineTemp = temp;
    _maxValues.lastMaxUpdate = now;
  }
}