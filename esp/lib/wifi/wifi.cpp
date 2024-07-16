#include <FS.h>
#include <WiFi.h>
#include <wifi.h>

void connectToWiFi(const char* ssid, const char* password, int connectTimeout)
{
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA); // switch off AP
    WiFi.setAutoReconnect(true);
    WiFi.disconnect();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        Serial.print(WiFi.status());
        if (--connectTimeout <= 0)
        {
            Serial.println();
            Serial.println("WiFi connect timeout");
            return;
        }
    }

    Serial.println("Connected to the WiFi network");

    Serial.println(WiFi.localIP());
}

void disconnect()
{
    WiFi.disconnect();
}
