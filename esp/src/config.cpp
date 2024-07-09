#include <Arduino.h>
#include <ArduinoJson.h>

#include <config.h>
#include <filesystem.h>

bool loadConfig(const char* filename, FileSystem& fs, Config& config) {
    File configFile = fs.openFile(filename, "r");
    Serial.println("File opened.");
    if(!configFile){
        Serial.print("Failed to open config file ");
        Serial.println(filename);
        return false;
    }

    JsonDocument doc;
    auto error = deserializeJson(doc, configFile);
    Serial.println("File deserialized to json.");
    if(error){
        Serial.print("Failed to parse config file ");
        Serial.println(filename);
        configFile.close();
        return false;
    }

    config.wifi_ssid = doc["wifi_ssid"].as<String>();
    config.wifi_password = doc["wifi_password"].as<String>();

    Serial.println("Wrote config to struct.");

    configFile.close();
    Serial.println("Closed configfile.");
    return true;
}
