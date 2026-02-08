#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

// UI-Nachrichten-Struktur
struct UiMsg {
    enum MsgType {
        UPDATE_LEAN,
        UPDATE_TIME,
        UPDATE_SPEED,
        UPDATE_INFO,
        UPDATE_TIRE_LEFT,
        UPDATE_ENGINE,
        UPDATE_TIRE_RIGHT,
        UPDATE_AMBIENT,
        UPDATE_HUMIDITY
    };
    
    MsgType type;
    float value;
    char text[32];
    uint16_t bgColor;
    uint16_t fgColor;
};

class DisplayManager {
public:
    DisplayManager(Adafruit_ST7789* tft);
    
    void computeLayout();
    void drawBootScreen();
    void updateUI();
    
    // UI update methods
    void setLeanAngle(float angle);
    void setTime(const char* timeStr);
    void setSpeed(float speed);
    void setInfoText(const char* text);
    void setTireTempLeft(float temp, uint16_t bgColor = 0, uint16_t fgColor = 0);
    void setTireTempRight(float temp, uint16_t bgColor = 0, uint16_t fgColor = 0);
    void setEngineTemp(float temp, uint16_t bgColor = 0, uint16_t fgColor = 0);
    void setAmbientTemp(float temp);
    void setHumidity(float humidity);
    
    // Für Task-Kommunikation
    void setMutexAndQueue(SemaphoreHandle_t mutex, QueueHandle_t queue);

    // Send message to UI queue
    bool sendToQueue(const UiMsg& msg) {
        if (!_uiQueue) return false;
        return xQueueSend(_uiQueue, &msg, 0) == pdTRUE;
    }
    
private:
    Adafruit_ST7789* _tft;
    
    // Layout-Bereiche (wie im alten Code)
    struct Area {
        int16_t x, y, width, height;
        Area() : x(0), y(0), width(0), height(0) {}
        Area(int16_t x, int16_t y, int16_t w, int16_t h) : x(x), y(y), width(w), height(h) {}
    };
    
    Area _leanCircleArea;
    Area _timeBoxArea;
    Area _speedBoxArea;
    Area _infoLineArea;
    Area _tireLeftArea;
    Area _engineArea;
    Area _tireRightArea;
    Area _ambientBoxArea;
    Area _humidityBoxArea;
    
    // Aktuelle Werte
    float _leanAngle = 0.0;
    char _timeText[32] = "--:--";
    float _speed = 0.0;
    char _infoText[32] = "System Ready";
    float _tireTempLeft = 0.0;
    float _tireTempRight = 0.0;
    float _engineTemp = 0.0;
    float _ambientTemp = 0.0;
    float _humidity = 0.0;
    uint16_t _tireLeftBgColor = ST77XX_BLACK;
    uint16_t _tireLeftFgColor = ST77XX_WHITE;
    uint16_t _tireRightBgColor = ST77XX_BLACK;
    uint16_t _tireRightFgColor = ST77XX_WHITE;
    uint16_t _engineBgColor = ST77XX_BLUE;
    uint16_t _engineFgColor = ST77XX_WHITE;
    
    // RTOS-Handles
    SemaphoreHandle_t _tftMutex = nullptr;
    QueueHandle_t _uiQueue = nullptr;
    
    // Private Methoden
    void drawFrame();
    void drawLeanCircle();
    void drawTimeBox();
    void drawSpeedBox();
    void drawInfoLine();
    void drawTempCircle(const Area& area, const char* label, const char* value, uint16_t fillColor, uint16_t textColor);
    void drawTempCircleFloat(const Area& area, const char* label, float value, uint16_t fillColor, uint16_t textColor);
    void drawAmbientBox();
    void drawHumidityBox();
    void processQueue();
    uint16_t getTempColor(float temp);
    uint16_t autoTextColor(uint16_t fill565);
    void safePrintf(int16_t x, int16_t y, uint16_t color, const char* format, ...);
    
    // Hilfsfunktionen
    static inline int16_t i16max(int16_t a, int16_t b) { return (a > b) ? a : b; }
    static inline int16_t i16min(int16_t a, int16_t b) { return (a < b) ? a : b; }
};

#endif