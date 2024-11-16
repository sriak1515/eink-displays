#include <timeUtils.h>
#include <esp_sntp.h>

const char *TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3"; // Europe/Zurich
const char *NTP_SERVER_1 = "pool.ntp.org";
const char *NTP_SERVER_2 = "time.nist.gov";
const unsigned long NTP_TIMEOUT = 20000; // ms

bool syncSNTP(tm *timeInfo)
{
    configTzTime(TIMEZONE, NTP_SERVER_1, NTP_SERVER_2);
    // Wait for SNTP synchronization to complete
    unsigned long timeout = millis() + NTP_TIMEOUT;
    if ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) && (millis() < timeout))
    {
        Serial.print("Wait for SNTP sync ");
        delay(100); // ms
        while ((sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) && (millis() < timeout))
        {
            Serial.print(".");
            delay(100); // ms
        }
        Serial.println();
    }
    return getCurrentTime(timeInfo);
}

bool getCurrentTime(tm *timeInfo)
{
    int attempts = 0;
    while (!getLocalTime(timeInfo) && attempts++ < 3)
    {
        Serial.println("Failed to get time.");
        return false;
    }
    Serial.println(timeInfo, "%A, %B %d, %Y %H:%M:%S");
    return true;
}