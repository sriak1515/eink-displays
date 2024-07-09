#include <FS.h>

#include <display.h>
#include <Reader.h>

GxEPD2_7C<GxEPD2_730c_ACeP_730, MAX_HEIGHT(GxEPD2_730c_ACeP_730)> display(GxEPD2_730c_ACeP_730(/*CS=5*/ 18, /*DC=*/20, /*RST=*/19, /*BUSY=*/1));

void initDisplay()
{
  display.init(115200, true, 2, false);
  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
  display.setTextWrap(false);
  display.setFullWindow();
  display.firstPage();
  return;
}

static const uint16_t input_buffer_pixels = 800; // may affect performance

static const uint16_t max_row_width = 800;      // for up to 7.8" display 1872x1404
static const uint16_t max_palette_pixels = 256; // for depth <= 8

uint8_t input_buffer[3 * input_buffer_pixels];        // up to depth 24
uint8_t output_row_mono_buffer[max_row_width / 8];    // buffer for at least one row of b/w bits
uint8_t output_row_color_buffer[max_row_width / 8];   // buffer for at least one row of color bits
uint8_t mono_palette_buffer[max_palette_pixels / 8];  // palette buffer for depth <= 8 b/w
uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
uint16_t rgb_palette_buffer[max_palette_pixels];      // palette buffer for depth <= 8 for buffered graphics, needed for 7-color display

uint16_t read16(File &f);
uint32_t read32(File &f);

boolean parseBMPHeader(Reader &reader, BMPHeader &header)
{
  if (reader.read16() != 0x4D42) // BMP signature
  {
    return false;
  }
  header.fileSize = reader.read32();
  uint32_t creatorBytes = reader.read32();
  (void)creatorBytes;                   // unused
  header.imageOffset = reader.read32(); // Start of image data
  header.headerSize = reader.read32();
  header.width = reader.read32();
  header.height = (int32_t)reader.read32();
  header.planes = reader.read16();
  header.depth = reader.read16(); // bits per pixel
  header.format = reader.read32();
  header.flip = (header.height < 0) ? false : true;
  header.rowSize = (header.width * header.depth / 8 + 3) & ~3;
  if (header.depth < 8)
    header.rowSize = ((header.width * header.depth + 8 - header.depth) / 8 + 3) & ~3;
  if (header.height < 0)
    header.height = -header.height;

  Serial.print("File size: ");
  Serial.println(header.fileSize);
  Serial.print("Image Offset: ");
  Serial.println(header.imageOffset);
  Serial.print("Header size: ");
  Serial.println(header.headerSize);
  Serial.print("Bit Depth: ");
  Serial.println(header.depth);
  Serial.print("Image size: ");
  Serial.print(header.width);
  Serial.print('x');
  Serial.println(header.height);

  return ((header.planes == 1) && ((header.format == 0) || (header.format == 3)));
}

void drawBitmapFromSpiffs_Buffered(Reader &reader, int16_t x, int16_t y, bool with_color, bool overwrite)
{
  bool has_multicolors = ((display.epd2.panel == GxEPD2::ACeP730) || display.epd2.panel == GxEPD2::ACeP565) || (display.epd2.panel == GxEPD2::GDEY073D46);
  BMPHeader *header = new BMPHeader();
  uint32_t startTime = millis();
  if ((x >= display.width()) || (y >= display.height()))
    return;
  parseBMPHeader(reader, *header);

  uint16_t w = min(header->width, static_cast<uint32_t>(display.width() - x));
  uint16_t h = min(header->height, static_cast<int32_t>(display.height() - y));

  uint8_t bitmask = 0xFF;
  uint8_t bitshift = 8 - header->depth;
  uint16_t red, green, blue;
  bool whitish = false;
  bool colored = false;
  if (header->depth == 1)
    with_color = false;
  if (header->depth <= 8)
  {
    if (header->depth < 8)
      bitmask >>= header->depth;
    // file.seek(54); //palette is always @ 54
    reader.seek(header->imageOffset - (4 << header->depth)); // 54 for regular, diff for colorsimportant
    for (uint16_t pn = 0; pn < (1 << header->depth); pn++)
    {
      blue = reader.read();
      green = reader.read();
      red = reader.read();
      reader.read();
      whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
      colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
      if (0 == pn % 8)
        mono_palette_buffer[pn / 8] = 0;
      mono_palette_buffer[pn / 8] |= whitish << pn % 8;
      if (0 == pn % 8)
        color_palette_buffer[pn / 8] = 0;
      color_palette_buffer[pn / 8] |= colored << pn % 8;
      rgb_palette_buffer[pn] = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
    }
  }
  display.setFullWindow();
  display.firstPage();
  do
  {
    if (!overwrite)
      display.fillScreen(GxEPD_WHITE);
    uint32_t rowPosition = header->flip ? header->imageOffset + (header->height - h) * header->rowSize : header->imageOffset;
    for (uint16_t row = 0; row < h; row++, rowPosition += header->rowSize) // for each line
    {
      uint32_t in_remain = header->rowSize;
      uint32_t in_idx = 0;
      uint32_t in_bytes = 0;
      uint8_t in_byte = 0; // for depth <= 8
      uint8_t in_bits = 0; // for depth <= 8
      uint16_t color = GxEPD_WHITE;
      reader.seek(rowPosition);
      for (uint16_t col = 0; col < w; col++) // for each pixel
      {
        // Time to read more pixel data?
        if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS multiple of 3)
        {
          in_bytes = reader.readBytes(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
          in_remain -= in_bytes;
          in_idx = 0;
        }
        switch (header->depth)
        {
        case 32:
          blue = input_buffer[in_idx++];
          green = input_buffer[in_idx++];
          red = input_buffer[in_idx++];
          in_idx++;                                                                                                     // skip alpha
          whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
          colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
          color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
          break;
        case 24:
          blue = input_buffer[in_idx++];
          green = input_buffer[in_idx++];
          red = input_buffer[in_idx++];
          whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
          colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
          color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
          break;
        case 16:
        {
          uint8_t lsb = input_buffer[in_idx++];
          uint8_t msb = input_buffer[in_idx++];
          if (header->format == 0) // 555
          {
            blue = (lsb & 0x1F) << 3;
            green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
            red = (msb & 0x7C) << 1;
            color = ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
          }
          else // 565
          {
            blue = (lsb & 0x1F) << 3;
            green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
            red = (msb & 0xF8);
            color = (msb << 8) | lsb;
          }
          whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
          colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0));                                                  // reddish or yellowish?
        }
        break;
        case 1:
        case 2:
        case 4:
        case 8:
        {
          if (0 == in_bits)
          {
            in_byte = input_buffer[in_idx++];
            in_bits = 8;
          }
          uint16_t pn = (in_byte >> bitshift) & bitmask;
          whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
          colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
          in_byte <<= header->depth;
          in_bits -= header->depth;
          color = rgb_palette_buffer[pn];
        }
        break;
        }
        if (with_color && has_multicolors)
        {
          // keep color
        }
        else if (whitish)
        {
          color = GxEPD_WHITE;
        }
        else if (colored && with_color)
        {
          color = GxEPD_COLORED;
        }
        else
        {
          color = GxEPD_BLACK;
        }
        uint16_t yrow = y + (header->flip ? h - row - 1 : row);
        display.drawPixel(x + col, yrow, color);
      } // end pixel
    } // end line
  } while (display.nextPage());
  Serial.print("loaded in ");
  Serial.print(millis() - startTime);
  Serial.println(" ms");
}

uint16_t read16(File &f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}