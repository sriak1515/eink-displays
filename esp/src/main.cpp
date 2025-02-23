#include <Arduino.h>
#include <LittleFS.h>
#include <stdexcept>

#include <WifiManager.h>
#include <timeUtils.h>
#include <Display.h>
#include <TimetableRenderer.h>
#include <Renderer.h>
#include <BitmapDrawer.h>
#include <BufferedHTTPClientReader.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#ifndef STARTUP_DELAY
#define STARTUP_DELAY 3
#endif

void beginDeepSleep(tm *timeInfo)
{
  getCurrentTime(timeInfo);
  uint64_t sleepDuration = 60 - timeInfo->tm_sec - STARTUP_DELAY; // wake up STARTUP_DELAY second before the end of the minute
  esp_sleep_enable_timer_wakeup(sleepDuration * 1000000);         // convert seconds to microseconds
  Serial.print("Entering deep sleep for ");
  Serial.print(sleepDuration);
  Serial.println("s");
  Serial.end();
  esp_deep_sleep_start();
}

void drawPrevious(Display *display)
{
  BufferedHTTPClientReader readerPrevious("192.168.0.10", 4000, "/previous_timetable?width=460&height=780&rotation=90", 2048, 10 * 1000);
  BitmapDrawer drawerPrevious(readerPrevious, *display);
  drawerPrevious.drawBitmap(10, 10);
}

void drawCurrent(Display *display)
{
  BufferedHTTPClientReader reader("192.168.0.10", 4000, "/timetable?width=460&height=780&n=5&font_header_size=44&font_entries_size=36&rotation=90", 2048, 10 * 1000);
  BitmapDrawer drawer(reader, *display);
  drawer.drawBitmap(10, 10);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  delay(1000);
  Serial.println("Starting app");

  connectToWiFi(WIFI_SSID, WIFI_PASSWORD);

  tm timeInfo = {};
  syncSNTP(&timeInfo);
  if (timeInfo.tm_sec >= 60 - STARTUP_DELAY)
  {
    int waitTime = 60 - timeInfo.tm_sec;
    Serial.print("Sleeping for ");
    Serial.print(waitTime);
    Serial.println("s for next minute");
    delay(waitTime * 1000);
    getCurrentTime(&timeInfo);
  }
  Serial.println("Initializing display.");
  Display *display = new Display();
  display->initDisplay();
  // display->clear();
  display->display.setRotation(0);
  Serial.println("Display infos");
  Serial.print("Display height: ");
  Serial.println(display->display.height());
  Serial.print("Display width: ");
  Serial.println(display->display.width());
  Serial.print("Display page height: ");
  Serial.println(display->display.pageHeight());
  Serial.println("Displaying image.");

  uint32_t startTime = millis();

#ifdef ENABLE_FAST_PARTIAL_MODE
  display->display.epd2.enableFastPartialMode();
#endif

  display->display.setPartialWindow(0, 0, 800, 480);
  try
  {
    Serial.println("Drawing previous image");
    drawPrevious(display);
    Serial.println("Drawing current image");
    drawCurrent(display);
    Serial.print("Image displayed in ");
    Serial.print(millis() - startTime);
    Serial.println(" ms");
  }
  catch(const std::runtime_error& re)
  {
    Serial.print("Runtime error occured: ");
    Serial.println(re.what());
  }
  catch(const std::exception& ex) {
    Serial.print("Exception occured: ");
    Serial.println(ex.what());
  }
  catch (...)
  {
    Serial.println("Unknown error");
  }

#ifdef ENABLE_FAST_PARTIAL_MODE
  display->display.epd2.disableFastPartialMode();
#endif

  disconnect();
  display->display.hibernate();
  beginDeepSleep(&timeInfo);
}

void loop()
{
}
