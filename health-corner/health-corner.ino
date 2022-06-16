#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MAX30100_PulseOximeter.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define REPORTING_PERIOD_MS 1000
#define WIFI_SSID "HASAN"
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

double sensorValue(double value) {
  int i;
  double arr[25];
  double sum;
  double avgrage;
  for (i = 0; i <= 25; i++) {
    arr[i] = value;
  }
  for (i = 0; i <= 25; i++) {
    sum = sum + arr[i];
  }
  avgrage = sum / 25;
  return avgrage;
}

void loop() {
  pox.update();
  double bpm = pox.getHeartRate();
  double spo2 = pox.getSpO2();
  double bodyTempC = mlx.readObjectTempC();
  double bodyTempF = mlx.readObjectTempF();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    Serial.println();
    Serial.print(bpm);
    Serial.print(" BPM | SpO2: ");
    Serial.print(spo2);
    Serial.println("%");
    Serial.print(bodyTempC);
    Serial.print("°C | ");
    Serial.print(bodyTempF);
    Serial.println("°F");
    tsLastReport = millis();
  }
  webServer.handleClient();
}

void setupWiFi() {
  Serial.println("WIFI SETUP");
  Serial.print("CONNECTED WITH: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("CONNECTED SUCCESSFULLY");
  Serial.print("YOUR ID ADDRESS: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  Serial.println("SETUP");
  webServer.on("/", handleRoot);
  Serial.println("STARTING...");
  webServer.begin();
  Serial.println("RUNNING");
}

void handleRoot() {
  Serial.println("REQUEST SUCCESSFUL");
  pox.update();

  double bpm = pox.getHeartRate();
  double spo2 = pox.getSpO2();
  double bodyTempC = mlx.readObjectTempC();
  double bodyTempF = mlx.readObjectTempF();

  double avgBpm = sensorValue(bpm);
  double avgSpo2 = sensorValue(spo2);
  double avgBodyTempC = sensorValue(bodyTempC);
  double avgBodyTempF = sensorValue(bodyTempF);

  String response = "";
  response += "[";
  response += "{";
  response += "\"bodyTempC\": ";
  response += bodyTempC;
  response += ",";
  response += "\"bodyTempF\": ";
  response += bodyTempF;
  response += ",";
  response += "\"bpm\": ";
  response += bpm;
  response += ",";
  response += "\"spo2\": ";
  response += spo2;

  if (avgBpm >= 60.00 && avgSpo2 >= 90.00) {
    response += ",";
    response += "\"avgBpm\": ";
    response += avgBpm;
    response += ",";
    response += "\"avgSpo2\": ";
    response += avgSpo2;
    response += ",";
  }
  if (avgBodyTempC >= 37.2 && avgBodyTempF >= 98.6) {
    response += "\"avgBodyTempC\": ";
    response += avgBodyTempC;
    response += ",";
    response += "\"avgBodyTempF\": ";
    response += avgBodyTempF;
  }

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
