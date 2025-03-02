#include <BufferedHTTPClientReader.h>
#include <stdexcept>
#include <format>

BufferedHTTPClientReader::BufferedHTTPClientReader(const char *url, size_t bufferSize, uint16_t timeout, uint16_t numRetries, uint16_t retryDelay) : url(url), bufferSize(bufferSize), bufferPos(0), bufferFill(0), timeout(timeout), pos(0), numRetries(numRetries), retryDelay(retryDelay)
{
    buffer = new uint8_t[bufferSize];
    connect();
}

BufferedHTTPClientReader::~BufferedHTTPClientReader()
{
    delete[] buffer;
    client.end();
}

void BufferedHTTPClientReader::connect()
{
    client.end();
    pos = 0;
    bufferPos = 0;
    bufferFill = 0;
    stream = nullptr;

    int retries = 0;
    while (retries <= numRetries)
    {
        if (retries == numRetries)
        {
            std::string errorMessage;
            errorMessage += "Could not connect to ";
            errorMessage += url;
            errorMessage += " after ";
            errorMessage += std::to_string(numRetries);
            errorMessage += " retries.";
            throw std::runtime_error(errorMessage);
        }
        if (!client.begin(url))
        {
            Serial.print("Connection failed, retrying...");
            retries++;
            delay(retryDelay);
            continue;
        }
        int returnCode = client.GET();
        if (returnCode == HTTP_CODE_OK)
        {
            break;
        }
        else
        {
            Serial.print("Connection failed with error code ");
            Serial.print(returnCode);
            Serial.println(", retrying...");
            retries++;
            delay(retryDelay);
            continue;
        }
    }

    stream = client.getStreamPtr();
}

size_t BufferedHTTPClientReader::getPos()
{
    return pos;
}

uint8_t BufferedHTTPClientReader::read()
{
    if (bufferPos == bufferFill)
    {
        uint32_t start = millis();
        while ((stream->connected() || stream->available()) && bufferPos == bufferFill)
        {
            size_t available = stream->available();
            if (available)
            {
                size_t fetch = available <= bufferSize ? available : bufferSize;
                bufferPos = 0;
                bufferFill = stream->read(buffer, fetch);
                if (bufferFill == 0)
                {
                    Serial.println("Tried to fetch more bytes than available");
                    throw std::runtime_error("Tried to fetch more bytes than available");
                }
            }
            else
            {
                delay(1);
            }
            if (millis() - start > timeout)
                break;
        }
        if (bufferPos == bufferFill)
        {
            Serial.print("Could not fetch more bytes before ");
            Serial.print(timeout);
            Serial.println(" ms timeout");
            throw std::runtime_error("Could not fetch more bytes before 5s timeout");
        }
    }
    uint8_t data = buffer[bufferPos];
    ++pos;
    ++bufferPos;
    return data;
}

uint16_t BufferedHTTPClientReader::read16()
{
    uint16_t data;
    data = (uint16_t)read();
    data |= (uint16_t)read() << 8;
    return data;
}

uint32_t BufferedHTTPClientReader::read32()
{
    uint32_t data;
    data = (uint32_t)read();
    data |= (uint32_t)read() << 8;
    data |= (uint32_t)read() << 16;
    data |= (uint32_t)read() << 24;
    return data;
}

size_t BufferedHTTPClientReader::readBytes(uint8_t *buffer, size_t length)
{
    size_t bytesRead = 0;
    uint32_t start = millis();
    while ((stream->connected() || stream->available()) && (bytesRead < length))
    {
        if (stream->available())
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

bool BufferedHTTPClientReader::isConnectedAndavailable()
{
    return stream->connected() && stream->available() > 0;
}

boolean BufferedHTTPClientReader::seek(size_t newPos)
{
    if (newPos < 0)
    {
        Serial.println("Error: new position is negative");
        return false;
    }
    if (newPos < pos)
    {
        connect();
    }
    while (pos < newPos)
    {
        read();
    }
    return true;
}
