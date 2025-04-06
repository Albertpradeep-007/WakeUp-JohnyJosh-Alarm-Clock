# WakeUp-JohnyJosh-Alarm-Clock

"WakeUp JohnyJosh Alarm Clock" is a smart alarm clock project built on the ESP32 platform. It features both a web application and a mobile application for setting the real-time clock (RTC) and alarm time, displaying sensor readings (temperature & humidity), detecting motion with a PIR sensor, and managing an automatic sleep mode for the display.


## Features
- **WiFi Access Point:** Runs as an access point (`ESP32-Alarm` with password `12345678`).
- **Web Interface:** Set RTC and alarm time via the hosted web server (accessible at [http://192.168.4.1](http://192.168.4.1)).
- **Sensor Integration:** Uses DHT11 for temperature/humidity readings.
- **PIR Motion Detection:** Wakes the display from sleep mode when motion is detected.
- **Gesture Control:** Stop an active alarm using a simple gesture.
- **Auto Sleep Mode:** Display turns off after 5 minutes of inactivity and wakes up upon detecting motion.

## Hardware Requirements
- ESP32 microcontroller
- DHT11 sensor
- DS3231 RTC module
- ST7735 TFT display
- PIR motion sensor
- Buzzer
- Supporting components (wiring, power supply, etc.)

## Wiring & Pinout
- **DHT11 Sensor:** Data pin connected to GPIO 27.
- **DS3231 RTC Module (I2C):** SDA → GPIO 21, SCL → GPIO 22.
- **ST7735 TFT Display:**  
  - CS  → GPIO 5  
  - RST → GPIO 4  
  - DC  → GPIO 2  
  - SCLK→ GPIO 18  
  - MOSI→ GPIO 23
- **Ultrasonic Sensor (for gesture control):**  
  - TRIG → GPIO 13  
  - ECHO → GPIO 12
- **PIR Motion Sensor:** Signal pin to GPIO 33.
- **Buzzer:** Connected to GPIO 32.

## Software Requirements
- Arduino IDE or PlatformIO
- Required libraries:
  - WiFi.h
  - WebServer.h
  - Adafruit_Sensor.h
  - DHT.h
  - Wire.h
  - RTClib.h
  - SPIFFS.h
  - Adafruit_GFX.h
  - Adafruit_ST7735.h

SPIFFS Data Upload
The project uses the data folder (which contains the index.html file and any other web assets) that will be uploaded to the ESP32's SPIFFS partition.
In the Arduino IDE, use the "ESP32 Sketch Data Upload" tool (available as a plugin) to upload the contents of the data folder to SPIFFS.

Mobile App Conversion (Optional)
If you wish to convert the web interface into a mobile app, the project includes an Alaramapp folder.
Use the Alaramapp folder with the WebIntoApp App Maker to convert your web/HTML code into Android & iOS apps.

Upload the Code

Open the project in the Arduino IDE or PlatformIO.

Install all required libraries using the Library Manager.

Upload the code to your ESP32 board.

Usage
Power up the ESP32 board. It will start a WiFi Access Point named ESP32-Alarm.

Connect your device to this AP using the password 12345678.

Open a browser and navigate to http://192.168.4.1 to access the web interface.

Set the RTC, alarm, and monitor sensor readings using the provided interface.

The display will enter sleep mode after 5 minutes of inactivity and wake up when the PIR sensor detects motion.



## Installation

1. **Clone the Repository**
   ```bash
   git clone https://github.com/Albertpradeep-007/WakeUp_JohnyJosh_AlarmClock.git
