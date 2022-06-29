#include <Adafruit_MLX90614.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <MAX30100_PulseOximeter.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ESP8266WebServer.h"
#define FIREBASE_HOST "my-project-2a99b-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "4kOA0npdTC5R6LreQUMjF6d4mF8dewQ55JJNLjTu"
#define WIFI_SSID "HASAN"
#define WIFI_PASSWORD "76278704H"
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define REPORTING_PERIOD_MS 500
PulseOximeter pox;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int numReadings = 10;
float filterweight = 0.5;
uint8_t max30100_address = 0x57;
uint8_t mlx90614_address = 0x5A;
uint32_t tsLastReport = 0;
uint32_t last_beat = 0;
int readIndex = 0;
float myBpm, avgBpm;
int mySpo2, avgSpO2;
float myTempC, avgTempC;
float myTempF, avgTempF;
bool calculation_complete = false;
bool calculating = false;
bool initialized = false;
byte beat = 0;

void onBeatDetected() {
  viewBeat();
  last_beat = millis();
}

void viewBeat() {
  if (beat == 0) {
    beat = 1;
  } else {
    beat = 0;
  }
}

void initial_display() {
  if (not initialized) {
    viewBeat();
    Serial.println("PLEASE PLACE YOUR FINGER ON THE SENSOR");
    initialized = true;
  }
}

void display_calculating(int j) {
  viewBeat();
  Serial.println("Measuring");
  for (int i = 0; i <= j; i++) {
    Serial.print("");
  }
}

void display_values() {
  Serial.print("Heart-rate: ");
  Serial.print(avgBpm);
  Serial.print(" BPM || SpO2: ");
  Serial.print(avgSpO2);
  Serial.print("% ");
  Serial.print("Body-tempC: ");
  Serial.print(avgTempC);
  Serial.print(" Body-tempF: ");
  Serial.print(avgTempF);
  Serial.println();
  StoreDatabase();
}

void StoreDatabase() {
  if (avgBpm > 0) {
    Firebase.pushFloat("Health-Corner/heart-rate", avgBpm);
    Firebase.pushInt("Health-Corner/oxygen-level", avgSpO2);
    Firebase.pushFloat("Health-Corner/body-tempC", avgTempC);
    Firebase.pushFloat("Health-Corner/body-tempF", avgTempF);
  }
  delay(1000);
  ESP.restart();
}

void displayData()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.println("BPM: ");
  display.setCursor(40, 10);
  display.print(myBpm);

  display.setCursor(0, 20);
  display.println("SpO2:");
  display.setCursor(40, 20);
  display.print(mySpo2);

  display.setCursor(0, 30);
  display.println("TempC:");
  display.setCursor(40, 30);
  display.print(myTempC);

  display.setCursor(0, 40);
  display.println("TempF:");
  display.setCursor(40, 40);
  display.print(myTempF);
  display.display();
}


void calculate_average(float bpm, int spo2, float tempC, float tempF) {
  if (readIndex == numReadings) {
    calculation_complete = true;
    calculating = false;
    initialized = false;
    readIndex = 0;
    display_values();
  }

  if (not calculation_complete and bpm > 30 and bpm < 220 and spo2 > 50 and tempC > 20 and tempF > 80) {
    avgBpm = filterweight * (bpm) + (1 - filterweight) * avgBpm;
    avgSpO2 = filterweight * (spo2) + (1 - filterweight) * avgSpO2;
    avgTempC = filterweight * (tempC) + (1 - filterweight) * avgTempC;
    avgTempF = filterweight * (tempF) + (1 - filterweight) * avgTempF;
    readIndex++;
    display_calculating(readIndex);
  }
}

void firebaseInitialize() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("CONNECTION SUCCESSFULL: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void setup() {
  Serial.begin(9600);
  firebaseInitialize();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("OLED Display Connection failed"));
    for (;;);
  }
  pox.begin();
  mlx.begin();
  setupSensors();
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {
  pox.update();

  myBpm = pox.getHeartRate();
  mySpo2 = pox.getSpO2();
  myTempC = mlx.readObjectTempC();
  myTempF = mlx.readObjectTempF();

  if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and
      (not calculation_complete)) {
    calculate_average(myBpm, mySpo2, myTempC, myTempF);
    displayData();
    tsLastReport = millis();
  }

  if ((millis() - last_beat > 5000)) {
    calculation_complete = false;
    avgBpm = 0;
    avgSpO2 = 0;
    avgTempC = 0;
    avgTempF = 0;
    initial_display();
  }
}

void setupSensors() {
  if (!pox.begin()) {
    Serial.println("Pulse Oximeter - SETUP FAILED");
    for (;;)
      ;
  } else {
    Serial.println("Pulse Oximeter - SETUP SUCCESS");
    digitalWrite(1, HIGH);
  }

  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
  if (!mlx.begin()) {
    Serial.println("MLX Infrared Thermometer - SETUP FAILED");
    for (;;)
      ;
  } else {
    Serial.println("MLX Infrared Thermometer - SETUP SUCCESS");
  }
}
