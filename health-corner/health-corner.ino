#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MAX30100_PulseOximeter.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define REPORTING_PERIOD_MS 1000
#define WIFI_SSID "Hasan"
#define WIFI_PASS "76278704H"
#define WEB_SERVER_PORT 80
ESP8266WebServer webServer(WEB_SERVER_PORT);
uint8_t max30100_address = 0x57;
uint8_t mlx90614_address = 0x5A;
uint32_t tsLastReport = 0;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;

void setup() {
  Serial.begin(9600);
  setupWiFi();
  setupWebServer();
  Wire.begin();
  mlx.begin();
  pox.begin();
  setupSensors();
}
void loop() {
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    int i;
    int sensorValue[20];
    int sum = 0;
    int avg = 0;
    int value = pox.getHeartRate();

    for (i = 0; i <= 20; i++) {
      sensorValue[i] = value;
    }

    for (i = 0; i <= 20; i++) {
      sum = sum + sensorValue[i];
    }

    Serial.print("Total: ");
    Serial.println(sum);
    avg = sum / 20;
    Serial.print("Average: ");
    Serial.println(avg);


    Serial.println("Pulse Oximeter:");
    Serial.print(pox.getHeartRate());
    Serial.print(" BPM | SpO2: ");
    Serial.print(pox.getSpO2());
    Serial.println("%");
    Serial.println("Thermometer:");
    Serial.print(mlx.readObjectTempC());
    Serial.print("°C | ");
    Serial.print(mlx.readObjectTempF());
    Serial.println("°F");
    Serial.println();

    tsLastReport = millis();
  }
  webServer.handleClient();
}
void setupWiFi() {
  Serial.println("[WiFi] Setup");
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("[WiFi] Connected!");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}
void setupWebServer() {
  Serial.println("[WebServer] Setup");
  webServer.on("/", handleRoot);
  Serial.println("[WebServer] Starting..");
  webServer.begin();
  Serial.println("[WebServer] Running!");
}
void handleRoot() {
  Serial.println("[WebServer] Request: /");
  pox.update();

  double bpm = pox.getHeartRate();
  double spo2 = pox.getSpO2();
  double bodyTempC = mlx.readObjectTempC();
  double bodyTempF = mlx.readObjectTempF();

  String response = "";
  response += "[";
  response += "{";
  response += "\"body_temp_c\":";
  response += bodyTempC;
  response += ",";
  response += "\"body_temp_f\":";
  response += bodyTempF;
  response += ",";
  response += "\"bpm\":";
  response += bpm;
  response += ",";
  response += "\"spo2\":";
  response += spo2;
  response += "}";
  response += "]";
  webServer.send(200, "application/json", response);
}
void setupSensors() {
  Serial.print("Initializing Pulse Oximeter..");
  if (!pox.begin()) {
    Serial.println("Pulse Oximeter - SETUP FAILED");
    for (;;);
  } else {
    Serial.println("Pulse Oximeter - SETUP SUCCESS");
    digitalWrite(1, HIGH);
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
  Serial.println("Initializing MLX Infrared Thermometer..");
  if (!mlx.begin()) {
    Serial.println("MLX Infrared Thermometer - SETUP FAILED");
    for (;;);
  } else {
    Serial.println("MLX Infrared Thermometer - SETUP SUCCESS");
  }
}
