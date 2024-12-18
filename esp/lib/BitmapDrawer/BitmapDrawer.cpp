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

void BitmapDrawer::drawRow(size_t rowIndex, int16_t x_offset, int16_t y_offset)
{
    const size_t rowStartPos = getRowPos(rowIndex);
    const size_t rowEndPos = rowStartPos + bmpHeader->rowSize;
    if (reader.getPos() != rowStartPos)
    {
        Serial.print("Warning, for row ");
        Serial.print(rowIndex);
        Serial.print(" expected to be at position ");
        Serial.print(rowStartPos);
        Serial.print(" but was at position ");
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
        display.drawPixel(colIndex + x_offset, rowIndex + y_offset, getColorToDraw(color));
    }

    while (reader.getPos() < rowEndPos)
    {
        reader.read();
    }
}

void BitmapDrawer::drawBitmap(int16_t x_offset, int16_t y_offset)
{
    uint32_t startTime = millis();
    Serial.println("Parsing header");
    parseBMPHeader();

    actualWidth = min(bmpHeader->width, static_cast<uint32_t>(display.width() - x_offset));
    actualHeight = min(bmpHeader->height, static_cast<int32_t>(display.height() - y_offset));
    Serial.print("Actual width: ");
    Serial.println(actualWidth);
    Serial.print("Actual height: ");
    Serial.println(actualHeight);

    Serial.println("Parsing palette");
    parseColorPalette();

    size_t pageStartRow = 0;
    size_t pageEndRow;
    size_t startPos;
    do
    {
        size_t pageHeight = display.getPageHeight();
        pageEndRow = min(pageStartRow + pageHeight - 1, actualHeight - 1);
        if (pageEndRow <= pageStartRow)
        {
            continue;
        }
        // Serial.print("Drawing page ");
        // Serial.print(display.curPage);
        // Serial.print(" of height ");
        // Serial.println(pageHeight);
        // Serial.print("Starting row is ");
        // Serial.print(pageStartRow);
        // Serial.print(" and end row is ");
        // Serial.println(pageEndRow);
        if (bmpHeader->flip)
        {
            Serial.print("Seeking to ");
            Serial.println(pageEndRow);
            reader.seek(getRowPos(pageEndRow));
            Serial.println("Start drawing.");
            for (size_t rowIndex = pageEndRow; rowIndex > pageStartRow; rowIndex--)
            {
                drawRow(rowIndex, x_offset, y_offset);
            }
            drawRow(pageStartRow, x_offset, y_offset);
        }
        else
        {
            reader.seek(getRowPos(pageStartRow));
            for (size_t rowIndex = pageStartRow; rowIndex <= pageEndRow; rowIndex++)
            {
                drawRow(rowIndex, x_offset, y_offset);
            }
        }

        pageStartRow += pageHeight;
    } while (display.nextPage());
    Serial.print("Bitmap loaded in ");
    Serial.print(millis() - startTime);
    Serial.println(" ms");
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
