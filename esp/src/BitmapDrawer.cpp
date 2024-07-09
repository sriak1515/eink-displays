#include <BitmapDrawer.h>

BitmapDrawer::BitmapDrawer(Reader &reader, Display &display) : reader(reader), display(display)
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
    reader.read32(); // image size
    reader.read32(); // horizontal ppm
    reader.read32(); // vertical ppm
    reader.read32(); // vertical ppm
    bmpHeader->numPaletteColors = reader.read32();
    if (bmpHeader->numPaletteColors == 0)
    {
        bmpHeader->numPaletteColors = (1 << bmpHeader->depth); // 2^depth
    }
    reader.read32(); // num important colors
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
    Serial.print("Num colors in palette: ");
    Serial.println(bmpHeader->numPaletteColors);
    Serial.print("Image flipped?: ");
    Serial.println(bmpHeader->flip ? "yes" : "no");
    Serial.print("Image size: ");
    Serial.print(bmpHeader->width);
    Serial.print('x');
    Serial.println(bmpHeader->height);
    Serial.print("Row size: ");
    Serial.println(bmpHeader->rowSize);

    return ((bmpHeader->planes == 1) && ((bmpHeader->format == 0) || (bmpHeader->format == 3)));
}

boolean BitmapDrawer::parseColorPalette()
{
    if (bmpHeader->depth > 8)
    {
        return true;
    }
    const size_t paletteStartPos = bmpHeader->imageOffset - 4 * bmpHeader->numPaletteColors;
    if (reader.getPos() != paletteStartPos)
    {
        if (!reader.seek(paletteStartPos))
            return false;
    }

    uint16_t red, green, blue;
    for (size_t idx = 0; idx < bmpHeader->numPaletteColors; idx++)
    {
        blue = reader.read();
        green = reader.read();
        red = reader.read();
        reader.read();

        colorPalette[idx] = ((red << 16) | (green << 8) | blue);
        Serial.print("Palette color for idx ");
        Serial.print(idx);
        Serial.print(": ");
        char hex[7] = {0};
        sprintf(hex, "#%02X%02X%02X", red, green, blue); // convert to an hexadecimal string. Lookup sprintf for what %02X means.
        Serial.println(hex);
    }
    return true;
}

uint32_t BitmapDrawer::readRgb888Color()
{
    uint32_t color;
    uint8_t r, g, b;
    switch (bmpHeader->depth)
    {
    case 32:
        b = reader.read();
        g = reader.read();
        r = reader.read();
        reader.read();
        color = ((r << 16) | (g << 8) | b);
        break;
    case 24:
        b = reader.read();
        g = reader.read();
        r = reader.read();
        color = ((r << 16) | (g << 8) | b);
        break;
    case 16:
    {
        uint8_t lsb = reader.read();
        uint8_t msb = reader.read();
        if (bmpHeader->format == 0) // 555
        {
            r = (msb & 0x7C) << 1;
            g = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
            b = (lsb & 0x1F) << 3;
            color = ((r << 16) | (g << 8) | b);
        }
        else // 565
        {
            r = (msb & 0xF8);
            g = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
            b = (lsb & 0x1F) << 3;
            color = ((r << 8) | (g << 3) | (b >> 3));
        }
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
    uint8_t paletteIndex;

    switch (bmpHeader->depth)
    {
    case 1:
        paletteIndex = (currentByte >> (7 - (columnIndex & 7))) & 0x1;
        break;
    case 2:
        paletteIndex = (currentByte >> (6 - ((columnIndex & 3) * 2))) & 0x3;
        break;
    case 4:
        paletteIndex = (currentByte >> (4 - ((columnIndex & 1) * 4))) & 0xF;
        break;
    case 8:
        paletteIndex = currentByte;
        break;
    default:
        // Handle error: unsupported depth
        return 0;
    }

    // Serial.println(paletteIndex);
    color = colorPalette[paletteIndex];
    return color;
}

uint16_t BitmapDrawer::getColorToDraw(uint32_t rgb888, boolean withColors)
{
    if (bmpHeader->depth == 1)
        withColors = false;

    if (withColors && display.hasMultiColors)
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

size_t BitmapDrawer::getRowPos(size_t rowIndex)
{
    return bmpHeader->flip ? bmpHeader->imageOffset + (bmpHeader->height - rowIndex - 1) * bmpHeader->rowSize : bmpHeader->imageOffset + rowIndex * bmpHeader->rowSize;
}

void BitmapDrawer::drawRow(size_t rowIndex)
{
    // if (rowIndex % 10 == 0)
    //{
    //     Serial.println();
    //     Serial.print("Drawing row ");
    //     Serial.println(rowIndex + 1);
    //     Serial.print("Index: ");
    //     Serial.println(getRowPos(rowIndex));
    //     Serial.print("Rows offset: ");
    //     Serial.println(bmpHeader->height - rowIndex);
    //     Serial.print("Target offset: ");
    //     Serial.println(getRowPos(rowIndex));
    //     Serial.print("Current pos: ");
    //     Serial.println(reader.getPos());
    //     Serial.println();
    // }
    const size_t rowStartPos = getRowPos(rowIndex);
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

    for (size_t colIndex = 0; colIndex < bmpHeader->width; colIndex++)
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
        display.drawPixel(colIndex, rowIndex, getColorToDraw(color));
    }

    // Skip remaining bytes
    // if (reader.getPos() < rowEndPos)
    //{
    //    Serial.print("We need to skip ");
    //    Serial.print(rowEndPos - reader.getPos());
    //    Serial.print(" bytes. We are at byte ");
    //    Serial.print(reader.getPos());
    //    Serial.print(" and need to arrive at byte ");
    //    Serial.println(rowEndPos);
    //}
    while (reader.getPos() < rowEndPos)
    {
        reader.read();
    }
}

void BitmapDrawer::drawBitmap(int16_t x, int16_t y)
{
    uint32_t startTime = millis();
    actualWidth = min(bmpHeader->width, static_cast<uint32_t>(display.width - x));
    actualHeight = min(bmpHeader->height, static_cast<int32_t>(display.height - y));
    Serial.print("Actual width: ");
    Serial.println(actualWidth);
    Serial.print("Actual height: ");
    Serial.println(actualHeight);

    Serial.println("Parsing header");
    parseBMPHeader();

    Serial.println("Parsing palette");
    parseColorPalette();

    Serial.println("Resetting display");
    display.reset();

    size_t pageStartRow;
    size_t pageEndRow;
    size_t startPos;
    do
    {
        size_t pageHeight = display.getPageHeight();
        pageEndRow = pageStartRow + pageHeight - 1;

        if (bmpHeader->flip)
        {
            for (size_t rowIndex = pageEndRow; rowIndex > pageStartRow; rowIndex--)
            {
                drawRow(rowIndex);
            }
            drawRow(pageStartRow);
        }
        else
        {
            reader.seek(getRowPos(pageStartRow));
            for (size_t rowIndex = pageStartRow; rowIndex <= pageEndRow; rowIndex++)
            {
                drawRow(rowIndex);
            }
        }

        pageStartRow += pageHeight;
    } while (display.nextPage());
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
