#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCardManager {
public:
    // Konstruktor
    SDCardManager();
    
    // Initialisierungsmethoden
    bool init();  // Ohne Parameter (verwendet Standard-Pins)
    bool init(uint8_t csPin, uint8_t sckPin, uint8_t misoPin, uint8_t mosiPin);
    
    // Statusmethoden
    bool isPresent();
    
    // Dateioperationen
    bool createLogFile(const char* filename = "/data.csv");
    File openFile(const char* filename, const char* mode = FILE_WRITE);
    bool fileExists(const char* filename);
    
    // Schreibmethoden (beide gehen)
    bool writeLogLine(const String& data);
    bool writeLine(const String& data);  // Alias für writeLogLine
    
    // Info-Methoden
    uint64_t getFreeSpace();
    uint64_t getCardSize();
    uint8_t getCardType();
    
    // Debug-Methoden
    void listFiles();

private:
    uint8_t csPin;
    uint8_t sckPin;
    uint8_t misoPin;
    uint8_t mosiPin;
    bool initialized;
    
    void setupSPIPins();
};

#endif