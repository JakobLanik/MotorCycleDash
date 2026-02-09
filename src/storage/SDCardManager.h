#ifndef SDCARDMANAGER_H
#define SDCARDMANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

class SDCardManager {
public:
    SDCardManager();

    // Init-Methoden
    bool init(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs);
    bool init(); // Fallback ohne Parameter

    // Status
    bool isPresent();

    // Datei-Operationen
    bool createLogFile(const char* filename);
    
    // Schreib-Methoden (Diese haben gefehlt!)
    bool writeLogLine(const String& data);
    bool writeLine(const String& data);

    // Datei-Hilfsmethoden
    File openFile(const char* filename, const char* mode);
    bool fileExists(const char* filename);
    
    // Info & Debug
    uint64_t getFreeSpace();
    uint64_t getCardSize();
    uint8_t getCardType();
    void listFiles();

private:
    uint8_t csPin;
    uint8_t sckPin;
    uint8_t misoPin;
    uint8_t mosiPin;
    bool initialized;
    String activeLogFile;
    
    void setupSPIPins();
};

#endif