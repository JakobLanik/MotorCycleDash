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

// Helper functions
static uint16_t getTempColor(float temp) {
    if (temp < 20.0) return ST77XX_BLUE;
    if (temp < 40.0) return ST77XX_CYAN;
    if (temp < 60.0) return ST77XX_GREEN;
    if (temp < 80.0) return ST77XX_YELLOW;
    if (temp < 100.0) return ST77XX_ORANGE;
    return ST77XX_RED;
}

static uint16_t getTextColor(uint16_t bgColor) {
    uint8_t r = ((bgColor >> 11) & 0x1F) * 255 / 31;
    uint8_t g = ((bgColor >> 5) & 0x3F) * 255 / 63;
    uint8_t b = (bgColor & 0x1F) * 255 / 31;
    int brightness = (r + g + b) / 3;
    return (brightness < 128) ? ST77XX_WHITE : ST77XX_BLACK;
}

// UI Task
void TaskManager::taskUITick(void* params) {
    Serial.println("UI Task started");
    while (1) {
        if (globalDisplayManager) globalDisplayManager->updateUI();
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

// DHT22 Task - FIX: Loggt jetzt auch
void TaskManager::taskDHT22(void* params) {
    Serial.println("DHT22 Task started");
    while (1) {
        if (globalDhtSensor && globalDisplayManager) {
            float temp = globalDhtSensor->readTemperature();
            float humidity = globalDhtSensor->readHumidity();
            
            if (!isnan(temp) && !isnan(humidity)) {
                globalDisplayManager->setAmbientTemp(temp);
                globalDisplayManager->setHumidity(humidity);
                
                // An Logger senden
                if (globalDataLogger) {
                    globalDataLogger->setAmbientTemp(temp);
                    globalDataLogger->setHumidity(humidity);
                }
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_AMBIENT;
                msg.value = temp;
                globalDisplayManager->sendToQueue(msg);
                
                msg.type = UiMsg::UPDATE_HUMIDITY;
                msg.value = humidity;
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(DHT22_UPDATE_INTERVAL));
    }
}

// Left Tire - FIX: Loggt jetzt auch
void TaskManager::taskTireL(void* params) {
    Serial.println("TireL Task started");
    while (1) {
        if (globalTireSensorLeft && globalDisplayManager) {
            float temp = globalTireSensorLeft->readObjectTemp();
            if (!isnan(temp)) {
                uint16_t bgColor = getTempColor(temp);
                uint16_t textColor = getTextColor(bgColor);
                
                globalDisplayManager->setTireTempLeft(temp, bgColor, textColor);
                if (globalDataLogger) globalDataLogger->setTireTempL(temp);
                
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

// Right Tire - FIX: Loggt jetzt auch
void TaskManager::taskTireR(void* params) {
    Serial.println("TireR Task started");
    while (1) {
        if (globalTireSensorRight && globalDisplayManager) {
            float temp = globalTireSensorRight->readObjectTemp();
            if (!isnan(temp)) {
                uint16_t bgColor = getTempColor(temp);
                uint16_t textColor = getTextColor(bgColor);
                
                globalDisplayManager->setTireTempRight(temp, bgColor, textColor);
                if (globalDataLogger) globalDataLogger->setTireTempR(temp);
                
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

// Engine Task - FIX: Loggt jetzt auch
void TaskManager::taskEngine(void* params) {
    Serial.println("Engine Task started");
    while (1) {
        if (globalEngineSensor && globalDisplayManager) {
            float temp = globalEngineSensor->readSmoothed();
            if (!isnan(temp) && temp > -40 && temp < 200) {
                uint16_t bgColor = getTempColor(temp);
                uint16_t textColor = getTextColor(bgColor);
                
                globalDisplayManager->setEngineTemp(temp, bgColor, textColor);
                if (globalDataLogger) globalDataLogger->setEngineTemp(temp);
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_ENGINE;
                msg.value = temp;
                msg.bgColor = bgColor;
                msg.fgColor = textColor;
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ENGINE_UPDATE_INTERVAL));
    }
}
//imu task
void TaskManager::taskIMU(void* params) {
    Serial.println("IMU Task started");
    while (1) {
        if (globalImuSensor && globalDisplayManager) {
            float leanAngle = globalImuSensor->calculateRollAngle();
            float pitchAngle = globalImuSensor->calculatePitchAngle();
            
            // 1. Daten an Logger (SD-Karte) - Immer beides setzen
            if (globalDataLogger) {
                globalDataLogger->setLeanAngle(leanAngle);
                globalDataLogger->setPitchAngle(pitchAngle);
            }

            // 2. Nachricht für LEAN an UI senden
            if (!isnan(leanAngle)) {
                UiMsg msgLean;
                msgLean.type = UiMsg::UPDATE_LEAN;
                msgLean.value = leanAngle;
                globalDisplayManager->sendToQueue(msgLean);
            }

            // 3. Nachricht für PITCH an UI senden
            if (!isnan(pitchAngle)) {
                UiMsg msgPitch;
                msgPitch.type = UiMsg::UPDATE_PITCH; // <--- Das fehlte!
                msgPitch.value = pitchAngle;
                globalDisplayManager->sendToQueue(msgPitch);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(IMU_UPDATE_INTERVAL));
    }
}

// GPS Task - FIX: Zeit inkl. Sekunden und volles Logging
void TaskManager::taskGPS(void* params) {
    Serial.println("GPS Task started");
    while (1) {
        if (globalGpsSensor && globalDisplayManager && globalDataLogger) {
            globalGpsSensor->update();
            
            float speed = globalGpsSensor->getSpeed();
            float lat = globalGpsSensor->getLatitude();
            float lon = globalGpsSensor->getLongitude();
            int sats = globalGpsSensor->getSatellites();
            int hour, minute, second;
            globalGpsSensor->getTime(hour, minute, second); 

            // 1. Logger füttern (Funktioniert bereits)
            if (hour >= 0) {
                char logTimeStr[12];
                snprintf(logTimeStr, sizeof(logTimeStr), "%02d:%02d:%02d", hour, minute, second);
                globalDataLogger->setGPSTime(logTimeStr);
            }
            globalDataLogger->setGPSData(lat, lon, sats);
            if (!isnan(speed)) globalDataLogger->setSpeedKmh(speed);

            // 2. Display füttern
            
            // NEU: Satelliten an die Queue senden!
            UiMsg msgSat;
            msgSat.type = UiMsg::UPDATE_SATS; // Stellen Sie sicher, dass dies im Enum ist
            msgSat.value = (float)sats;
            globalDisplayManager->sendToQueue(msgSat);

            // Geschwindigkeit (bestehend)
            if (!isnan(speed)) {
                UiMsg msgSpeed;
                msgSpeed.type = UiMsg::UPDATE_SPEED;
                msgSpeed.value = speed;
                globalDisplayManager->sendToQueue(msgSpeed);
            }

            // Zeit (bestehend)
            if (hour >= 0) {
                char timeStr[16];
                snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
                UiMsg msgT;
                msgT.type = UiMsg::UPDATE_TIME;
                strncpy(msgT.text, timeStr, sizeof(msgT.text));
                globalDisplayManager->sendToQueue(msgT);
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

