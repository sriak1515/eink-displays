#ifndef DISPLAY_INFO_H
#define DISPLAY_INFO_H

#include <ArduinoJson.h>
#include <string>

class DisplayInfo {
public:
    DisplayInfo();
    DisplayInfo(const std::string& name, const std::string& url, const std::string& previousUrl, int refreshFrequency,  int fullRefreshFrequency, int id);

    std::string name;
    std::string url;
    std::string previousUrl;
    int refreshFrequency;
    int fullRefreshFrequency;
    int id;

    bool serialize(JsonDocument& doc) const;
    bool deserialize(const JsonDocument& doc);
};

#endif // DISPLAY_INFO_H
