#include "SDCardManager.h"
#include <Arduino.h>

// Konstruktor - Initialisiert die Variablen
SDCardManager::SDCardManager() : 
    csPin(5), 
    sckPin(18), 
    misoPin(19), 
    mosiPin(23), 
    initialized(false), 
    activeLogFile("/data.csv") 
{}

// Die Haupt-Initialisierung, die von deiner main.cpp aufgerufen wird
bool SDCardManager::init(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs) {
    sckPin = sck;
    misoPin = miso;
    mosiPin = mosi;
    csPin = cs;
    
    Serial.println("========== SD CARD INIT ==========");
    
    // SPI Instanz mit den übergebenen Pins konfigurieren
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
    
    Serial.printf("Using pins: SCK=%d, MISO=%d, MOSI=%d, CS=%d\n", sckPin, misoPin, mosiPin, csPin);
    
    if (!SD.begin(csPin)) {
        Serial.println("SD Card Mount FAILED!");
        initialized = false;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card detected!");
        initialized = false;
        return false;
    }

    Serial.printf("SD Card initialized. Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    
    initialized = true;
    Serial.println("==================================");
    return true;
}

// Fallback init ohne Parameter (verwendet Default Pins)
bool SDCardManager::init() {
    return init(18, 19, 23, 5);
}

bool SDCardManager::isPresent() {
    return initialized && (SD.cardType() != CARD_NONE);
}

// Erstellt die Datei und schreibt den Header
bool SDCardManager::createLogFile(const char* filename) {
    if (!initialized) return false;
    
    // Speichere den Dateinamen für spätere Schreibvorgänge
    activeLogFile = String(filename);
    if (!activeLogFile.startsWith("/")) {
        activeLogFile = "/" + activeLogFile;
    }

    // Wir suchen nach einer freien Nummer, damit nichts überschrieben wird
    char finalPath[32];
    String base = activeLogFile;
    if (base.endsWith(".csv")) base = base.substring(0, base.length() - 4);

    for (int i = 1; i < 1000; i++) {
        snprintf(finalPath, sizeof(finalPath), "%s_%03d.csv", base.c_str(), i);
        if (!SD.exists(finalPath)) {
            activeLogFile = String(finalPath);
            break;
        }
    }
    
    File file = SD.open(activeLogFile, FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create file '%s'\n", activeLogFile.c_str());
        return false;
    }
    
    // Dieser Header muss EXAKT zur Reihenfolge im DataLogger.cpp passen!
    // WICHTIG: gps_time hinzugefügt, wie im TaskManager/DataLogger gefordert
    String header = "timestamp,gps_time,lean,pitch,speed,tireL,tireR,engine,amb_temp,humidity,lat,lon,sats,";
    header += "maxLeanL,maxLeanR,maxSpeed,maxTireL,maxTireR,maxEng,maxPitch";
    
    if (file.println(header)) {
        Serial.printf("New log file created: %s\n", activeLogFile.c_str());
        file.flush();
        file.close();
        return true;
    } else {
        file.close();
        return false;
    }
}

// Schreibt eine Zeile in die aktive Log-Datei
bool SDCardManager::writeLogLine(const String& data) {
    if (!initialized) return false;
    
    File file = SD.open(activeLogFile, FILE_APPEND); 
    if (!file) {
        return false;
    }
    
    if (file.println(data)) {
        file.flush(); // Datensicherheit bei Stromausfall/Vibration
        file.close();
        return true;
    }
    file.close();
    return false;
}

// Alias für writeLogLine
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
    return (SD.totalBytes() - SD.usedBytes()) / (1024 * 1024);
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
    Serial.println("--- Files on SD Card ---");
    while (file) {
        Serial.printf("  %s  %u bytes\n", file.name(), file.size());
        file = root.openNextFile();
    }
    Serial.println("------------------------");
    root.close();
}

void SDCardManager::setupSPIPins() {
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
}