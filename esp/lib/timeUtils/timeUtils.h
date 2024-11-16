#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <Arduino.h>

extern const char *TIMEZONE;
extern const char *NTP_SERVER_1;
extern const char *NTP_SERVER_2;
extern const char *NTP_SERVER_2;
extern const unsigned long NTP_TIMEOUT;

bool syncSNTP(tm *timeInfo);
bool getCurrentTime(tm *timeInfo);

#endif