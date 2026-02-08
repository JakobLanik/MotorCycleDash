#include "DisplayManager.h"
#include <stdarg.h>
#include <math.h>
#include "../config/Constants.h"
#include "../config/DisplayConfig.h"

DisplayManager::DisplayManager(Adafruit_ST7789* tft) : _tft(tft) {
}

void DisplayManager::setMutexAndQueue(SemaphoreHandle_t mutex, QueueHandle_t queue) {
    _tftMutex = mutex;
    _uiQueue = queue;
}

void DisplayManager::computeLayout() {
    // IMPORTANT: Match the old code exactly
    // The old code used setRotation(3) which makes it 320x240 landscape
    const int16_t W = 320;   // Width after rotation 3 (landscape)
    const int16_t H = 240;   // Height after rotation 3 (landscape)
    const int16_t pad = 4;
    
    // Top row: LEAN | TIME | SPEED (EXACTLY like old code)
    const int16_t topH = 80;
    const int16_t topW = (W - pad * 4) / 3;
    
    _leanCircleArea = Area(pad, pad, topW, topH);
    _timeBoxArea = Area(static_cast<int16_t>(pad + topW + pad), pad, topW, topH);
    _speedBoxArea = Area(static_cast<int16_t>(pad + topW * 2 + pad * 2), pad, topW, topH);
    
    // Info line (centered, full width)
    _infoLineArea = Area(pad, static_cast<int16_t>(pad + topH + pad), 
                        static_cast<int16_t>(W - 2 * pad), 28);
    
    // Bottom row: TIRE L | ENGINE | TIRE R
    const int16_t bottomTop = _infoLineArea.y + _infoLineArea.height + pad;
    const int16_t bottomH = 100;
    const int16_t bottomW = (W - pad * 4) / 3;
    
    _tireLeftArea = Area(pad, bottomTop, bottomW, bottomH);
    _engineArea = Area(static_cast<int16_t>(pad + bottomW + pad), bottomTop, bottomW, bottomH);
    _tireRightArea = Area(static_cast<int16_t>(pad + bottomW * 2 + pad * 2), bottomTop, bottomW, bottomH);
    
    // Ambient and Humidity boxes (side by side below temp circles)
    const int16_t ambiTop = static_cast<int16_t>(bottomTop + bottomH + 2);
    const int16_t ambiW = (W - pad * 3) / 2;
    const int16_t ambiH = 18;
    
    _ambientBoxArea = Area(pad, ambiTop, ambiW, ambiH);
    _humidityBoxArea = Area(static_cast<int16_t>(pad + ambiW + pad), ambiTop, ambiW, ambiH);
    
    // Debug output
    Serial.printf("[Display] Layout computed for 320x240 (rotation 3)\n");
    Serial.printf("[Display] Info line: y=%d, h=%d\n", _infoLineArea.y, _infoLineArea.height);
    Serial.printf("[Display] Tire boxes: y=%d, h=%d\n", _tireLeftArea.y, _tireLeftArea.height);
    Serial.printf("[Display] Ambient boxes: y=%d\n", _ambientBoxArea.y);
}

void DisplayManager::drawBootScreen() {
    if (_tftMutex != NULL && xSemaphoreTake(_tftMutex, portMAX_DELAY) == pdTRUE) {
        // Ensure rotation is set correctly
        _tft->setRotation(3);  // CRITICAL: Match old code
        
        // Get actual dimensions after rotation
        int16_t screenWidth = _tft->width();
        int16_t screenHeight = _tft->height();
        
        Serial.printf("[Display] Boot screen: %dx%d (rotation 3)\n", screenWidth, screenHeight);
        
        _tft->fillScreen(ST77XX_BLACK);
        
        // Draw border like in old code (using actual screen dimensions)
        _tft->drawRect(0, 0, screenWidth, screenHeight, _tft->color565(60, 60, 60));
        
        // Draw default values
        drawLeanCircle();
        drawTimeBox();
        drawSpeedBox();
        drawInfoLine();
        
        // Draw temperature circles with default values
        drawTempCircle(_tireLeftArea, "TIRE L", "--.-C", ST77XX_BLACK, ST77XX_WHITE);
        drawTempCircle(_engineArea, "ENGINE", "--.-C", ST77XX_BLUE, ST77XX_WHITE);
        drawTempCircle(_tireRightArea, "TIRE R", "--.-C", ST77XX_BLACK, ST77XX_WHITE);
        
        // Draw ambient and humidity boxes
        drawAmbientBox();
        drawHumidityBox();
        
        xSemaphoreGive(_tftMutex);
    }
}

