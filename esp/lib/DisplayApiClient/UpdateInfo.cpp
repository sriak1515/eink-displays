#include "UpdateInfo.h"

UpdateInfo::UpdateInfo() {}

UpdateInfo::UpdateInfo(const std::string& message, UpdateStatus status)
    : message(message), status(status) {}

bool UpdateInfo::serialize(JsonDocument& doc) const {
    doc["message"] = message;
    switch (status) {
        case UpdateStatus::PASS:
            doc["status"] = "pass";
            break;
        case UpdateStatus::WARN:
            doc["status"] = "warn";
            break;
        case UpdateStatus::ERROR:
            doc["status"] = "error";
            break;
    }
    return true;
}

bool UpdateInfo::deserialize(const JsonDocument& doc) {
    message = doc["message"] | "";
    const char* statusStr = doc["status"] | "";
    if (strcmp(statusStr, "pass") == 0) {
        status = UpdateStatus::PASS;
    } else if (strcmp(statusStr, "warn") == 0) {
        status = UpdateStatus::WARN;
    } else if (strcmp(statusStr, "error") == 0) {
        status = UpdateStatus::ERROR;
    } else {
        return false; // Invalid status
    }
    return true;
}
