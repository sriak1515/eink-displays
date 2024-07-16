#include <Arduino.h>
#include <LittleFS.h>

#include <wifi.h>
#include <Display.h>
#include <BitmapDrawer.h>
#include <BufferedWifiClientReader.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting app");

  connectToWiFi(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Initializing display.");
  Display *display = new Display();
  display->initDisplay();

  Serial.println("Displaying image.");

  BufferedWifiClientReader reader("192.168.0.10", 3001, "/matteo3.bmp", 2048);
  BitmapDrawer drawer(reader, *display);
  drawer.drawBitmap();
}

void loop()
{
}
