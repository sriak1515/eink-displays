#include <Arduino.h>
#include <LittleFS.h>
#include <stdexcept>
#include <Preferences.h>

#include <WifiManager.h>
#include <timeUtils.h>
#include <Display.h>
#include <TimetableRenderer.h>
#include <Renderer.h>
#include <BitmapDrawer.h>
#include <BufferedHTTPClientReader.h>
#include <DisplayApiClient.h>
#include <DisplayInfo.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#ifndef STARTUP_DELAY
#define STARTUP_DELAY 3
#endif

#define STORAGE_NAMESPACE "display"
#define COUNTER_KEY "runs"

Preferences preferences;

DisplayApiClient displayApiClient(API_BASE_URL);

void sendUpdate(const std::string& message, UpdateStatus status) {
    UpdateInfo updateInfo(message, status);
    if (displayApiClient.createDisplayUpdate(DISPLAY_ID, updateInfo)) {
        Serial.println("Update created successfully");
    } else {
        Serial.println("Failed to create update");
    }
}

void initializeDisplay(Display* display) {
    Serial.println("Initializing display.");
    display->initDisplay();
    display->display.setRotation(0);
    Serial.println("Display infos");
    Serial.print("Display height: ");
    Serial.println(display->display.height());
    Serial.print("Display width: ");
    Serial.println(display->display.width());
    Serial.print("Display page height: ");
    Serial.println(display->display.pageHeight());
}

void drawImages(Display* display, const DisplayInfo& displayInfo) {
    uint32_t startTime = millis();

#ifdef ENABLE_FAST_PARTIAL_MODE
    display->display.epd2.enableFastPartialMode();
#endif

    display->display.setPartialWindow(0, 0, 800, 480);
    try
    {
        if (!displayInfo.previousUrl.empty()) {
            Serial.println("Drawing previous image");
            BufferedHTTPClientReader readerPrevious(displayInfo.previousUrl.c_str(), 2048, 10 * 1000);
            BitmapDrawer drawerPrevious(readerPrevious, *display);
            drawerPrevious.drawBitmap(10, 10);
        }

        Serial.println("Drawing current image");
        BufferedHTTPClientReader reader(displayInfo.url.c_str(), 2048, 10 * 1000);
        BitmapDrawer drawer(reader, *display);
        drawer.drawBitmap(10, 10);

        Serial.print("Image displayed in ");
        Serial.print(millis() - startTime);
        Serial.println(" ms");

        sendUpdate("Images displayed successfully", UpdateStatus::PASS);
    }
    catch (const std::runtime_error &re)
    {
        Serial.print("Runtime error occurred: ");
        Serial.println(re.what());

        sendUpdate("Runtime error occurred", UpdateStatus::ERROR);
    }
    catch (const std::exception &ex)
    {
        Serial.print("Exception occurred: ");
        Serial.println(ex.what());

        sendUpdate("Exception occurred", UpdateStatus::ERROR);
    }
    catch (...)
    {
        Serial.println("Unknown error");

        sendUpdate("Unknown error", UpdateStatus::ERROR);
    }

#ifdef ENABLE_FAST_PARTIAL_MODE
    display->display.epd2.disableFastPartialMode();
#endif
}

uint64_t calculateSleepDuration(tm *timeInfo, int refreshFrequency) {
    return refreshFrequency - (timeInfo->tm_sec + STARTUP_DELAY);
}

void enterDeepSleep(tm *timeInfo, int refreshFrequency) {
    uint64_t sleepDuration = calculateSleepDuration(timeInfo, refreshFrequency);
    esp_sleep_enable_timer_wakeup(sleepDuration * 1000000); // convert seconds to microseconds
    Serial.print("Entering deep sleep for ");
    Serial.print(sleepDuration);
    Serial.println("s");
    Serial.end();
    esp_deep_sleep_start();
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    Serial.println();
    delay(100);
    preferences.begin(STORAGE_NAMESPACE, false);
    unsigned int numRuns = preferences.getUInt(COUNTER_KEY, 0);
    Serial.print("Current number of runs is ");
    Serial.println(numRuns);

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

    DisplayInfo displayInfo;
    Display *display;
    if (displayApiClient.getDisplayInfo(1, displayInfo))
    {
        Serial.println("Display Config:");
        Serial.print("Name: ");
        Serial.println(displayInfo.name.c_str());
        Serial.print("URL: ");
        Serial.println(displayInfo.url.c_str());
        Serial.print("Previous URL: ");
        Serial.println(displayInfo.previousUrl.empty() ? "null" : displayInfo.previousUrl.c_str());
        Serial.print("Refresh Frequency: ");
        Serial.println(displayInfo.refreshFrequency);
        Serial.print("Full Refresh Frequency: ");
        Serial.println(displayInfo.fullRefreshFrequency);
        Serial.print("ID: ");
        Serial.println(displayInfo.id);

        sendUpdate("Initializing display", UpdateStatus::PASS);

        display = new Display();

        initializeDisplay(display);
        if (displayInfo.fullRefreshFrequency != -1 && numRuns % displayInfo.fullRefreshFrequency == 0) {
          Serial.println("Clearing display due to full refresh frequency.");
          display->clear();
          Serial.println("Display cleared.");
        }

        Serial.println("Displaying image.");
        drawImages(display, displayInfo);

        numRuns++;
        if (numRuns >= 10000) {
          numRuns = 0;
        }
        preferences.putUInt(COUNTER_KEY, numRuns);

        preferences.end();
        disconnect();
        display->display.hibernate();

        enterDeepSleep(&timeInfo, displayInfo.refreshFrequency);
    } else {
        Serial.println("Could not get display config.");

        sendUpdate("Could not get display config", UpdateStatus::ERROR);

        preferences.end();
        disconnect();
        if(display)
          display->display.hibernate();

        // Enter deep sleep for 5 minutes if config could not be fetched
        esp_sleep_enable_timer_wakeup(300 * 1000000); // 5 minutes in microseconds
        Serial.println("Entering deep sleep for 300s due to config failure");
        Serial.end();
        esp_deep_sleep_start();
    }
}

void loop()
{
}
