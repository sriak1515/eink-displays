#ifndef BUFFERED_HTTP_CLIENT_READER_H
#define BUFFERED_HTTP_CLIENT_READER_H

#include <HTTPClient.h>

#include <Reader.h>


class BufferedHTTPClientReader : public Reader
{
private:
    HTTPClient *client;
    NetworkClient *stream;
    const char *host;
    const uint16_t port;
    const char *path;
    const uint16_t timeout;
    const uint16_t numRetries;
    const uint16_t retryDelay;
    uint8_t *buffer;
    size_t bufferSize;
    size_t bufferPos;
    size_t bufferFill;
    size_t pos;

public:
    BufferedHTTPClientReader(const char *host, uint16_t port, const char *path, size_t bufferSize, uint16_t timeout = 5000, uint16_t numRetries = 5, uint16_t retryDelay = 500);
    ~BufferedHTTPClientReader();

    size_t getPos() override;
    uint8_t read() override;
    uint16_t read16() override;
    uint32_t read32() override;
    size_t readBytes(uint8_t *buffer, size_t length) override;
    boolean seek(size_t newPos) override;
    bool isConnectedAndavailable();
    void connect();
};

#endif
