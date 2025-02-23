#ifndef __WIFI_MANAGER_H__
#define __WIFI_MANAGER_H__


#define BUFFER_SIZE 4096

void connectToWiFi(const char* ssid, const char* password, int connectTimeout = 60);
void disconnect();

#endif // __WIFI_H__
