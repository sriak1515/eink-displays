#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
#include <WiFiClient.h>
#include <GxEPD2_7C.h>

#include <Reader.h>

#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))

extern GxEPD2_7C<GxEPD2_730c_ACeP_730, MAX_HEIGHT(GxEPD2_730c_ACeP_730)> display;

struct BMPHeader {
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
};

boolean parseBMPHeader(Reader &reader, BMPHeader &header);


void initDisplay();
void drawBitmapFromSpiffs_Buffered(Reader &reader, int16_t x = 0, int16_t y = 0, bool with_color = true, bool overwrite = true);

#endif // __DISPLAY_H__
