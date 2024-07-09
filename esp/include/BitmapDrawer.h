#ifndef BITMAP_DRAWER_H
#define BITMAP_DRAWER_H

#include <display.h>
#include <Reader.h>

struct BMPHeader
{
    uint32_t fileSize;
    uint32_t imageOffset;
    uint32_t headerSize;
    uint32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t depth;
    uint32_t format;
    bool flip;
    uint32_t rowSize;
    uint32_t numPaletteColors;
};

class BitmapDrawer
{
private:
    static const uint16_t max_palette_pixels = 256;
    Reader &reader;
    Display &display;
    BMPHeader *bmpHeader;
    size_t actualHeight = 0;
    size_t actualWidth = 0;
    uint32_t colorPalette[max_palette_pixels];

    boolean parseBMPHeader();
    boolean parseColorPalette();
    uint32_t readRgb888Color();
    uint32_t readPaletteColor(uint8_t currentByte, size_t index);
    uint16_t getColorToDraw(uint32_t rgb888, boolean withColor = true);
    size_t getRowPos(size_t rowIndex);
    void drawRow(size_t rowIndex);

public:
    BitmapDrawer(Reader &reader, Display &display);
    void drawBitmap(int16_t x = 0, int16_t y = 0);
};

uint16_t rgb888ToRgb565(uint32_t rgb888);
uint32_t rgb565ToRgb888(uint16_t rgb565);
bool isWhitish(uint32_t rgb888, boolean withColor = true);
bool isWhitish(uint16_t rgb565, boolean withColor = true);
bool isColored(uint32_t rgb888);
bool isColored(uint16_t rgb565);

#endif
