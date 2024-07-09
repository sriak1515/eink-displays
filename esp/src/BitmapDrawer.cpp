#include <BitmapDrawer.h>

BitmapDrawer::BitmapDrawer(Reader &reader) : reader(reader)
{
    bmpHeader = new BMPHeader();
}

boolean BitmapDrawer::parseBMPHeader()
{
    if (reader.getPos() != 0)
    {
        reader.seek(0);
    }
    if (reader.read16() != 0x4D42) // BMP signature
    {
        return false;
    }
    bmpHeader->fileSize = reader.read32();
    uint32_t creatorBytes = reader.read32();
    (void)creatorBytes;                       // unused
    bmpHeader->imageOffset = reader.read32(); // Start of image data
    bmpHeader->headerSize = reader.read32();
    bmpHeader->width = reader.read32();
    bmpHeader->height = (int32_t)reader.read32();
    bmpHeader->planes = reader.read16();
    bmpHeader->depth = reader.read16(); // bits per pixel
    bmpHeader->format = reader.read32();
    bmpHeader->flip = (bmpHeader->height < 0) ? false : true;
    bmpHeader->rowSize = (bmpHeader->width * bmpHeader->depth / 8 + 3) & ~3;
    if (bmpHeader->depth < 8)
        bmpHeader->rowSize = ((bmpHeader->width * bmpHeader->depth + 8 - bmpHeader->depth) / 8 + 3) & ~3;
    if (bmpHeader->height < 0)
        bmpHeader->height = -bmpHeader->height;

    Serial.print("File size: ");
    Serial.println(bmpHeader->fileSize);
    Serial.print("Image Offset: ");
    Serial.println(bmpHeader->imageOffset);
    Serial.print("Header size: ");
    Serial.println(bmpHeader->headerSize);
    Serial.print("Bit Depth: ");
    Serial.println(bmpHeader->depth);
    Serial.print("Image size: ");
    Serial.print(bmpHeader->width);
    Serial.print('x');
    Serial.println(bmpHeader->height);

    return ((bmpHeader->planes == 1) && ((bmpHeader->format == 0) || (bmpHeader->format == 3)));
}

boolean BitmapDrawer::parseColorPalette()
{
    if (bmpHeader->depth > 8)
    {
        return true;
    }
    const size_t paletteStartPos = bmpHeader->imageOffset - (4 << bmpHeader->depth);
    if (reader.getPos() != paletteStartPos)
    {
        if (!reader.seek(paletteStartPos))
            return false;
    }

    uint16_t red, green, blue;
    size_t maxNumColors = (1 << bmpHeader->depth); // 2^depth
    for (size_t idx = 0; idx < maxNumColors; idx++)
    {
        colorPalette[idx] = reader.read32();
    }
    return true;
}

uint32_t BitmapDrawer::readRgb888Color()
{
    uint32_t color;
    switch (bmpHeader->depth)
    {
    case 32:
        color = reader.read32();
        break;
    case 24:
        uint8_t r = reader.read();
        uint8_t g = reader.read();
        uint8_t b = reader.read();
        color = ((r << 16) | (g << 8) | b);
        break;
    case 16:
        uint8_t lsb = reader.read();
        uint8_t msb = reader.read();
        if (bmpHeader->format == 0) // 555
        {
            uint8_t r = (msb & 0x7C) << 1;
            uint8_t g = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
            uint8_t b = (lsb & 0x1F) << 3;
            color = ((r << 16) | (g << 8) | b);
        }
        else // 565
        {
            uint8_t r = (msb & 0xF8);
            uint8_t g = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
            uint8_t b = (lsb & 0x1F) << 3;
            color = ((r << 8) | (g << 3) | (b >> 3));
        }
        break;
    default:
        color = 0; // invalid depth
        break;
    }
    return color;
}

uint32_t BitmapDrawer::readPaletteColor(uint8_t currentByte, size_t columnIndex)
{
    uint32_t color;
    switch (bmpHeader->depth)
    {
    case 1:
        color = (currentByte & (0x01 << (7 - (columnIndex % 8)))) ? colorPalette[1] : colorPalette[0];
        break;
    case 2:
        color = colorPalette[(currentByte >> ((3 - (columnIndex % 4)) * 2)) & 0x03];
        break;
    case 4:
        color = colorPalette[(currentByte >> ((1 - (columnIndex % 2)) * 4)) & 0x0F];
        break;
    case 8:
        color = colorPalette[currentByte];
        break;
    default:
        color = 0; // invalid depth
        break;
    }
    return color;
}

