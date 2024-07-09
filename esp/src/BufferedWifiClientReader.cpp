#include <BufferedWifiClientReader.h>

bool BufferedWifiClientReader::connect()
{
    if (!client->connect(host, port))
    {
        Serial.println("Connection failed");
        return false;
    }

    // Send HTTP request
    client->print(String("GET ") + path + " HTTP/1.1\r\n" +
                  "Host: " + host + "\r\n" +
                  "Connection: close\r\n\r\n");

    bool ok = false;
    int contentLength = -1;
    while (client->connected() || client->available())
    {
        String line = client->readStringUntil('\n');
        if (!ok)
        {
            ok = line.startsWith("HTTP/1.1 200 OK");
        }

        if (line.startsWith("Content-Length: "))
        {
            contentLength = line.substring(16).toInt();
        }

        if (!ok)
            Serial.println(line);
        if (line == "\r")
        {
            break;
        }
    }

    if (ok && contentLength >= 0)
    {
        Serial.print("Content-Length: ");
        Serial.println(contentLength);
    }

    return ok;
}

bool BufferedWifiClientReader::reset()
{
    client->stop();
    if (!connect())
        return false;
    pos = 0;
    bufferPos = 0;
    bufferFill = 0;
    return true;
}

BufferedWifiClientReader::BufferedWifiClientReader(const char *host, uint16_t port, const char *path, size_t bufferSize) : host(host), port(port), path(path), bufferSize(bufferSize), bufferPos(0), bufferFill(0), pos(0)
{
    buffer = new uint8_t[bufferSize];
    client = new WiFiClient();
    reset();
}

BufferedWifiClientReader::~BufferedWifiClientReader()
{
    delete[] buffer;
}

size_t BufferedWifiClientReader::getPos()
{
    return pos;
}

uint8_t BufferedWifiClientReader::read()
{
    if (bufferPos == bufferFill)
    {
        size_t available = client->available();
        size_t fetch = available <= bufferSize ? available : bufferSize;
        bufferPos = 0;
        bufferFill = client->read(buffer, fetch);
        if (bufferFill == 0)
        {
            Serial.println("Tried to fetch more bytes than available");
            throw std::runtime_error("Tried to fetch more bytes than available");
        }
    }
    uint8_t data = buffer[bufferPos];
    ++pos;
    ++bufferPos;
    return data;
}

uint16_t BufferedWifiClientReader::read16()
{
    uint16_t data;
    data = (uint16_t)read();
    data |= (uint16_t)read() << 8;
    return data;
}

uint32_t BufferedWifiClientReader::read32()
{
    uint32_t data;
    data = (uint32_t)read();
    data |= (uint32_t)read() << 8;
    data |= (uint32_t)read() << 16;
    data |= (uint32_t)read() << 24;
    return data;
}

size_t BufferedWifiClientReader::readBytes(uint8_t *buffer, size_t length)
{
    size_t bytesRead = 0;
    uint32_t start = millis();
    while ((client->connected() || client->available()) && (bytesRead < length))
    {
        if (client->available())
        {
            *buffer++ = read();
            bytesRead++;
            pos++;
        }
        else
        {
            delay(1);
        }
        if (millis() - start > 2000)
        {
            break;
        }
    }
    return bytesRead;
}

bool BufferedWifiClientReader::isConnectedAndavailable()
{
    return client->connected() && client->available() > 0;
}

boolean BufferedWifiClientReader::seek(size_t newPos)
{
    Serial.print("Seeking to pos ");
    Serial.print(newPos);
    Serial.print(" from pos ");
    Serial.println(pos);
    if (newPos < 0)
    {
        Serial.println("Error: new position is negative");
        return false;
    }
    if (newPos < pos)
    {
        if (!reset())
        {
            Serial.println("Error: failed to reset connection");
            return false;
        }
    }
    Serial.print("Skipping ");
    Serial.print(newPos - pos);
    Serial.println(" bytes");
    while (pos < newPos)
    {
        if (!client->available())
        {
            Serial.println("Error: tried to seek past end of file");
            return false;
        }
        read();
    }
    return true;
}

boolean BufferedWifiClientReader::reconnect(int maxRetries)
{
    size_t oldPos = pos;
    int retries = 0;
    while (!reset() && retries < maxRetries)
    {
        retries++;
        delay(1000); // Wait for a second before retrying
    }
    if (retries == maxRetries)
        return false; // Failed to reset after maxRetries
    if (!seek(oldPos))
        return false;
    return true;
}