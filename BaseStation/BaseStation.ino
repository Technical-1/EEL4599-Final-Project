#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// The following file is ignored by git, so that
// credentials aren't stored in the repository.
// Its structure is as follows:
/*
 * const char* ssid = "...";
 * const char* pwd = "...";
 */
#include "WiFiCredentials.h"

// The HTTP endpoint for submitting data, IP address for willmccoy.xyz

const char* endpoint = "http://155.138.160.18:8888/


Adafruit_SSD1306 display(128, 32, &Wire, -1);


bool initDisplay() {
  int displayInit = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if(!displayInit) {
    Serial.println("Could not initialize display\n");
    return false;
  }

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  
  return true;
}

void logToDisplay(const char* line) {
  display.write(line);
  display.display();
}

bool initWiFi() {
  WiFi.begin(ssid, pwd);
  logToDisplay("Initializing Wi-Fi\n");

  // Wait until have successful connection
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting");
    delay(500);
  }
  logToDisplay("Connected to Wi-Fi\n");

  logToDisplay(WiFi.localIP().toString().c_str());

  return true;
}

void setup() {
  Serial.begin(115200);

  initDisplay();

  initWiFi();
}

void loop() {
    Serial.println("Done");
    delay(1000);
}
