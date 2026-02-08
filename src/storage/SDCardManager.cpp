#include "SDCardManager.h"
#include <Arduino.h>

// Standard-Pins für ESP32 (VSPI)
#define DEFAULT_CS_PIN    5
#define DEFAULT_SCK_PIN   18
#define DEFAULT_MISO_PIN  19
#define DEFAULT_MOSI_PIN  23

SDCardManager::SDCardManager() {
    csPin = DEFAULT_CS_PIN;
    sckPin = DEFAULT_SCK_PIN;
    misoPin = DEFAULT_MISO_PIN;
    mosiPin = DEFAULT_MOSI_PIN;
    initialized = false;
}

bool SDCardManager::init() {
    return init(DEFAULT_CS_PIN, DEFAULT_SCK_PIN, DEFAULT_MISO_PIN, DEFAULT_MOSI_PIN);
}

bool SDCardManager::init(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi) {
    csPin = cs;
    sckPin = sck;
    misoPin = miso;
    mosiPin = mosi;
    
    Serial.println("========== SD CARD INIT ==========");
    
    // SPI mit den definierten Pins starten
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
    
    Serial.printf("Using pins: CS=%d, SCK=%d, MISO=%d, MOSI=%d\n", csPin, sckPin, misoPin, mosiPin);
    
    if (!SD.begin(csPin)) {
        Serial.println("SD Card initialization FAILED!");
        initialized = false;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card detected!");
        initialized = false;
        return false;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC) Serial.println("MMC");
    else if(cardType == CARD_SD) Serial.println("SDSC");
    else if(cardType == CARD_SDHC) Serial.println("SDHC");
    else Serial.println("UNKNOWN");

    Serial.printf("Card Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    
    initialized = true;
    Serial.println("SD Card initialized successfully!");
    Serial.println("==================================");
    
    return true;
}

bool SDCardManager::isPresent() {
    return initialized && (SD.cardType() != CARD_NONE);
}

// Erstellt die Datei und schreibt den Header
bool SDCardManager::createLogFile(const char* filename) {
    if (!initialized) return false;
    
    // Wir überschreiben die Datei beim Start neu (FILE_WRITE), 
    // um einen sauberen Header zu haben.
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create file '%s'\n", filename);
        return false;
    }
    
    // Dieser Header muss EXAKT zur Reihenfolge im DataLogger.cpp passen!
    String header = "timestamp,lean,pitch,speed,tireL,tireR,engine,amb_temp,humidity,lat,lon,sats,";
    header += "maxLeanL,maxLeanR,maxSpeed,maxTireL,maxTireR,maxEng,maxPitch";
    
    if (file.println(header)) {
        Serial.printf("New log file created: %s\n", filename);
        file.flush();
        file.close();
        return true;
    } else {
        file.close();
        return false;
    }
}

// Schreibt eine Zeile und erzwingt den physikalischen Schreibvorgang (Flush)
bool SDCardManager::writeLogLine(const String& data) {
    if (!initialized) return false;
    
    // Immer im Append-Modus öffnen
    File file = SD.open("/data.csv", FILE_APPEND); 
    if (!file) {
        return false;
    }
    
    file.println(data);
    file.flush(); // Zwingend erforderlich für Datensicherheit
    file.close();
    return true;
}

bool SDCardManager::writeLine(const String& data) {
    return writeLogLine(data);
}

File SDCardManager::openFile(const char* filename, const char* mode) {
    if (!initialized) return File();
    return SD.open(filename, mode);
}

bool SDCardManager::fileExists(const char* filename) {
    if (!initialized) return false;
    return SD.exists(filename);
}

uint64_t SDCardManager::getFreeSpace() {
    if (!initialized) return 0;
    return (SD.cardSize() - SD.usedBytes()) / (1024 * 1024);
}

uint64_t SDCardManager::getCardSize() {
    if (!initialized) return 0;
    return SD.cardSize() / (1024 * 1024);
}

uint8_t SDCardManager::getCardType() {
    if (!initialized) return CARD_NONE;
    return SD.cardType();
}

void SDCardManager::listFiles() {
    if (!initialized) return;
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
        Serial.printf("  %s  %u bytes\n", file.name(), file.size());
        file = root.openNextFile();
    }
    root.close();
}

void SDCardManager::setupSPIPins() {
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
}