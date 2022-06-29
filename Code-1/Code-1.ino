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
float average_beat;
int average_SpO2;
float average_tempC;
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
  displayData();
  StoreDatabase();
}

void StoreDatabase() {
  if (average_beat > 60 and average_SpO2 > 70 and average_tempC > 20)
  {
    Firebase.pushFloat("Health-Corner/bpm", average_beat);
    Firebase.pushInt("Health-Corner/spo2", average_SpO2);
    Firebase.pushFloat("Health-Corner/tempC", average_tempC);
  }
}

void displayData()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);



  display.setCursor(0, 10);
  display.println("BPM: ");
  display.setCursor(40, 10);
  display.print(average_beat);

  display.setCursor(0, 20);
  display.println("SpO2:");
  display.setCursor(40, 20);
  display.print(average_SpO2);

  display.setCursor(0, 30);
  display.println("TempC:");
  display.setCursor(40, 30);
  display.print(average_tempC);
  display.display();
}


void calculate_average(float beat, int SpO2, float tempC) {
  if (readIndex == numReadings) {
    calculation_complete = true;
    calculating = false;
    initialized = false;
    readIndex = 0;
    display_values();
  }

  if (not calculation_complete and beat > 30 and beat < 220 and SpO2 > 50 and tempC > 20) {
    average_beat = filterweight * (beat) + (1 - filterweight) * average_beat;
    average_SpO2 = filterweight * (SpO2) + (1 - filterweight) * average_SpO2;
    average_tempC = filterweight * (tempC) + (1 - filterweight) * average_tempC;
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
  if ((millis() - tsLastReport > REPORTING_PERIOD_MS) and
      (not calculation_complete)) {
    calculate_average(pox.getHeartRate(), pox.getSpO2(), mlx.readObjectTempC());
    tsLastReport = millis();
  }

  if ((millis() - last_beat > 5000)) {
    calculation_complete = false;
    average_beat = 0;
    average_SpO2 = 0;
    average_tempC = 0;
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