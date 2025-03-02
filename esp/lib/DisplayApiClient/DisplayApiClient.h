#ifndef DISPLAY_CONFIG_MANAGER_H
#define DISPLAY_CONFIG_MANAGER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "DisplayInfo.h"
#include "UpdateInfo.h"

class DisplayApiClient {
public:
    DisplayApiClient(const char* baseUrl);
    ~DisplayApiClient();

    bool getDisplayInfo(int displayId, DisplayInfo& config);
    bool createDisplayUpdate(int displayId, const UpdateInfo& update);

private:
    const char* _baseUrl;
    HTTPClient _httpClient;

    bool sendRequest(const char* path, const char* method, const JsonDocument* payloadJson, JsonDocument& responseJson);
};

#endif // DISPLAY_CONFIG_MANAGER_H
