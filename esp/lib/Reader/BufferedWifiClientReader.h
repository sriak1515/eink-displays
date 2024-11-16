#ifndef BUFFERED_WIFI_CLIENT_H
#define BUFFERED_WIFI_CLIENT_H

#include <WiFiClient.h>

#include <Reader.h>

class BufferedWifiClientReader: public Reader
{
private:
    WiFiClient *client;
    const char *host;
    const uint16_t port;
    const char *path;
    const uint16_t timeout;
    uint8_t *buffer;
    size_t bufferSize;
    size_t bufferPos;
    size_t bufferFill;
    size_t pos;

    bool connect();
    bool reset();

public:
    BufferedWifiClientReader(const char *host, uint16_t port, const char *path, size_t bufferSize, uint16_t timeout = 5000);
    ~BufferedWifiClientReader();

    size_t getPos() override;
    uint8_t read() override;
    uint16_t read16() override;
    uint32_t read32() override;
    size_t readBytes(uint8_t *buffer, size_t length) override;
    boolean seek(size_t newPos) override;
    bool isConnectedAndavailable();
    boolean reconnect(int maxRetries = 3);
};

#endif // BUFFERED_WIFI_CLIENT_H
