 /* 
  Tutorial on how to get started with OLED screen, and how to wire.
  https://randomnerdtutorials.com/esp8266-0-96-inch-oled-display-with-arduino-ide/
 */

/* ESP8266 HTTP Requests */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

/* OLED SCREEN */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {

  Serial.begin(9600);
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("ssid", "password");

  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void loop() {

  StaticJsonDocument<1000> doc;
  String json = "";
  
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://info-backend1.herokuapp.com/weather")) {  // HTTP request


      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP  header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          json = payload;
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  DeserializationError error = deserializeJson(doc, json);

  // Test if parsing succeeds.
  if (error) {
    Serial.println("parseObject() failed");
    Serial.print(error.c_str());
  }

  // Fetch values from JSON response

  String celsius = doc["main"]["temp"];
  String weather = doc["weather"][0]["main"];
  float cDegrees = celsius.toFloat() - 273;
  cDegrees = round(cDegrees);
  cDegrees = (int) cDegrees;

  printData(cDegrees, weather);
  delay(1750000); // Approximately every 30th minute. TODO: need to fix it so that it runs specific times.
}

void printData(int celsius, String weather) {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(24, 20);
  
  // Display static text
  display.println(String(celsius) + (char)247 + "C"); // (char)247 equals the degree sign
  display.setTextSize(2);
  display.println("  "+ weather);
  display.display(); 
} 
