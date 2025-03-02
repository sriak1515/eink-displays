#include "DisplayApiClient.h"

DisplayApiClient::DisplayApiClient(const char *baseUrl)
    : _baseUrl(baseUrl)
{
}

DisplayApiClient::~DisplayApiClient()
{
    _httpClient.end();
}

bool DisplayApiClient::getDisplayInfo(int displayId, DisplayInfo &config)
{
    char path[64];
    snprintf(path, sizeof(path), "/eink/display/%d", displayId);
    JsonDocument responseJson;
    if (sendRequest(path, "GET", nullptr, responseJson))
    {
        return config.deserialize(responseJson);
    }
    return false;
}

bool DisplayApiClient::createDisplayUpdate(int displayId, const UpdateInfo &update)
{
    char path[64];
    snprintf(path, sizeof(path), "/eink/display/%d/update", displayId);
    JsonDocument payloadJson;
    update.serialize(payloadJson);
    JsonDocument responseJson;
    return sendRequest(path, "POST", &payloadJson, responseJson);
}

bool DisplayApiClient::sendRequest(const char *path, const char *method, const JsonDocument *payloadJson, JsonDocument &responseJson)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("WiFi is not connected.");
        return false;
    }

    String url = String(_baseUrl) + path;

    _httpClient.begin(url);
    _httpClient.addHeader("Content-Type", "application/json");

    int httpCode = -1;

    if (strcmp(method, "GET") == 0)
    {
        httpCode = _httpClient.GET();
    }
    else if (strcmp(method, "POST") == 0)
    {
        String payload;
        serializeJson(*payloadJson, payload);
        httpCode = _httpClient.POST(payload);
    }

    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED)
        {
            String response = _httpClient.getString();
            DeserializationError error = deserializeJson(responseJson, response);
            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.c_str());
                return false;
            }
            deserializeJson(responseJson, response);
            return true;
        }
        else
        {
            Serial.print("HTTP error: ");
            Serial.println(httpCode);
        }
    }
    else
    {
        Serial.print("Error on HTTP request: ");
        Serial.println(_httpClient.errorToString(httpCode).c_str());
    }

    _httpClient.end();
    return false;
}
