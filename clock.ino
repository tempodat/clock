#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <Time.h>

#include "secrets.h"

/* =====CONSTANTS===== */

// This only works if this is high on powerup
const int enablePin = 13; 

// Got from I2C scanner, google it
const byte oledAddress = 0x3C;

// Replace with your network name&pass
const char* ssid = NETWORK_SSID;
const char* password = NETWORK_PASS;

// Timezone & Daylight Savings correction
const long utcOffsetInSeconds = 7200;
const int daylightOffsetInSeconds = 3600;

// period in ms to fetch the local time (not ntp!) and update the screen
const long pollingInterval = 100;

/* DON'T CHANGE BEYOND THIS POINT */

Adafruit_SSD1306 display(128,32);

// this is set at the end of setup(), to make sure it ran smoothly before looping
bool enabled = false;

// used for the non intrusive delay system
unsigned long previousMillis = 0;

time_t rawtime;

void setup() {
  /* Verify enable switch */
  pinMode(enablePin, INPUT);
  if (!digitalRead(enablePin)) return;
  
  /* Serial */
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Serial good");

  /* I2C */
  Serial.print("Setting up OLED Display...");
  Wire.begin();
  // Verify good connection to OLED
  Wire.beginTransmission(oledAddress);
  if (Wire.endTransmission() > 0) {
    Serial.println("Failed");
    Serial.println("Can't connect to OLED on channel 0x3C, Make sure SCL -> D1, SDA -> D2");
    // for (;;) {}
    return;
  }
  Serial.println("Done!");

  /* Get current time from network */
  // Connect to WIFI
  Serial.print("Connecting to wifi");
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay (500);
    Serial.print ( "." );
  }
  Serial.println("Done!");

  // Synchronize with NTP
  configTime(utcOffsetInSeconds, daylightOffsetInSeconds, "pool.ntp.org");
  Serial.println("Synchronized time with NTP server.");

  /* Display setup */
  Serial.print("Initializing display...");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.display();
  delay(1);
  Serial.println("Done!");

  Serial.println("Setup complete.");

  enabled = true;
}

void loop() {
  // Make sure this doesn't run if setup fails
  if (!enabled) return;

  // Poll only every interval (100ms).
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis < pollingInterval) return;

  // Fetch current time
  time(&rawtime);
  struct tm* timeinfo = localtime(&rawtime);

  // Format time
  char buf[16];
  strftime(buf, 16, "%H:%M:%S", timeinfo);

  
  
  // Display time
  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.write(buf);
  display.display();
  delay(1);

  // Setup the next poll in pollingInterval ms
  previousMillis = currentMillis;
}
