#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "../sensors/MLX90614Sensor.h"    // Ändern
#include "../sensors/DHT22Sensor.h"       // Ändern
#include "../sensors/EngineThermistor.h"  // Ändern
#include "../sensors/ADXL345Sensor.h"     // Ändern
#include "../sensors/GPSSensor.h"         // Ändern
#include "../display/DisplayManager.h"    // Ändern
#include "../data/DataLogger.h"           // Ändern

class TaskManager {
public:
  void startAllTasks(DisplayManager* display, DataLogger* logger,
                     MLX90614Sensor* tireL, MLX90614Sensor* tireR,
                     DHT22Sensor* dht, EngineThermistor* engine,
                     ADXL345Sensor* imu, GPSSensor* gps);
                     
private:
  static void taskUITick(void* params);
  static void taskDHT22(void* params);
  static void taskTireL(void* params);
  static void taskTireR(void* params);
  static void taskEngine(void* params);
  static void taskIMU(void* params);
  static void taskGPS(void* params);
  static void taskDataLogger(void* params);
};

#endif