#ifndef READER_H
#define READER_H

#include <Arduino.h>

class Reader
{
public:
    virtual size_t getPos();
    virtual uint8_t read();
    virtual uint16_t read16();
    virtual uint32_t read32();
    virtual size_t readBytes(uint8_t *buffer, size_t length);
    virtual boolean seek(size_t newPos);
};

#endif // READER_H