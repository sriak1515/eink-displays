#include "FileSystemReader.h"

FileSystemReader::FileSystemReader(FileSystem &fs, const char *filename) : fs(fs), file(fs.openFile(filename, "r")), pos(0) {}

FileSystemReader::~FileSystemReader()
{
    if (file)
    {
        file.close();
    }
}

size_t FileSystemReader::getPos()
{
    return pos;
}

uint8_t FileSystemReader::read()
{
    ++pos;
    return file.read();
}

uint16_t FileSystemReader::read16()
{
    uint16_t result;
    ((uint8_t *)&result)[0] = read(); // LSB
    ((uint8_t *)&result)[1] = read(); // MSB
    return result;
}

uint32_t FileSystemReader::read32()
{
    uint32_t result;
    ((uint8_t *)&result)[0] = read(); // LSB
    ((uint8_t *)&result)[1] = read();
    ((uint8_t *)&result)[2] = read();
    ((uint8_t *)&result)[3] = read(); // MSB
    return result;
}

size_t FileSystemReader::readBytes(uint8_t *buffer, size_t length)
{
    const size_t numBytesRead = file.read(buffer, length);
    pos += numBytesRead;
    return numBytesRead;
}

boolean FileSystemReader::seek(size_t newPos)
{
    if (file.seek(newPos)) {
        pos = newPos;
        return true;
    } else {
        return false;
    }
}
