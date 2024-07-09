#include <FS.h>

#include <filesystem.h>

FileSystem::FileSystem(FS fs) : fs(fs)
{
}

void FileSystem::listDir(const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(file.path(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

File FileSystem::openFile(const char *filename, const char *mode)
{
    File file = fs.open(filename, mode);
    if (!file)
    {
        Serial.print("Failed to open file ");
        Serial.println(filename);
    }
    return file;
}

bool FileSystem::removeFile(const char *filename)
{
    if (!fs.exists(filename))
    {
        Serial.print("File ");
        Serial.print(filename);
        Serial.println(" does not exist");
        return false;
    }

    if (fs.remove(filename))
    {
        Serial.print("File ");
        Serial.print(filename);
        Serial.println(" removed");
        return true;
    }
    else
    {
        Serial.print("Failed to remove file ");
        Serial.println(filename);
        return false;
    }
}
