#include "GPSSensor.h"
#include "config/PinConfig.h"
#include "config/Constants.h"
#include <Arduino.h>

// Konstruktor: Initialisiert HardwareSerial auf Port 2
GPSSensor::GPSSensor() : _serial(2) {}

bool GPSSensor::begin() {
    _serial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
    _initialized = true;
    return true;
}

void GPSSensor::update() {
    while (_serial.available()) {
        _gps.encode(_serial.read());
    }
}

float GPSSensor::getSpeed() {
    return _gps.speed.isValid() ? (float)_gps.speed.kmph() : 0.0f;
}

float GPSSensor::getLatitude() {
    return _gps.location.isValid() ? (float)_gps.location.lat() : 0.0f;
}

float GPSSensor::getLongitude() {
    return _gps.location.isValid() ? (float)_gps.location.lng() : 0.0f;
}

int GPSSensor::getSatellites() {
    return _gps.satellites.isValid() ? (int)_gps.satellites.value() : 0;
}

void GPSSensor::getTime(int &hour, int &minute, int &second) {
    if (_gps.time.isValid()) {
        hour = _gps.time.hour();
        minute = _gps.time.minute();
        second = _gps.time.second();
    } else {
        hour = minute = second = -1;
    }
}

// Hilfsfunktionen für Vererbung / Kompatibilität
float GPSSensor::readValue() {
    return getSpeed();
}

bool GPSSensor::isAvailable() {
    return _initialized;
}

void GPSSensor::getLocation(float &lat, float &lng) {
    if (_gps.location.isValid()) {
        lat = (float)_gps.location.lat();
        lng = (float)_gps.location.lng();
    } else {
        lat = lng = 0.0f;
    }
}