uint16_t BitmapDrawer::getColorToDraw(uint32_t rgb888, boolean withColors)
{
    bool has_multicolors = ((display.epd2.panel == GxEPD2::ACeP730) || display.epd2.panel == GxEPD2::ACeP565) || (display.epd2.panel == GxEPD2::GDEY073D46);

    if (bmpHeader->depth == 1)
        withColors = false;

    if (withColors && has_multicolors)
    {
        return rgb888ToRgb565(rgb888);
    }
    else if (isWhitish(rgb888))
    {
        return GxEPD_WHITE;
    }
    else if (isColored(rgb888) && withColors)
    {
        return GxEPD_COLORED;
    }
    else
    {
        return GxEPD_BLACK;
    }
}

void BitmapDrawer::drawRow(size_t rowIndex)
{
    const size_t rowStartPos = bmpHeader->flip ? bmpHeader->imageOffset + (bmpHeader->height - rowIndex + 1) * bmpHeader->rowSize : bmpHeader->imageOffset + rowIndex * bmpHeader->rowSize;
    const size_t rowEndPos = rowStartPos + bmpHeader->rowSize;
    if (reader.getPos() != rowStartPos)
    {
        Serial.print("Warning, expected to be at position ");
        Serial.print(rowStartPos);
        Serial.print(" but was a position ");
        Serial.print(reader.getPos());
        Serial.println(". Seeking to correct position");
        reader.seek(rowStartPos);
    }

    for (size_t colIndex = 0; colIndex < actualWidth; colIndex++)
    {
        uint32_t color;
        uint8_t currentByte;
        if (bmpHeader->depth > 8)
        {
            color = readRgb888Color();
        }
        else
        {
            if (colIndex % (8 / bmpHeader->depth) == 0)
            {
                currentByte = reader.read();
            }
            color = readPaletteColor(currentByte, colIndex);
        }

        display.drawPixel(rowIndex, colIndex, getColorToDraw(color));
    }

    // Skip remaining bytes
    while (reader.getPos() < rowEndPos)
    {
        reader.read();
    }
}

void BitmapDrawer::drawBitmap(int16_t x, int16_t y)
{
    actualWidth = min(bmpHeader->width, static_cast<uint32_t>(display.width() - x));
    actualHeight = min(bmpHeader->height, static_cast<int32_t>(display.height() - y));
}

uint16_t rgb888ToRgb565(uint32_t rgb888)
{
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8) & 0xFF;
    uint8_t b = rgb888 & 0xFF;

    uint16_t r5 = (r >> 3) & 0x1F;
    uint16_t g6 = (g >> 2) & 0x3F;
    uint16_t b5 = (b >> 3) & 0x1F;

    uint16_t rgb565 = (r5 << 11) | (g6 << 5) | b5;
    return rgb565;
}

uint32_t rgb565ToRgb888(uint16_t rgb565)
{
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;

    // Scale RGB 565 values to 8-bit range
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return (r << 16) | (g << 8) | b;
}

bool isWhitish(uint32_t rgb888, boolean withColors)
{
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8) & 0xFF;
    uint8_t b = rgb888 & 0xFF;

    if (withColors)
    {
        return (r > 0x80) && (g > 0x80) && (b > 0x80);
    }
    else
    {
        return ((r + g + b) > 3 * 0x80);
    }
}

bool isWhitish(uint16_t rgb565, boolean withColors)
{
    uint32_t rgb888 = rgb565ToRgb888(rgb565);
    return isWhitish(rgb888, withColors);
}

bool isColored(uint32_t rgb888)
{
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8) & 0xFF;
    uint8_t b = rgb888 & 0xFF;

    return (r > 0xF0) || ((g > 0xF0) && (b > 0xF0));
}

bool isColored(uint16_t rgb565)
{
    uint32_t rgb888 = rgb565ToRgb888(rgb565);
    return isColored(rgb888);
}