void DisplayManager::updateUI() {
    if (_uiQueue != NULL) {
        processQueue();
    }
    
    if (_tftMutex != NULL && xSemaphoreTake(_tftMutex, portMAX_DELAY) == pdTRUE) {
        // Update all UI elements with current values
        drawLeanCircle();
        drawTimeBox();
        drawSpeedBox();
        drawInfoLine();
        
        // Use the float version for temperature values
        drawTempCircleFloat(_tireLeftArea, "TIRE L", _tireTempLeft, _tireLeftBgColor, _tireLeftFgColor);
        drawTempCircleFloat(_engineArea, "ENGINE", _engineTemp, _engineBgColor, _engineFgColor);
        drawTempCircleFloat(_tireRightArea, "TIRE R", _tireTempRight, _tireRightBgColor, _tireRightFgColor);
        
        drawAmbientBox();
        drawHumidityBox();
        
        xSemaphoreGive(_tftMutex);
    }
}

void DisplayManager::drawFrame() {
    // Don't clear the screen entirely, just draw borders if needed
    // The actual drawing is done in the individual draw methods
}

void DisplayManager::drawLeanCircle() {
    // 1. Bereich löschen
    _tft->fillRect(_leanCircleArea.x, _leanCircleArea.y, 
                   _leanCircleArea.width, _leanCircleArea.height, ST77XX_BLACK);
    
    int16_t cx = _leanCircleArea.x + _leanCircleArea.width / 2;
    int16_t cy = _leanCircleArea.y + _leanCircleArea.height / 2;
    int16_t radius = i16min(_leanCircleArea.width, _leanCircleArea.height) / 2 - 10;
    
    // 2. Äußere Ringe
    _tft->drawCircle(cx, cy, radius, ST77XX_WHITE);
    _tft->drawCircle(cx, cy, radius - 1, _tft->color565(80, 80, 80));
    
    // 3. Fadenkreuz & Zentrum
    _tft->drawFastHLine(cx - 8, cy, 16, _tft->color565(60, 60, 60));
    _tft->drawFastVLine(cx, cy - 8, 16, _tft->color565(60, 60, 60));
    _tft->fillCircle(cx, cy, 2, ST77XX_WHITE);
    
    // 4. BERECHNUNG PITCH & LEAN (Der Ball)
    // X-Achse: Lean (-45 bis 45 Grad)
    float normLean = constrain(_leanAngle / 45.0f, -1.0f, 1.0f);
    int16_t ballX = cx + (int16_t)(normLean * radius * 0.8f);

    // Y-Achse: Pitch (-30 bis 30 Grad für Bremsen/Beschleunigen)
    float normPitch = constrain(_pitchAngle / 30.0f, -1.0f, 1.0f);
    int16_t ballY = cy - (int16_t)(normPitch * radius * 0.8f);
    
    // 5. Ball Farbe bestimmen (basiert auf Lean)
    uint16_t ballColor;
    float absLean = fabsf(_leanAngle);
    if (absLean < 15) ballColor = ST77XX_GREEN;
    else if (absLean < 30) ballColor = ST77XX_YELLOW;
    else ballColor = ST77XX_RED;
    
    // 6. Ball zeichnen
    _tft->fillCircle(ballX, ballY, 6, ballColor);
    _tft->drawCircle(ballX, ballY, 6, ST77XX_WHITE);
    
    // 7. Text Labels
    _tft->setTextColor(ST77XX_CYAN);
    _tft->setTextSize(1);
    _tft->setCursor(cx - 12, _leanCircleArea.y + 2);
    _tft->print("ATTIT");

    char angleStr[16];
    snprintf(angleStr, sizeof(angleStr), "L%.0f P%.0f", _leanAngle, _pitchAngle);
    _tft->setTextColor(ST77XX_WHITE);
    _tft->setTextSize(1);
    int16_t tw = strlen(angleStr) * 6;
    _tft->setCursor(cx - tw / 2, cy + radius + 2);
    _tft->print(angleStr);
}

