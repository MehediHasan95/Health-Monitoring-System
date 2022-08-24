#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <MAX30100_PulseOximeter.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266mDNS.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
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
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
const int buzzer = D4;
double bpm, avgBpm;
double spo2, avgSpo2;
double bodyTempC, avgBodyTempC;
double bodyTempF, avgBodyTempF;
IPAddress myIpAddress;

void setup()
{
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("OLED Display Connection failed"));
    for (;;);
  }
  pinMode(buzzer, OUTPUT);
  setupWiFi();
  setupWebServer();
  Wire.begin();
  mlx.begin();
  pox.begin();
  setupSensors();
}

void loop()
{
  pox.update();
  MDNS.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    bpm = pox.getHeartRate();
    spo2 = pox.getSpO2();
    bodyTempC = mlx.readObjectTempC();
    bodyTempF = mlx.readObjectTempF();

    avgBpm = sensorValue(bpm);
    avgSpo2 = sensorValue(spo2);
    avgBodyTempC = sensorValue(bodyTempC);
    avgBodyTempF = sensorValue(bodyTempF);

    buzzerSensor();
    displayData();
    tsLastReport = millis();
  }
  webServer.handleClient();
}

void buzzerSensor() {
  if ((bpm > 185) or (spo2 > 100) or (bodyTempC > 50)) {
    tone(buzzer, 1000);
    delay(500);
    noTone(buzzer);
    delay(500);
    pox.begin();
  } else {
    noTone(buzzer);
  }
}

// Calculate average value
double sensorValue(double value)
{
  double arr[25];
  double sum;
  double avgrage;
  for (int i = 0; i <= 25; i++)
  {
    arr[i] = value;
  }

  for (int i = 0; i <= 25; i++)
  {
    sum = sum + arr[i];
  }
  avgrage = sum / 25;
  return avgrage;
}

// Display sensor data
void displayData()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(30, 0);
  display.print(myIpAddress );

  display.setCursor(15, 24);
  display.println(bpm + String("BPM") + String(" | ") + spo2 + String("%"));

  display.setCursor(15, 36);
  display.println(bodyTempC + String((char)247) + String("C") + String(" | ") + bodyTempF + String((char)247) + String("F"));

  display.display();
}

// Wifi setup
void setupWiFi()
{
  Serial.println("WIFI SETUP");
  Serial.print("CONNECTED WIFI: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("CONNECTION SUCCESSFULL");
  Serial.print("ID ADDRESS: ");
  myIpAddress = WiFi.localIP();
  Serial.println(myIpAddress);
  if (WiFi.status() == WL_CONNECTED)
  {
    if (MDNS.begin("health-corner")) {}
  }
}

void setupWebServer()
{
  webServer.on("/", handleRoot);
  webServer.begin();
  MDNS.addService("http", "tcp", 80);
}

void handleRoot()
{
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
  response += "\"bpm\": ";
  response += bpm;
  response += ",";
  response += "\"spo2\": ";
  response += spo2;
  response += ",";
  response += "\"avgBpm\": ";
  response += avgBpm;
  response += ",";
  response += "\"avgSpo2\": ";
  response += avgSpo2;
  response += ",";
  response += "\"avgBodyTempC\": ";
  response += avgBodyTempC;
  response += ",";
  response += "\"avgBodyTempF\": ";
  response += avgBodyTempF;
  response += "}";
  response += "]";
  webServer.send(200, "application/json", response);
}

void setupSensors()
{
  if (!pox.begin())
  {

    Serial.println("MAX30100 - SETUP FAILED");
    for (;;);
  }
  else
  {
    Serial.println("MAX30100 - SETUP SUCCESS");
    digitalWrite(1, HIGH);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
  if (!mlx.begin())
  {
    Serial.println("MLX90614 - SETUP FAILED");
    for (;;);
  }
  else
  {
    Serial.println("MLX90614 - SETUP SUCCESS");
  }
}
