#ifndef __FILESYSTEMREADER_H__
#define __FILESYSTEMREADER_H__

#include "Reader.h"
#include "filesystem.h"

class FileSystemReader : public Reader
{
private:
    FileSystem &fs;
    File file;
    size_t pos;

public:
    FileSystemReader(FileSystem &fs, const char *filename);
    ~FileSystemReader();
    size_t getPos() override;
    uint8_t read() override;
    uint16_t read16() override;
    uint32_t read32() override;
    size_t readBytes(uint8_t *buffer, size_t length) override;
    boolean seek(size_t newPos) override;
};

#endif