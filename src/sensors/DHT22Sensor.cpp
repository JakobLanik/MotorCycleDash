#include "DHT22Sensor.h"
#include "../config/PinConfig.h"
#include "../config/Constants.h"

DHT22Sensor::DHT22Sensor() : _dht(DHT_PIN, DHTTYPE), _lastTemperature(0), _lastHumidity(0) {}

bool DHT22Sensor::begin() {
  _dht.begin();
  delay(1000); // Give sensor time to initialize
  
  // Test read
  float testTemp = _dht.readTemperature();
  float testHumid = _dht.readHumidity();
  
  _initialized = (!isnan(testTemp) && !isnan(testHumid));
  return _initialized;
}

float DHT22Sensor::readValue() {
  return readTemperature();
}

bool DHT22Sensor::isAvailable() {
  return _initialized;
}

float DHT22Sensor::readTemperature() {
  float temp = _dht.readTemperature();
  if (!isnan(temp)) {
    _lastTemperature = temp;
  }
  return temp;
}

float DHT22Sensor::readHumidity() {
  float humid = _dht.readHumidity();
  if (!isnan(humid)) {
    _lastHumidity = humid;
  }
  return humid;
}
