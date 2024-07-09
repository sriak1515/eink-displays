#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>

#include <filesystem.h>

struct Config
{
    String wifi_ssid;
    String wifi_password;
};

bool loadConfig(const char* filename, FileSystem& fs, Config& config);

#endif
