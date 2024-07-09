#include <FS.h>
#include <WiFi.h>
#include <WiFiClient.h>

#include <config.h>
#include <network.h>
#include <filesystem.h>

void connectToWiFi(Config &config, int connectTimeout)
{
    WiFi.persistent(true);
    WiFi.mode(WIFI_STA); // switch off AP
    WiFi.setAutoReconnect(true);
    WiFi.disconnect();
    Serial.print("Connecting to ");
    Serial.println(config.wifi_ssid);
    WiFi.begin(config.wifi_ssid, config.wifi_password);

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

bool downloadFile(const char *host, uint16_t port, const char *path, const char *filename, FileSystem &fs)
{
    WiFiClient client;

    if (!client.connect(host, port))
    {
        Serial.println("Connection failed");
        return false;
    }

    // Send HTTP request
    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    bool ok = false;
    while (client.connected() || client.available())
    {
        String line = client.readStringUntil('\n');
        if (!ok)
        {
            ok = line.startsWith("HTTP/1.1 200 OK");
        }

        if (!ok)
            Serial.println(line);
        if (line == "\r")
        {
            break;
        }
    }
    if (!ok)
        return false;

    uint8_t buffer[BUFFER_SIZE]; // Increase buffer size to 1024 bytes
    size_t total = 0;
    unsigned long startTime = millis(); // Start time for speed calculation
    unsigned long lastPrintTime = 0;    // Time of last print

    // Open file for writing
    File file = fs.openFile(filename, "w");
    if (!file)
    {
        Serial.println("Failed to open file for writing");
        return false;
    }

    while (client.connected() || client.available())
    {
        size_t available = client.available();
        size_t fetch = available <= sizeof(buffer) ? available : sizeof(buffer);
        if (fetch > 0)
        {
            size_t got = client.readBytes(buffer, fetch);
            file.write(buffer, got);
            total += got;

            // Calculate and print progress and speed every second
            unsigned long currentTime = millis();
            if (currentTime - lastPrintTime >= 1000)
            {
                Serial.print("Got: ");
                Serial.println(got);
                float timeDiff = (currentTime - startTime) / 1000.0; // Time difference in seconds
                float speed = total / timeDiff / 1024.0;             // Speed in kilobytes per second
                Serial.print("Progress: ");
                Serial.print(total / 1024.0, 2);
                Serial.print(" KB, Speed: ");
                Serial.print(speed, 2);
                Serial.println(" KB/s");
                lastPrintTime = currentTime;
            }
        }
        // delay(10);
    }
    file.close();
    unsigned long currentTime = millis();
    Serial.print("done, ");
    Serial.print(total / 1024.0, 2);
    Serial.print(" KB transferred in ");
    Serial.print((currentTime - startTime) / 1000.0);
    Serial.println(" seconds.");
    return true;
}
