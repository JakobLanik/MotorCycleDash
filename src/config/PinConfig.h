#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ============================================
// SPI BUS 1: VSPI (Display & SD Karte)
// ============================================
// Diese Komponenten teilen sich den schnellen VSPI Bus
#define VSPI_SCK    18    
#define VSPI_MISO   19    
#define VSPI_MOSI   23    

#define TFT_CS       5     // Chip Select Display
#define TFT_DC      16     // Data/Command
#define TFT_RST     17     // Reset
#define TFT_BL      -1     // Backlight (nicht verwendet)
#define SD_CS       26     // Chip Select SD-Karte (Strapping Pin, meist OK)

// ============================================
// SPI BUS 2: HSPI (Beschleunigungssensor / IMU)
// ============================================
// Getrennter Bus für die IMU, um Timing-Konflikte zu vermeiden
#define ADXL_SCK    14    
#define ADXL_MISO   12    
#define ADXL_MOSI   13    
#define ADXL_CS     27    

// ============================================
// I2C BUSSE (Reifen-Temperatursensoren)
// ============================================
// BUS 0: Linker Reifen
#define I2C_SDA     21
#define I2C_SCL     22

// BUS 1: Rechter Reifen (verhindert Adress-Konflikt 0x5A)
#define I2C1_SDA    32
#define I2C1_SCL    33

// ============================================
// SONSTIGE SENSOREN & SYSTEM
// ============================================
// GPS (UART2)
#define GPS_RX       4     // GPS TX -> ESP RX
#define GPS_TX      -1     // Nicht benötigt (ESP sendet nichts an GPS)

// DHT22 (Digital)
#define DHT_PIN     25     
#define DHTPIN      25     // Doppelte Definition für Kompatibilität

// Engine Thermistor (Analog)
// WICHTIG: GPIO 34 ist ADC1 (funktioniert stabil mit WiFi)
#define ENGINE_ADC  34     

// Status LED
#define LED_PIN      2     

#endif