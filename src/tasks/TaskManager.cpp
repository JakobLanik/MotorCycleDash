#include "TaskManager.h"
#include <Arduino.h>
#include "../config/Constants.h"
#include "../display/DisplayManager.h"

// Globale Zeiger auf die Hardware-Instanzen
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
    
    globalDisplayManager = displayManager;
    globalDataLogger = dataLogger;
    globalTireSensorLeft = tireSensorLeft;
    globalTireSensorRight = tireSensorRight;
    globalDhtSensor = dhtSensor;
    globalEngineSensor = engineSensor;
    globalImuSensor = imuSensor;
    globalGpsSensor = gpsSensor;
    
    Serial.println("========== STARTING TASKS ==========");
    
    xTaskCreatePinnedToCore(taskUITick, "UI_Task", 4096, NULL, TASK_PRIORITY_UI, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskDHT22, "DHT22_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskTireL, "TireL_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskTireR, "TireR_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskEngine, "Engine_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskIMU, "IMU_Task", 4096, NULL, TASK_PRIORITY_IMU, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskGPS, "GPS_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(taskDataLogger, "Logger_Task", 4096, NULL, TASK_PRIORITY_SENSORS, NULL, APP_CPU_NUM);
    
    Serial.println("All tasks started successfully!");
}

// Hilfsfunktionen für Farben
static uint16_t getTempColor(float temp) {
    if (temp < 20.0) return ST77XX_BLUE;
    if (temp < 40.0) return ST77XX_CYAN;
    if (temp < 60.0) return ST77XX_GREEN;
    if (temp < 80.0) return ST77XX_YELLOW;
    if (temp < 100.0) return ST77XX_ORANGE;
    return ST77XX_RED;
}

static uint16_t getTextColor(uint16_t bgColor) {
    uint8_t r = ((bgColor >> 11) & 0x1F);
    uint8_t g = ((bgColor >> 5) & 0x3F);
    uint8_t b = (bgColor & 0x1F);
    return (r + g + b < 30) ? ST77XX_WHITE : ST77XX_BLACK;
}

void TaskManager::taskUITick(void* params) {
    while (1) {
        if (globalDisplayManager) globalDisplayManager->updateUI();
        vTaskDelay(pdMS_TO_TICKS(UI_UPDATE_INTERVAL));
    }
}

void TaskManager::taskDHT22(void* params) {
    while (1) {
        if (globalDhtSensor) {
            float temp = globalDhtSensor->readTemperature();
            float hum = globalDhtSensor->readHumidity();
            if (!isnan(temp)) {
                if (globalDataLogger) {
                    globalDataLogger->setAmbientTemp(temp);
                    globalDataLogger->setHumidity(hum);
                }
                UiMsg msg;
                msg.type = UiMsg::UPDATE_AMBIENT;
                msg.value = temp;
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(DHT22_UPDATE_INTERVAL));
    }
}

void TaskManager::taskTireL(void* params) {
    while (1) {
        if (globalTireSensorLeft) {
            float temp = globalTireSensorLeft->readObjectTemp();
            if (!isnan(temp)) {
                uint16_t bg = getTempColor(temp);
                if (globalDataLogger) globalDataLogger->setTireTempL(temp);
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_TIRE_LEFT;
                msg.value = temp;
                msg.bgColor = bg;
                msg.fgColor = getTextColor(bg);
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TIRE_UPDATE_INTERVAL));
    }
}

void TaskManager::taskTireR(void* params) {
    while (1) {
        if (globalTireSensorRight) {
            float temp = globalTireSensorRight->readObjectTemp();
            if (!isnan(temp)) {
                uint16_t bg = getTempColor(temp);
                if (globalDataLogger) globalDataLogger->setTireTempR(temp);
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_TIRE_RIGHT;
                msg.value = temp;
                msg.bgColor = bg;
                msg.fgColor = getTextColor(bg);
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(TIRE_UPDATE_INTERVAL));
    }
}

void TaskManager::taskEngine(void* params) {
    while (1) {
        if (globalEngineSensor) {
            float temp = globalEngineSensor->readSmoothed();
            if (!isnan(temp)) {
                uint16_t bg = getTempColor(temp);
                if (globalDataLogger) globalDataLogger->setEngineTemp(temp);
                
                UiMsg msg;
                msg.type = UiMsg::UPDATE_ENGINE;
                msg.value = temp;
                msg.bgColor = bg;
                msg.fgColor = getTextColor(bg);
                globalDisplayManager->sendToQueue(msg);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(ENGINE_UPDATE_INTERVAL));
    }
}

void TaskManager::taskIMU(void* params) {
    while (1) {
        if (globalImuSensor) {
            float lean = globalImuSensor->calculateRollAngle();
            float pitch = globalImuSensor->calculatePitchAngle();
            if (globalDataLogger) {
                globalDataLogger->setLeanAngle(lean);
                globalDataLogger->setPitchAngle(pitch);
            }
            
            UiMsg msgL;
            msgL.type = UiMsg::UPDATE_LEAN;
            msgL.value = lean;
            globalDisplayManager->sendToQueue(msgL);
            
            UiMsg msgP;
            msgP.type = UiMsg::UPDATE_PITCH;
            msgP.value = pitch;
            globalDisplayManager->sendToQueue(msgP);
        }
        vTaskDelay(pdMS_TO_TICKS(IMU_UPDATE_INTERVAL));
    }
}

void TaskManager::taskGPS(void* params) {
    while (1) {
        if (globalGpsSensor) {
            globalGpsSensor->update();
            
            int sats = globalGpsSensor->getSatellites();
            float speed = globalGpsSensor->getSpeed();
            float lat = globalGpsSensor->getLatitude();
            float lon = globalGpsSensor->getLongitude();
            int h, m, s;
            globalGpsSensor->getTime(h, m, s);

            if (globalDataLogger) {
                globalDataLogger->setGPSData(lat, lon, sats);
                globalDataLogger->setSpeedKmh(speed);
                if (h >= 0) {
                    char tBuf[12];
                    snprintf(tBuf, sizeof(tBuf), "%02d:%02d:%02d", h, m, s);
                    globalDataLogger->setGPSTime(tBuf);
                }
            }

            UiMsg msgS;
            msgS.type = UiMsg::UPDATE_SATS;
            msgS.value = (float)sats;
            globalDisplayManager->sendToQueue(msgS);

            UiMsg msgSp;
            msgSp.type = UiMsg::UPDATE_SPEED;
            msgSp.value = speed;
            globalDisplayManager->sendToQueue(msgSp);

            if (h >= 0) {
                UiMsg msgT;
                msgT.type = UiMsg::UPDATE_TIME;
                snprintf(msgT.text, sizeof(msgT.text), "%02d:%02d", h, m);
                globalDisplayManager->sendToQueue(msgT);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(GPS_UPDATE_INTERVAL));
    }
}

void TaskManager::taskDataLogger(void* params) {
    while (1) {
        if (globalDataLogger) {
            globalDataLogger->logCurrentData();
        }
        vTaskDelay(pdMS_TO_TICKS(LOGGER_UPDATE_INTERVAL));
    }
}