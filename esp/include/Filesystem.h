#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <FS.h>

class FileSystem {
private:
    fs::FS fs;

public:
    FileSystem(fs::FS fs);
    void listDir(const char *dirname, uint8_t levels);
    File openFile(const char *filename, const char *mode);
    bool removeFile(const char *filename);
};

#endif