void DisplayManager::drawTimeBox() {
    // Clear area
    _tft->fillRect(_timeBoxArea.x, _timeBoxArea.y, 
                   _timeBoxArea.width, _timeBoxArea.height, ST77XX_BLACK);
    
    // Draw border
    _tft->drawRect(_timeBoxArea.x, _timeBoxArea.y, 
                   _timeBoxArea.width, _timeBoxArea.height, _tft->color565(80, 80, 80));
    
    // Draw "TIME" label
    _tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    _tft->setTextSize(1);
    _tft->setCursor(_timeBoxArea.x + _timeBoxArea.width / 2 - 12, _timeBoxArea.y + 4);
    _tft->print("TIME");
    
    // Draw time value (big)
    _tft->setTextColor(ST77XX_GREEN, ST77XX_BLACK);
    _tft->setTextSize(3);
    int16_t valueWidth = strlen(_timeText) * 18;
    _tft->setCursor(_timeBoxArea.x + _timeBoxArea.width / 2 - valueWidth / 2, 
                    _timeBoxArea.y + _timeBoxArea.height / 2 - 12);
    _tft->print(_timeText);
}

void DisplayManager::drawSpeedBox() {
    // Clear area
    _tft->fillRect(_speedBoxArea.x, _speedBoxArea.y, 
                   _speedBoxArea.width, _speedBoxArea.height, ST77XX_BLACK);
    
    // Draw border
    _tft->drawRect(_speedBoxArea.x, _speedBoxArea.y, 
                   _speedBoxArea.width, _speedBoxArea.height, _tft->color565(80, 80, 80));
    
    // Draw "SPEED" label at top
    _tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    _tft->setTextSize(1);
    _tft->setCursor(_speedBoxArea.x + _speedBoxArea.width / 2 - 15, _speedBoxArea.y + 4);
    _tft->print("SPEED");
    
    // Draw speed value
    char speedStr[16];
    if (_speed < 100) {
        snprintf(speedStr, sizeof(speedStr), "%.0f", _speed);
    } else {
        snprintf(speedStr, sizeof(speedStr), "%d", (int)_speed);
    }
    
    _tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    _tft->setTextSize(3);
    int16_t valueWidth = strlen(speedStr) * 18;
    _tft->setCursor(_speedBoxArea.x + _speedBoxArea.width / 2 - valueWidth / 2, 
                    _timeBoxArea.y + _timeBoxArea.height / 2 - 12);
    _tft->print(speedStr);
    
    // Draw "km/h" label next to value (small font)
    _tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    _tft->setTextSize(1);
    _tft->setCursor(_speedBoxArea.x + _speedBoxArea.width / 2 + valueWidth / 2 + 2, 
                    _timeBoxArea.y + _timeBoxArea.height / 2 - 4);
    _tft->print("km/h");
}

void DisplayManager::drawInfoLine() {
    // Clear area
    _tft->fillRect(_infoLineArea.x, _infoLineArea.y, 
                   _infoLineArea.width, _infoLineArea.height, ST77XX_BLACK);
    
    // Draw subtle border
    _tft->drawRect(_infoLineArea.x, _infoLineArea.y, 
                   _infoLineArea.width, _infoLineArea.height, _tft->color565(50, 50, 50));
    
    // Draw text centered
    _tft->setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    _tft->setTextSize(2);
    int16_t textWidth = strlen(_infoText) * 12;
    int16_t xPos = _infoLineArea.x + _infoLineArea.width / 2 - textWidth / 2;
    xPos = i16max(xPos, _infoLineArea.x + 4);
    _tft->setCursor(xPos, _infoLineArea.y + 6);
    _tft->print(_infoText);
}

