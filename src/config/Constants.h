#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================
// DISPLAY CONSTANTS
// ============================================
#define TFT_WIDTH     240
#define TFT_HEIGHT    320
#define TFT_ROTATION  3
#define TFT_SPI_SPEED 10000000

// ============================================
// I2C CONSTANTS
// ============================================
#define I2C_CLOCK_SPEED 100000
#define MLX1_ADDR 0x5A
#define MLX2_ADDR 0x5A

// ============================================
// DHT22 CONSTANTS
// ============================================
#define DHTTYPE DHT22

// ============================================
// ENGINE THERMISTOR CONSTANTS
// ============================================
#define R_FIXED        100000.0
#define R0_THERM       100000.0
#define BETA_THERM     3950.0
#define T0_K           298.15
#define ADC_BITS       12
#define ADC_MAX        ((1<<ADC_BITS)-1)
#define ADC_VREF       3.30
#define EMA_ALPHA      0.2f
#define ENGINE_WARN_C  95.0
#define ENGINE_ALARM_C 110.0

// ============================================
// GPS CONSTANTS
// ============================================
#define GPS_BAUD 9600

// ============================================
// ADXL345 CONSTANTS
// ============================================
#define ADXL_SPI_HZ 2000000
#define ADXL_REG_DEVID       0x00
#define ADXL_REG_BW_RATE     0x2C
#define ADXL_REG_POWER_CTL   0x2D
#define ADXL_REG_DATA_FORMAT 0x31
#define ADXL_REG_DATAX0      0x32

// ============================================
// SD CARD CONSTANTS
// ============================================
#define LOG_INTERVAL 200

// ============================================
// UI CONSTANTS
// ============================================
#define UI_QUEUE_SIZE 24
#define MAX_UPDATE_COOLDOWN 5000

// ============================================
// TASK PRIORITIES
// ============================================
#define TASK_PRIORITY_UI 3
#define TASK_PRIORITY_IMU 2
#define TASK_PRIORITY_SENSORS 1

// ============================================
// TASK INTERVALS (milliseconds)
// ============================================
#define UI_UPDATE_INTERVAL     100
#define DHT22_UPDATE_INTERVAL  2000
#define TIRE_UPDATE_INTERVAL   1000
#define ENGINE_UPDATE_INTERVAL 1000
#define IMU_UPDATE_INTERVAL    100
#define GPS_UPDATE_INTERVAL    1000
#define LOGGER_UPDATE_INTERVAL 1000

// ============================================
// ESP32 CORE CONFIGURATION
// ============================================
#ifndef APP_CPU_NUM
#define APP_CPU_NUM 1
#endif

#endif // CONSTANTS_H