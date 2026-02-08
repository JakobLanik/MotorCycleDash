#include "TaskManager.h"
#include <Arduino.h>
#include "../config/Constants.h"
#include "../display/DisplayManager.h"

// Global variables
static DisplayManager* globalDisplayManager = nullptr;
static DataLogger* globalDataLogger = nullptr;
static MLX90614Sensor* globalTireSensorLeft = nullptr;
static MLX90614Sensor* globalTireSensorRight = nullptr;
static DHT22Sensor* globalDhtSensor = nullptr;
static EngineThermistor* globalEngineSensor = nullptr;
static ADXL345Sensor* globalImuSensor = nullptr;
static GPSSensor* globalGpsSensor = nullptr;

// KEIN Konstruktor hier - er ist bereits in der Header-Datei implizit

void TaskManager::startAllTasks(DisplayManager* displayManager,
                                DataLogger* dataLogger,
                                MLX90614Sensor* tireSensorLeft,
                                MLX90614Sensor* tireSensorRight,
                                DHT22Sensor* dhtSensor,
                                EngineThermistor* engineSensor,
                                ADXL345Sensor* imuSensor,
                                GPSSensor* gpsSensor) {
    
    // Set global pointers
    globalDisplayManager = displayManager;
    globalDataLogger = dataLogger;
    globalTireSensorLeft = tireSensorLeft;
    globalTireSensorRight = tireSensorRight;
    globalDhtSensor = dhtSensor;
    globalEngineSensor = engineSensor;
    globalImuSensor = imuSensor;
    globalGpsSensor = gpsSensor;
    
    Serial.println("========== STARTING TASKS ==========");
    
    // Create FreeRTOS tasks
    xTaskCreatePinnedToCore(taskUITick, "UI_Task", 4096, NULL, TASK_PRIORITY_UI, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskDHT22, "DHT22_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskTireL, "TireL_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskTireR, "TireR_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskEngine, "Engine_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskIMU, "IMU_Task", 4096, NULL, TASK_PRIORITY_IMU, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskGPS, "GPS_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskDataLogger, "Logger_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    
    Serial.println("All tasks started successfully!");
    Serial.println("====================================");
}

// Helper function to get temperature color
static uint16_t getTempColor(float temp) {
    if (temp < 20.0) return ST77XX_BLUE;
    if (temp < 40.0) return ST77XX_CYAN;
    if (temp < 60.0) return ST77XX_GREEN;
    if (temp < 80.0) return ST77XX_YELLOW;
    if (temp < 100.0) return ST77XX_ORANGE;
    return ST77XX_RED;
}

// Helper function to get text color based on background
static uint16_t getTextColor(uint16_t bgColor) {
    // Simple logic: if background is dark, use white text
    uint8_t r = ((bgColor >> 11) & 0x1F) * 255 / 31;
    uint8_t g = ((bgColor >> 5) & 0x3F) * 255 / 63;
    uint8_t b = (bgColor & 0x1F) * 255 / 31;
    int brightness = (r + g + b) / 3;
    return (brightness < 128) ? ST77XX_WHITE : ST77XX_BLACK;
}

// UI Task - Updates display
void TaskManager::taskUITick(void* params) {
    Serial.println("UI Task started");
    while (1) {
        if (globalDisplayManager) {
            globalDisplayManager->updateUI();
        }
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

// DHT22 Task - Reads temperature/humidity
void TaskManager::taskDHT22(void* params) {
    Serial.println("DHT22 Task started");
    while (1) {
        if (globalDhtSensor && globalDisplayManager) {
            // Read sensor values
            float temp = globalDhtSensor->readTemperature();
            float humidity = globalDhtSensor->readHumidity();
            
            if (!isnan(temp) && !isnan(humidity)) {
                // Send to display via DisplayManager methods
                globalDisplayManager->setAmbientTemp(temp);
                globalDisplayManager->setHumidity(humidity);
                
                // Also send to queue for immediate update
                UiMsg msg;
                
                msg.type = UiMsg::UPDATE_AMBIENT;
                msg.value = temp;
                if (globalDisplayManager->sendToQueue(msg)) {
                    // Success
                }
                
                msg.type = UiMsg::UPDATE_HUMIDITY;
                msg.value = humidity;
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(DHT22_UPDATE_INTERVAL));
    }
}

// Left tire temperature
void TaskManager::taskTireL(void* params) {
    Serial.println("TireL Task started");
    while (1) {
        if (globalTireSensorLeft && globalDisplayManager) {
            // Read temperature (adjust method name based on your sensor class)
            float temp = globalTireSensorLeft->readObjectTemp();
            
            if (!isnan(temp)) {
                uint16_t bgColor = getTempColor(temp);
                uint16_t textColor = getTextColor(bgColor);
                
                // Send to display
                globalDisplayManager->setTireTempLeft(temp, bgColor, textColor);
                
                // Send to queue
                UiMsg msg;
                msg.type = UiMsg::UPDATE_TIRE_LEFT;
                msg.value = temp;
                msg.bgColor = bgColor;
                msg.fgColor = textColor;
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TIRE_UPDATE_INTERVAL));
    }
}

// Right tire temperature
void TaskManager::taskTireR(void* params) {
    Serial.println("TireR Task started");
    while (1) {
        if (globalTireSensorRight && globalDisplayManager) {
            // Read temperature
            float temp = globalTireSensorRight->readObjectTemp();
            
            if (!isnan(temp)) {
                uint16_t bgColor = getTempColor(temp);
                uint16_t textColor = getTextColor(bgColor);
                
                // Send to display
                globalDisplayManager->setTireTempRight(temp, bgColor, textColor);
                
                // Send to queue
                UiMsg msg;
                msg.type = UiMsg::UPDATE_TIRE_RIGHT;
                msg.value = temp;
                msg.bgColor = bgColor;
                msg.fgColor = textColor;
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TIRE_UPDATE_INTERVAL));
    }
}

// Engine temperature
void TaskManager::taskEngine(void* params) {
    Serial.println("Engine Task started");
    while (1) {
        if (globalEngineSensor && globalDisplayManager) {
            // Read temperature (adjust method name)
            float temp = globalEngineSensor->readSmoothed();
            
            if (!isnan(temp) && temp > -40 && temp < 200) {
                uint16_t bgColor = getTempColor(temp);
                uint16_t textColor = getTextColor(bgColor);
                
                // Send to display
                globalDisplayManager->setEngineTemp(temp, bgColor, textColor);
                
                // Send to queue
                UiMsg msg;
                msg.type = UiMsg::UPDATE_ENGINE;
                msg.value = temp;
                msg.bgColor = bgColor;
                msg.fgColor = textColor;
                globalDisplayManager->sendToQueue(msg);
                
                // Update info if engine is hot
                if (temp > 95.0) {
                    char info[32];
                    snprintf(info, sizeof(info), "Engine HOT: %.1fC", temp);
                    globalDisplayManager->setInfoText(info);
                    
                    msg.type = UiMsg::UPDATE_INFO;
                    strncpy(msg.text, info, sizeof(msg.text));
                    globalDisplayManager->sendToQueue(msg);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ENGINE_UPDATE_INTERVAL));
    }
}

// IMU sensor
void TaskManager::taskIMU(void* params) {
    Serial.println("IMU Task started");
    while (1) {
        // Wir brauchen hier zusätzlich globalDataLogger!
        if (globalImuSensor && globalDisplayManager && globalDataLogger) {
            
            float leanAngle = globalImuSensor->calculateRollAngle();
            float pitchAngle = globalImuSensor->calculatePitchAngle();

            if (!isnan(leanAngle)) {
                // 1. Display aktualisieren
                globalDisplayManager->setLeanAngle(leanAngle);
                
                // 2. WICHTIG: Logger füttern!
                globalDataLogger->setLeanAngle(leanAngle);
                
                // 3. UI Queue
                UiMsg msg;
                msg.type = UiMsg::UPDATE_LEAN;
                msg.value = leanAngle;
                globalDisplayManager->sendToQueue(msg);
            }

            if (!isnan(pitchAngle)) {
                // 4. Logger füttern! (Diese Methode musst du in DataLogger.h haben)
                globalDataLogger->setPitchAngle(pitchAngle);

                // 5. UI Queue für Pitch
                UiMsg msgP;
                msgP.type = UiMsg::UPDATE_PITCH;
                msgP.value = pitchAngle;
                globalDisplayManager->sendToQueue(msgP);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(IMU_UPDATE_INTERVAL));
    }
}

// GPS
void TaskManager::taskGPS(void* params) {
    Serial.println("GPS Task started");
    while (1) {
        if (globalGpsSensor && globalDisplayManager) {
            // Update GPS data
            globalGpsSensor->update();
            
            // Get values (adjust method names)
            float speed = globalGpsSensor->getSpeed();
            int hour, minute;
            globalGpsSensor->getTime(hour, minute);
            
            // Send speed to display
            if (!isnan(speed)) {
                globalDisplayManager->setSpeed(speed);
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_SPEED;
                msg.value = speed;
                globalDisplayManager->sendToQueue(msg);
            }
            
            // Send time to display
            if (hour >= 0 && minute >= 0) {
                char timeStr[16];
                snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
                globalDisplayManager->setTime(timeStr);
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_TIME;
                strncpy(msg.text, timeStr, sizeof(msg.text));
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(GPS_UPDATE_INTERVAL));
    }
}

// Data logger
void TaskManager::taskDataLogger(void* params) {
    Serial.println("Logger Task started");
    while (1) {
        if (globalDataLogger) {
            globalDataLogger->logCurrentData();
        }
        vTaskDelay(pdMS_TO_TICKS(LOGGER_UPDATE_INTERVAL));
    }
}