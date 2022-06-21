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

    displayData();
    tsLastReport = millis();
  }
  webServer.handleClient();
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

  display.setCursor(0, 0);
  display.println("IP:");
  display.setCursor(30, 0);
  display.print(myIpAddress);

  display.setCursor(0, 10);
  display.println("BPM: ");
  display.setCursor(40, 10);
  display.print(bpm);

  display.setCursor(0, 20);
  display.println("SpO2:");
  display.setCursor(40, 20);
  display.print(spo2);

  display.setCursor(0, 30);
  display.println("TempC:");
  display.setCursor(40, 30);
  display.print(bodyTempC);

  display.setCursor(0, 40);
  display.println("TempF:");
  display.setCursor(40, 40);
  display.print(bodyTempF);
  display.display();
}

// Wifi setup
void setupWiFi()
{
  Serial.println("WIFI SETUP");
  Serial.print("CONNECTED WITH: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(200);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("CONNECTED SUCCESSFULLY");
  Serial.print("YOUR ID ADDRESS: ");
  myIpAddress = WiFi.localIP();
  Serial.println(myIpAddress);
}

void setupWebServer()
{
  Serial.println("SETUP");
  webServer.on("/", handleRoot);
  Serial.println("STARTING...");
  webServer.begin();
  Serial.println("RUNNING");
}

void handleRoot()
{
  Serial.println("REQUEST SUCCESSFUL");
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
  Serial.print("Initializing Pulse Oximeter..");
  if (!pox.begin())
  {
    Serial.println("Pulse Oximeter - SETUP FAILED");
    for (;;);
  }
  else
  {
    Serial.println("Pulse Oximeter - SETUP SUCCESS");
    digitalWrite(1, HIGH);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
  Serial.println("Initializing MLX Infrared Thermometer..");
  if (!mlx.begin())
  {
    Serial.println("MLX Infrared Thermometer - SETUP FAILED");
    for (;;);
  }
  else
  {
    Serial.println("MLX Infrared Thermometer - SETUP SUCCESS");
  }
}
