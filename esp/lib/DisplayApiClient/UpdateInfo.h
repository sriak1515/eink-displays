#ifndef UPDATE_INFO_H
#define UPDATE_INFO_H

#include <ArduinoJson.h>
#include <string>

enum class UpdateStatus {
    PASS,
    WARN,
    ERROR
};

class UpdateInfo {
public:
    UpdateInfo();
    UpdateInfo(const std::string& message, UpdateStatus status);

    std::string message;
    UpdateStatus status;

    bool serialize(JsonDocument& doc) const;
    bool deserialize(const JsonDocument& doc);
};

#endif // UPDATE_INFO_H
