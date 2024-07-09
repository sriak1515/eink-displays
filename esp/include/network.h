#ifndef __NETWORK_H__
#define __NETWORK_H__


#include <config.h>
#include <filesystem.h>

#define BUFFER_SIZE 4096

void connectToWiFi(Config &config, int connectTimeout = 60);
void disconnect();
bool downloadFile(const char *host, uint16_t port, const char *path, const char *filename, FileSystem &fs);

#endif // __NETWORK_H__