void DisplayManager::drawTempCircle(const Area& area, const char* label, const char* value, 
                                   uint16_t fillColor, uint16_t textColor) {
    // Clear area
    _tft->fillRect(area.x, area.y, area.width, area.height, ST77XX_BLACK);
    
    int16_t cx = area.x + area.width / 2;
    int16_t cy = area.y + area.height / 2;
    int16_t radius = i16min(area.width, area.height) / 2 - 6;
    
    // Draw filled circle
    _tft->fillCircle(cx, cy, radius, fillColor);
    _tft->drawCircle(cx, cy, radius, ST77XX_WHITE);
    
    // Draw label at top
    _tft->setTextColor(ST77XX_WHITE, fillColor);
    _tft->setTextSize(1);
    int16_t labelWidth = strlen(label) * 6;
    _tft->setCursor(cx - labelWidth / 2, area.y + 6);
    _tft->print(label);
    
    // Draw temperature value
    _tft->setTextColor(textColor, fillColor);
    _tft->setTextSize(2);
    
    int16_t valueWidth = strlen(value) * 12;
    _tft->setCursor(cx - valueWidth / 2, cy - 8);
    _tft->print(value);
}

void DisplayManager::drawTempCircleFloat(const Area& area, const char* label, float value, 
                                        uint16_t fillColor, uint16_t textColor) {
    char tempStr[16];
    if (value < 0.1 && value > -0.1) {
        snprintf(tempStr, sizeof(tempStr), "--.-C");
    } else if (value < 100) {
        snprintf(tempStr, sizeof(tempStr), "%.1fC", value);
    } else {
        snprintf(tempStr, sizeof(tempStr), "%.0fC", value);
    }
    
    drawTempCircle(area, label, tempStr, fillColor, textColor);
}

void DisplayManager::drawAmbientBox() {
    // Clear area
    _tft->fillRect(_ambientBoxArea.x, _ambientBoxArea.y, 
                   _ambientBoxArea.width, _ambientBoxArea.height, ST77XX_BLACK);
    
    // Draw border
    _tft->drawRect(_ambientBoxArea.x, _ambientBoxArea.y, 
                   _ambientBoxArea.width, _ambientBoxArea.height, _tft->color565(60, 60, 60));
    
    // Draw "AMB" label
    _tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    _tft->setTextSize(1);
    _tft->setCursor(_ambientBoxArea.x + 4, _ambientBoxArea.y + 3);
    _tft->print("AMB");
    
    // Draw temperature value
    char tempStr[16];
    if (_ambientTemp < 0.1 && _ambientTemp > -0.1) {
        snprintf(tempStr, sizeof(tempStr), "--.-C");
    } else {
        snprintf(tempStr, sizeof(tempStr), "%.1fC", _ambientTemp);
    }
    
    int16_t wpx = (int16_t)strlen(tempStr) * 6;
    _tft->setCursor(_ambientBoxArea.x + _ambientBoxArea.width - wpx - 4, _ambientBoxArea.y + 3);
    _tft->print(tempStr);
}

void DisplayManager::drawHumidityBox() {
    // Clear area
    _tft->fillRect(_humidityBoxArea.x, _humidityBoxArea.y, 
                   _humidityBoxArea.width, _humidityBoxArea.height, ST77XX_BLACK);
    
    // Draw border
    _tft->drawRect(_humidityBoxArea.x, _humidityBoxArea.y, 
                   _humidityBoxArea.width, _humidityBoxArea.height, _tft->color565(60, 60, 60));
    
    // Draw "HUM" label
    _tft->setTextColor(ST77XX_CYAN, ST77XX_BLACK);
    _tft->setTextSize(1);
    _tft->setCursor(_humidityBoxArea.x + 4, _humidityBoxArea.y + 3);
    _tft->print("HUM");
    
    // Draw humidity value
    char humidStr[16];
    if (_humidity < 0.1 && _humidity > -0.1) {
        snprintf(humidStr, sizeof(humidStr), "--%");
    } else {
        snprintf(humidStr, sizeof(humidStr), "%.0f%%", _humidity);
    }
    
    int16_t wpx = (int16_t)strlen(humidStr) * 6;
    _tft->setCursor(_humidityBoxArea.x + _humidityBoxArea.width - wpx - 4, _humidityBoxArea.y + 3);
    _tft->print(humidStr);
}

