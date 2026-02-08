#include "EngineThermistor.h"
#include "../config/PinConfig.h"
#include "../config/Constants.h"
#include <math.h>
#include <Adafruit_ST7789.h>

EngineThermistor::EngineThermistor() : _emaValue(25.0), _emaInitialized(false) {}

bool EngineThermistor::begin() {
  analogReadResolution(ADC_BITS);
  analogSetPinAttenuation(ENGINE_ADC, ADC_11db);
  pinMode(ENGINE_ADC, INPUT);
  
  _initialized = true;
  return true;
}

float EngineThermistor::readValue() {
  return readSmoothed();
}

bool EngineThermistor::isAvailable() {
  return _initialized;
}

float EngineThermistor::readRaw() {
  return readRawTemperature();
}

float EngineThermistor::readSmoothed() {
  double rawC = readRawTemperature();
  
  if (!_emaInitialized) {
    _emaValue = rawC;
    _emaInitialized = true;
  } else {
    _emaValue = EMA_ALPHA * rawC + (1.0 - EMA_ALPHA) * _emaValue;
  }
  
  return _emaValue;
}

uint16_t EngineThermistor::getColorForTemperature(float temp) {
  if (temp < 40.0) return ST77XX_BLUE;
  else if (temp < ENGINE_WARN_C) return ST77XX_GREEN;
  else if (temp < ENGINE_ALARM_C) return ST77XX_ORANGE;
  else return ST77XX_RED;
}

// Private methods
double EngineThermistor::adcToRntc(uint16_t adc) {
  if (adc <= 0) adc = 1;
  if (adc >= ADC_MAX) adc = ADC_MAX - 1;
  
  double v = (double)adc * ADC_VREF / (double)ADC_MAX;
  double r = (R_FIXED * v) / (ADC_VREF - v);
  return r;
}

double EngineThermistor::rToTempC(double rntc) {
  double tK = 1.0 / ((1.0 / BETA_THERM) * log(rntc / R0_THERM) + (1.0 / T0_K));
  return tK - 273.15;
}

double EngineThermistor::readRawTemperature() {
  const int N = 12;
  uint32_t acc = 0;
  
  for (int i = 0; i < N; ++i) {
    acc += analogRead(ENGINE_ADC);
    delayMicroseconds(500);
  }
  
  uint16_t adc = (uint16_t)(acc / N);
  double r = adcToRntc(adc);
  r = constrain(r, 100.0, 1.0e7);
  
  return rToTempC(r);
}
