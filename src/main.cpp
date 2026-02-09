#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <SD.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#include "config/PinConfig.h"
#include "config/Constants.h"
#include "config/DisplayConfig.h"
#include "sensors/MLX90614Sensor.h"
#include "sensors/DHT22Sensor.h"
#include "sensors/EngineThermistor.h"
#include "sensors/ADXL345Sensor.h"
#include "sensors/GPSSensor.h"
#include "display/DisplayManager.h"
#include "storage/SDCardManager.h"
#include "data/DataLogger.h"
#include "tasks/TaskManager.h"

// Display-Objekt
Adafruit_ST7789 tft(&SPI, TFT_CS, TFT_DC, TFT_RST);

// Sensor-Objekte (Adressen aus Constants.h)
MLX90614Sensor tireSensorLeft(MLX1_ADDR);
MLX90614Sensor tireSensorRight(MLX2_ADDR);
DHT22Sensor dhtSensor;
EngineThermistor engineSensor;
ADXL345Sensor imuSensor;
GPSSensor gpsSensor;

// Manager-Objekte
DisplayManager displayManager(&tft);
SDCardManager sdManager;
DataLogger dataLogger(&sdManager);
TaskManager taskManager;

// Globale Mutexe und Queues
QueueHandle_t uiQueue = NULL;
SemaphoreHandle_t tftMutex = NULL;
SemaphoreHandle_t dataMutex = NULL;

void disableWiFiBT() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    btStop();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    esp_bt_mem_release(ESP_BT_MODE_BTDM);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n==================================");
    Serial.println("MotorCycleDash Hardware Booting...");
    Serial.println("==================================");

    // 1. RTOS Ressourcen
    tftMutex = xSemaphoreCreateMutex();
    dataMutex = xSemaphoreCreateMutex();
    uiQueue = xQueueCreate(UI_QUEUE_SIZE, sizeof(UiMsg));

    // 2. Radio aus für ADC Stabilität
    disableWiFiBT();

    // 3. SPI & Display (WICHTIG: CS 5)
    SPI.begin(VSPI_SCK, VSPI_MISO, VSPI_MOSI);
    tft.init(TFT_WIDTH, TFT_HEIGHT);
    tft.setRotation(TFT_ROTATION);
    tft.setSPISpeed(TFT_SPI_SPEED);
    tft.fillScreen(ST77XX_BLACK);
    
    // Display Manager Setup
    displayManager.setMutexAndQueue(tftMutex, uiQueue);
    displayManager.computeLayout();
    displayManager.drawBootScreen();

    // 4. I2C Initialisierung (Zwei Busse)
    Serial.println("Init I2C Bus 0 & 1...");
    Wire.begin(I2C_SDA, I2C_SCL);     // Bus 0 (Pin 21, 22)
    Wire1.begin(I2C1_SDA, I2C1_SCL);  // Bus 1 (Pin 32, 33)

    // 5. Sensoren starten
    Serial.println("Starting Sensors...");
    
    // MLX Sensoren explizit den Bussen zuweisen
    bool mlx1OK = tireSensorLeft.begin(&Wire); 
    bool mlx2OK = tireSensorRight.begin(&Wire1);
    
    bool dhtOK = dhtSensor.begin();
    bool imuOK = imuSensor.begin();
    bool gpsOK = gpsSensor.begin();
    bool engineOK = engineSensor.begin();

    if (imuOK) {
        Serial.println("IMU erkannt. Starte Auto-Zero Kalibrierung...");
        displayManager.setInfoText("KEEP BIKE UPRIGHT..."); // Info ans Display
        displayManager.updateUI();
        
        imuSensor.calibrate(); // Misst 2 Sek. lang den Versatz (deine -103°)
        
        Serial.println("Kalibrierung abgeschlossen!");
        displayManager.setInfoText("CALIBRATION OK");

    }

    Serial.printf("Status: L:%s R:%s DHT:%s IMU:%s GPS:%s ENG:%s\n",
                  mlx1OK?"OK":"FAIL", mlx2OK?"OK":"FAIL", dhtOK?"OK":"FAIL",
                  imuOK?"OK":"FAIL", gpsOK?"OK":"FAIL", engineOK?"OK":"FAIL");

// 6. SD Karte (RICHTIGE INITIALISIERUNG)
Serial.println("Init SD Card...");

// WICHTIG: Nutze die init() Methode des Managers, nicht nur SD.begin()!
// Ersetze 'sdManager' durch den Namen deiner globalen Variable (z.B. sdCardManager)
if (sdManager.init(VSPI_SCK, VSPI_MISO, VSPI_MOSI, SD_CS)) { 
    Serial.println("SD Manager: Initialized");
    
    if (sdManager.createLogFile("/data.csv")) {
        Serial.println("Datei /data.csv bereit!");
    } else {
        Serial.println("Header konnte nicht geschrieben werden");
    }
} else {
    Serial.println("SD Hardware Fehler!");
}

    // 7. Tasks starten
    taskManager.startAllTasks(&displayManager, &dataLogger, 
                             &tireSensorLeft, &tireSensorRight,
                             &dhtSensor, &engineSensor, 
                             &imuSensor, &gpsSensor);

    Serial.println("System Ready!");
}

void loop() {
    // UI Update Loop (verarbeitet die Queue)
    static unsigned long lastUIUpdate = 0;
    if (millis() - lastUIUpdate > UI_UPDATE_INTERVAL) {
        lastUIUpdate = millis();
        displayManager.updateUI(); 
    }

    // Heartbeat & Debug
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        lastLog = millis();
        Serial.printf("Free Heap: %u bytes\n", ESP.getFreeHeap());
        digitalWrite(2, !digitalRead(2));
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
}