void DisplayManager::processQueue() {
    if (!_uiQueue) return;
    
    UiMsg msg;
    while (xQueueReceive(_uiQueue, &msg, 0) == pdTRUE) {
        switch (msg.type) {
            case UiMsg::UPDATE_LEAN:
                _leanAngle = msg.value;
                break;

            case UiMsg::UPDATE_PITCH:
              _pitchAngle = msg.value;
              break;
                
            case UiMsg::UPDATE_TIME:
                strncpy(_timeText, msg.text, sizeof(_timeText) - 1);
                _timeText[sizeof(_timeText) - 1] = '\0';
                break;
                
            case UiMsg::UPDATE_SPEED:
                _speed = msg.value;
                break;
                
            case UiMsg::UPDATE_INFO:
                strncpy(_infoText, msg.text, sizeof(_infoText) - 1);
                _infoText[sizeof(_infoText) - 1] = '\0';
                break;
                
            case UiMsg::UPDATE_TIRE_LEFT:
                _tireTempLeft = msg.value;
                _tireLeftBgColor = msg.bgColor;
                _tireLeftFgColor = msg.fgColor;
                break;
                
            case UiMsg::UPDATE_TIRE_RIGHT:
                _tireTempRight = msg.value;
                _tireRightBgColor = msg.bgColor;
                _tireRightFgColor = msg.fgColor;
                break;
                
            case UiMsg::UPDATE_ENGINE:
                _engineTemp = msg.value;
                _engineBgColor = msg.bgColor;
                _engineFgColor = msg.fgColor;
                break;
                
            case UiMsg::UPDATE_AMBIENT:
                _ambientTemp = msg.value;
                break;
                
            case UiMsg::UPDATE_HUMIDITY:
                _humidity = msg.value;
                break;
        }
    }
}

uint16_t DisplayManager::getTempColor(float temp) {
    if (temp < 20.0) return ST77XX_BLUE;
    if (temp < 40.0) return ST77XX_CYAN;
    if (temp < 60.0) return ST77XX_GREEN;
    if (temp < 80.0) return ST77XX_YELLOW;
    if (temp < 100.0) return ST77XX_ORANGE;
    return ST77XX_RED;
}

uint16_t DisplayManager::autoTextColor(uint16_t fill565) {
    uint8_t r = ((fill565 >> 11) & 0x1F) * 255 / 31;
    uint8_t g = ((fill565 >> 5)  & 0x3F) * 255 / 63;
    uint8_t b = ( fill565        & 0x1F) * 255 / 31;
    int lum = (r * 299 + g * 587 + b * 114) / 1000;
    return (lum > 150) ? ST77XX_BLACK : ST77XX_WHITE;
}

void DisplayManager::safePrintf(int16_t x, int16_t y, uint16_t color, const char* format, ...) {
    char buffer[64];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    _tft->setTextColor(color);
    _tft->setCursor(x, y);
    _tft->print(buffer);
}

// Setter-Methoden
void DisplayManager::setLeanAngle(float angle) {
    _leanAngle = angle;
}

void DisplayManager::setTime(const char* timeStr) {
    strncpy(_timeText, timeStr, sizeof(_timeText) - 1);
    _timeText[sizeof(_timeText) - 1] = '\0';
}

void DisplayManager::setSpeed(float speed) {
    _speed = speed;
}

void DisplayManager::setInfoText(const char* text) {
    strncpy(_infoText, text, sizeof(_infoText) - 1);
    _infoText[sizeof(_infoText) - 1] = '\0';
}

void DisplayManager::setTireTempLeft(float temp, uint16_t bgColor, uint16_t fgColor) {
    _tireTempLeft = temp;
    if (bgColor != 0) _tireLeftBgColor = bgColor;
    if (fgColor != 0) _tireLeftFgColor = fgColor;
}

void DisplayManager::setTireTempRight(float temp, uint16_t bgColor, uint16_t fgColor) {
    _tireTempRight = temp;
    if (bgColor != 0) _tireRightBgColor = bgColor;
    if (fgColor != 0) _tireRightFgColor = fgColor;
}

void DisplayManager::setEngineTemp(float temp, uint16_t bgColor, uint16_t fgColor) {
    _engineTemp = temp;
    if (bgColor != 0) _engineBgColor = bgColor;
    if (fgColor != 0) _engineFgColor = fgColor;
}

void DisplayManager::setAmbientTemp(float temp) {
    _ambientTemp = temp;
}

void DisplayManager::setHumidity(float humidity) {
    _humidity = humidity;
}