#include "DisplayInfo.h"

DisplayInfo::DisplayInfo() {}

DisplayInfo::DisplayInfo(const std::string& name, const std::string& url, const std::string& previousUrl, int refreshFrequency, int fullRefreshFrequency, int id)
    : name(name), url(url), previousUrl(previousUrl), refreshFrequency(refreshFrequency), fullRefreshFrequency(fullRefreshFrequency), id(id) {}

bool DisplayInfo::serialize(JsonDocument& doc) const {
    doc["name"] = name;
    doc["url"] = url;
    if (!previousUrl.empty()) {
        doc["previous_url"] = previousUrl;
    } else {
        doc["previous_url"] = nullptr;
    }
    doc["refresh_frequency"] = refreshFrequency;
    if (fullRefreshFrequency != -1) {
        doc["full_refresh_frequency"] = refreshFrequency;
    } else {
        doc["full_refresh_frequency"] = nullptr;
    }
    doc["id"] = id;
    return true;
}

bool DisplayInfo::deserialize(const JsonDocument& doc) {
    name = doc["name"] | "";
    url = doc["url"] | "";
    previousUrl = doc["previous_url"] | "";
    refreshFrequency = doc["refresh_frequency"] | 0;
    fullRefreshFrequency = doc["full_refresh_frequency"] | -1;

    id = doc["id"] | 0;
    return true;
}
