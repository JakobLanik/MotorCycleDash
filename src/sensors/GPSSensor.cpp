#include "GPSSensor.h"
#include "config/PinConfig.h"
#include "config/Constants.h"

GPSSensor::GPSSensor() : _serial(2) {}

bool GPSSensor::begin() {
  _serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  _initialized = true;
  return true;
}

float GPSSensor::readValue() {
  return getSpeed();
}

bool GPSSensor::isAvailable() {
  return _initialized;
}

float GPSSensor::getSpeed() {
  if (_gps.speed.isValid()) {
    return _gps.speed.kmph();
  }
  return 0.0f;
}

void GPSSensor::getTime(int &hour, int &minute) {
  if (_gps.time.isValid()) {
    hour = _gps.time.hour();
    minute = _gps.time.minute();
  } else {
    hour = minute = 0;
  }
}

void GPSSensor::getLocation(float &lat, float &lng) {
  if (_gps.location.isValid()) {
    lat = _gps.location.lat();
    lng = _gps.location.lng();
  } else {
    lat = lng = 0.0f;
  }
}

int GPSSensor::getSatellites() {
  if (_gps.satellites.isValid()) {
    return _gps.satellites.value();
  }
  return 0;
}

void GPSSensor::update() {
  while (_serial.available()) {
    _gps.encode(_serial.read());
  }
}