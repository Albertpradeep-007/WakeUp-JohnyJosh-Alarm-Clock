#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Wire.h>
#include "RTClib.h"
#include "SPIFFS.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

// Pin Definitions
#define DHTPIN      27
#define DHTTYPE     DHT11
#define BUZZER_PIN  32
#define TFT_CS      5
#define TFT_RST     4
#define TFT_DC      2
#define TFT_SCLK    18
#define TFT_MOSI    23
#define TRIG_PIN    13
#define ECHO_PIN    12
#define PIR_PIN     33  // PIR motion sensor

// Modules
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;
WebServer server(80);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// WiFi Credentials
const char* ssid = "ESP32-Alarm";
const char* password = "12345678";

// Globals
String alarmTime = "--:--";
bool alarmTriggered = false;
unsigned long alarmStartTime = 0;
unsigned long alarmDuration = 60000;  // Alarm sound duration
unsigned long lastActivity = 0;       // To track idle time
const unsigned long idleTimeout = 300000;  // 5 minutes in milliseconds

void updateDisplay(String msg) {
  tft.fillRect(5, 120, 200, 20, ST77XX_BLACK);
  tft.setCursor(5, 120);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.print(msg);
}

void resetAlarm(String reason) {
  digitalWrite(BUZZER_PIN, LOW);
  alarmTriggered = false;
  alarmTime = "--:--";
  updateDisplay("Alarm Stopped (" + reason + ")");
  Serial.println("Alarm Stopped: " + reason);
}

void measureGestureStop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  int distance = duration * 0.034 / 2;

  if (distance > 0 && distance < 10) {
    resetAlarm("Gesture");
  }
}

// Web Handlers
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void handleSetRTC() {
  if (server.hasArg("date") && server.hasArg("time")) {
    int y = server.arg("date").substring(0, 4).toInt();
    int m = server.arg("date").substring(5, 7).toInt();
    int d = server.arg("date").substring(8).toInt();
    int hh = server.arg("time").substring(0, 2).toInt();
    int mm = server.arg("time").substring(3, 5).toInt();
    rtc.adjust(DateTime(y, m, d, hh, mm, 0));
    server.send(200, "text/plain", "RTC updated");
  } else {
    server.send(400, "text/plain", "Missing date or time");
  }
}

void handleSetAlarm() {
  if (server.hasArg("time")) {
    alarmTime = server.arg("time");
    alarmTriggered = false;
    server.send(200, "text/plain", "Alarm set");
  } else {
    server.send(400, "text/plain", "Missing time");
  }
}

void handleGetData() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp)) temp = 0;
  if (isnan(hum))  hum = 0;
  String json = "{\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}";
  server.send(200, "application/json", json);
}

// Puts the display and WiFi into sleep mode when idle
void enterSleepIfIdle() {
  updateDisplay("Sleeping...");
  delay(200);
  tft.fillScreen(ST77XX_BLACK);
  WiFi.softAPdisconnect(true);

  // Configure PIR to wake up the ESP32 from light sleep
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, 1); // PIR HIGH wakes up
  esp_light_sleep_start();

  // On wake, reinitialize display and WiFi AP
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  WiFi.softAP(ssid, password);
  Serial.println("Woke up from PIR motion");

  // Reset idle timer on wake
  lastActivity = millis();
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  dht.begin();
  Wire.begin(21, 22);
  rtc.begin();

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  WiFi.softAP(ssid, password);
  Serial.println("AP IP: " + WiFi.softAPIP().toString());

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  server.on("/", handleRoot);
  server.on("/setrtc", handleSetRTC);
  server.on("/setalarm", handleSetAlarm);
  server.on("/getdata", handleGetData);
  server.begin();

  // Initialize the idle timer
  lastActivity = millis();
}

void loop() {
  server.handleClient();

  DateTime now = rtc.now();
  char currentTime[6];
  sprintf(currentTime, "%02d:%02d", now.hour(), now.minute());

  // Check for motion with the PIR sensor and update lastActivity if motion detected
  if (digitalRead(PIR_PIN) == HIGH) {
    lastActivity = millis();
  }

  // Trigger alarm if current time matches set alarm time
  if (String(currentTime) == alarmTime && !alarmTriggered) {
    digitalWrite(BUZZER_PIN, HIGH);
    alarmTriggered = true;
    alarmStartTime = millis();
    updateDisplay("Alarm Triggered");
    Serial.println("Alarm Triggered");
  }

  // If alarm is active, check if duration expired or if gesture stops alarm
  if (alarmTriggered) {
    if (millis() - alarmStartTime > alarmDuration) {
      resetAlarm("Timeout");
    } else {
      measureGestureStop();
    }
  }

  // Display updates
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(5, 0);
  tft.printf("%02d/%02d/%d", now.day(), now.month(), now.year());

  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.setCursor(10, 25);
  tft.printf("%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.setCursor(5, 70);
  tft.printf("Temp: %.1f C", isnan(temp) ? 0.0 : temp);
  tft.setCursor(5, 85);
  tft.printf("Hum : %.1f %%", isnan(hum) ? 0.0 : hum);

  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(5, 105);
  tft.printf("Alarm: %s", alarmTime.c_str());

  // Check for 5 minutes idle (only if no alarm is set/triggered)
  if (!alarmTriggered && alarmTime == "--:--" && (millis() - lastActivity > idleTimeout)) {
    enterSleepIfIdle();
  }

  delay(1000);
}
