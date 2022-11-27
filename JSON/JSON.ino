#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MAX30100_PulseOximeter.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
double bpm;
double spo2;
double bodyTempC;
double bodyTempF;
double roomTempC;
double roomTempF;
IPAddress myIpAddress;

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
    bpm = pox.getHeartRate();
    spo2 = pox.getSpO2();
    bodyTempC = mlx.readObjectTempC();
    bodyTempF = mlx.readObjectTempF();
    roomTempC = mlx.readAmbientTempC();
    roomTempF = mlx.readAmbientTempF();
    displayData();
    tsLastReport = millis();
  }
  webServer.handleClient();
}

// Display sensor data
void displayData() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(myIpAddress);

  display.setTextSize(2);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.println(bpm);

  display.setCursor(80, 10);
  display.println("BPM");

  display.setCursor(0, 30);
  display.println(spo2);

  display.setCursor(80, 30);
  display.println("%");

  display.setCursor(0, 50);
  display.println(bodyTempC);

  display.setCursor(80, 50);
  display.println("C");

  display.display();
}

// Wifi setup
void setupWiFi() {
  Serial.println("WIFI SETUP");
  Serial.print("CONNECTED WIFI: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("CONNECTION SUCCESSFULL");
  Serial.print("IP ADDRESS: ");
  myIpAddress = WiFi.localIP();
  Serial.println(myIpAddress);
}

void setupWebServer() {
  webServer.on("/", handleRoot);
  webServer.begin();
}

void handleRoot() {
  Serial.println("REQUEST SUCCESSFULL");
  String response = "";
  response += "[";
  response += "{";

  response += "\"bodyTempC\": ";
  response += bodyTempC;
  response += ",";

  response += "\"bodyTempF\": ";
  response += bodyTempF;
  response += ",";

  response += "\"roomTempC\": ";
  response += roomTempC;
  response += ",";

  response += "\"roomTempF\": ";
  response += roomTempF;
  response += ",";

  response += "\"bpm\": ";
  response += bpm;
  response += ",";

  response += "\"spo2\": ";
  response += spo2;

  response += "}";
  response += "]";

  webServer.send(200, "text/json", response);
}

void setupSensors() {
  if (!pox.begin()) {
    Serial.println("MAX30100 - SETUP FAILED");
    for (;;);
  } else {
    Serial.println("MAX30100 - SETUP SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);

  if (!mlx.begin()) {
    Serial.println("MLX90614 - SETUP FAILED");
    for (;;);
  } else {
    Serial.println("MLX90614 - SETUP SUCCESS");
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED - SETUP FAILED"));
    for (;;);
  } else {
    Serial.println("OLED - SETUP SUCCESS");
  }
}
