# 🏍️ MotorCycleDash

**MotorCycleDash** ist ein hochentwickeltes Telemetrie- und Dashboardsystem für Motorräder auf Basis des ESP32. Das System erfasst fahrdynamische Daten in Echtzeit, visualisiert diese auf einem TFT-Display und loggt sie parallel für spätere Analysen auf einer SD-Karte.

Dank der Nutzung von **FreeRTOS** arbeitet das System vollkommen asynchron. Das bedeutet, dass rechenintensive Aufgaben wie das Schreiben auf die SD-Karte oder das Parsen von GPS-Daten niemals das flüssige Update des Displays (UI) blockieren.



## ✨ Features

* **Echtzeit-Schräglagenanzeige:** Berechnung von Roll- und Nickwinkel (Lean & Pitch) über einen ADXL345 Beschleunigungssensor.
* **GPS-Telemetrie:** Präzise Geschwindigkeit (km/h), Zeitstempel, Satellitenanzahl und Standortkoordinaten via TinyGPS++.
* **Duales Infrarot-Reifenmonitoring:** Kontaktlose Temperaturmessung der linken und rechten Reifenflanke (2x MLX90614 auf getrennten I2C-Bussen).
* **Motortemperatur-Überwachung:** Messung via Thermistor mit **Exponential Moving Average (EMA)** Filterung zur Unterdrückung von Bordnetzrauschen.
* **Umgebungsdaten:** Luftfeuchtigkeit und Umgebungstemperatur über einen DHT22 Sensor.
* **Intelligentes Logging:** Speicherung aller Daten im `.csv`-Format auf SD-Karte (200ms Intervall). Neue Fahrten werden automatisch als neue Dateien angelegt (`data_001.csv`, etc.).
* **Multitasking-Architektur:** Volle Ausnutzung des ESP32 Dual-Core Prozessors mit FreeRTOS Queues und Mutex-Sperren für thread-sichere SPI/I2C-Kommunikation.

---

## 🛠️ Hardware & Pinbelegung

| Komponente | Pin (ESP32) | Protokoll |
| :--- | :--- | :--- |
| **TFT Display (ST7789)** | SCK: 18, MOSI: 23, CS: 15, DC: 2, RST: 4 | SPI |
| **SD-Kartenslot** | SCK: 18, MISO: 19, MOSI: 23, CS: 5 | SPI |
| **IMU (ADXL345)** | SDA: 21, SCL: 22 | I2C (Bus 0) |
| **IR-Sensor Links** | SDA: 21, SCL: 22 | I2C (Bus 0) |
| **IR-Sensor Rechts** | SDA: 32, SCL: 33 | I2C (Bus 1) |
| **GPS Modul** | TX: 17, RX: 16 | UART2 |
| **Motor-Thermistor** | GPIO 34 | Analog (ADC) |
| **DHT22** | GPIO 27 | 1-Wire |

---

## 🏗️ Software-Struktur

Das Projekt folgt einem modularen **Manager-Pattern**:

* **`SensorBase`**: Eine abstrakte Basisklasse, die ein einheitliches Interface für alle Sensoren vorgibt.
* **`TaskManager`**: Das Herzstück der Steuerung. Er initialisiert die FreeRTOS-Tasks und regelt die Prioritäten.
* **`DisplayManager`**: Verwaltet die `uiQueue`. Alle Sensoren senden ihre Daten in diese Queue, damit der Display-Task sie ohne SPI-Konflikte nacheinander zeichnen kann.
* **`DataLogger`**: Sammelt die aktuellen Werte und berechnet Statistiken (wie z.B. maximale Schräglage oder Höchstgeschwindigkeit), bevor sie auf die SD-Karte geschrieben werden.

---

## 🚀 Installation

1.  Klone das Repository:
    ```bash
    git clone [https://github.com/JakobLanik/MotorCycleDash.git](https://github.com/JakobLanik/MotorCycleDash.git)
    ```
2.  Öffne das Projekt in **PlatformIO** (empfohlen) oder der Arduino IDE.
3.  Installiere die benötigten Bibliotheken:
    * Adafruit GFX & ST7789
    * Adafruit MLX90614
    * TinyGPSPlus
    * DHT sensor library
4.  Passe ggf. die Schwellenwerte für die Temperaturwarnung in `src/config/Constants.h` an.
5.  Flashe den ESP32.

---

## 📈 Datenformat (CSV)

Die Log-Dateien sind wie folgt aufgebaut:
`Timestamp, GPS_Time, Lean, Pitch, Speed, Tire_L, Tire_R, Engine_Temp, Lat, Lon, Sats`

---

## 📝 Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Siehe `LICENSE` für Details.

**Entwickelt von [JakobLanik](https://github.com/JakobLanik)**
