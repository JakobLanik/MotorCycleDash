#include "SDCardManager.h"
#include <Arduino.h>

// Standard-Pins für ESP32
#define DEFAULT_CS_PIN   5
#define DEFAULT_SCK_PIN  18
#define DEFAULT_MISO_PIN 19
#define DEFAULT_MOSI_PIN 23

SDCardManager::SDCardManager() {
    // Standard-Pins setzen
    csPin = DEFAULT_CS_PIN;
    sckPin = DEFAULT_SCK_PIN;
    misoPin = DEFAULT_MISO_PIN;
    mosiPin = DEFAULT_MOSI_PIN;
    initialized = false;
}

// init() ohne Parameter (verwendet Standard-Pins)
bool SDCardManager::init() {
    return init(DEFAULT_CS_PIN, DEFAULT_SCK_PIN, DEFAULT_MISO_PIN, DEFAULT_MOSI_PIN);
}

// init() mit Parametern
bool SDCardManager::init(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi) {
    // Pins speichern
    csPin = cs;
    sckPin = sck;
    misoPin = miso;
    mosiPin = mosi;
    
    Serial.println("========== SD CARD INIT ==========");
    
    // SPI initialisieren
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
    
    // SD-Karte initialisieren
    Serial.printf("Using pins: CS=%d, SCK=%d, MISO=%d, MOSI=%d\n", 
                  csPin, sckPin, misoPin, mosiPin);
    
    if (!SD.begin(csPin)) {
        Serial.println("SD Card initialization FAILED!");
        Serial.println("Check:");
        Serial.println("1. Is SD card inserted?");
        Serial.println("2. Are pins correctly connected?");
        Serial.println("3. Is the card formatted (FAT32)?");
        initialized = false;
        return false;
    }
    
    // Kartentyp ermitteln
    uint8_t cardType = SD.cardType();
    
    if (cardType == CARD_NONE) {
        Serial.println("No SD card detected!");
        initialized = false;
        return false;
    }
    
    Serial.print("SD Card Type: ");
    switch (cardType) {
        case CARD_MMC:
            Serial.println("MMC");
            break;
        case CARD_SD:
            Serial.println("SDSC");
            break;
        case CARD_SDHC:
            Serial.println("SDHC");
            break;
        default:
            Serial.println("UNKNOWN");
            break;
    }
    
    // Kartengröße anzeigen
    uint64_t cardSize = SD.cardSize() / (1024 * 1024); // MB
    uint64_t usedSpace = SD.usedBytes() / (1024 * 1024); // MB
    uint64_t freeSpace = (SD.cardSize() - SD.usedBytes()) / (1024 * 1024); // MB
    
    Serial.printf("Card Size: %llu MB\n", cardSize);
    Serial.printf("Used Space: %llu MB\n", usedSpace);
    Serial.printf("Free Space: %llu MB\n", freeSpace);
    
    // Root-Verzeichnis auflisten
    Serial.println("Root directory:");
    listFiles();
    
    initialized = true;
    Serial.println("SD Card initialized successfully!");
    Serial.println("==================================");
    
    return true;
}

bool SDCardManager::isPresent() {
    return initialized && (SD.cardType() != CARD_NONE);
}

bool SDCardManager::writeLine(const String& data) {
    return writeLogLine(data);
}

bool SDCardManager::createLogFile(const char* filename) {
    if (!initialized) {
        Serial.println("SD Card not initialized!");
        return false;
    }
    
    // Prüfen ob Datei existiert
    if (SD.exists(filename)) {
        Serial.printf("Log file '%s' already exists\n", filename);
        
        // Größe der bestehenden Datei anzeigen
        File file = SD.open(filename);
        if (file) {
            Serial.printf("Existing file size: %d bytes\n", file.size());
            file.close();
        }
        return true;
    }
    
    // Neue Datei erstellen
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.printf("Failed to create file '%s'\n", filename);
        return false;
    }
    
    // CSV-Header schreiben
    String header = "timestamp,temp_left,temp_right,engine_temp,ambient_temp,humidity,";
    header += "accel_x,accel_y,accel_z,latitude,longitude,speed_kmh";
    
    if (file.println(header)) {
        file.close();
        Serial.printf("Created new log file: '%s'\n", filename);
        Serial.printf("Header: %s\n", header.c_str());
        return true;
    } else {
        file.close();
        Serial.printf("Failed to write header to '%s'\n", filename);
        return false;
    }
}

File SDCardManager::openFile(const char* filename, const char* mode) {
    if (!initialized) {
        Serial.println("SD Card not initialized!");
        return File();
    }
    
    return SD.open(filename, mode);
}

bool SDCardManager::fileExists(const char* filename) {
    if (!initialized) return false;
    return SD.exists(filename);
}

bool SDCardManager::writeLogLine(const String& data) {
    if (!initialized) return false;
    
    File file = SD.open("/data.csv", FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open log file for writing");
        return false;
    }
    
    bool success = file.println(data);
    file.close();
    
    // Alle 100 Zeilen Flush erzwingen
    static int writeCounter = 0;
    if (++writeCounter >= 100) {
        writeCounter = 0;
        file.flush();
    }
    
    return success;
}

uint64_t SDCardManager::getFreeSpace() {
    if (!initialized) return 0;
    return SD.totalBytes() - SD.usedBytes();
}

uint64_t SDCardManager::getCardSize() {
    if (!initialized) return 0;
    return SD.cardSize();
}

uint8_t SDCardManager::getCardType() {
    if (!initialized) return CARD_NONE;
    return SD.cardType();
}

void SDCardManager::listFiles() {
    if (!initialized) {
        Serial.println("SD Card not initialized!");
        return;
    }
    
    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open root directory");
        return;
    }
    
    if (!root.isDirectory()) {
        Serial.println("Root is not a directory!");
        root.close();
        return;
    }
    
    File file = root.openNextFile();
    int fileCount = 0;
    
    while (file) {
        fileCount++;
        if (file.isDirectory()) {
            Serial.printf("  DIR : %s\n", file.name());
        } else {
            Serial.printf("  FILE: %s (%.1f KB)\n", 
                         file.name(), 
                         file.size() / 1024.0);
        }
        file = root.openNextFile();
    }
    
    if (fileCount == 0) {
        Serial.println("  (empty)");
    }
    
    root.close();
}

void SDCardManager::setupSPIPins() {
    // SPI mit spezifischen Pins initialisieren
    SPI.begin(sckPin, misoPin, mosiPin, csPin);
}