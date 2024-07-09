#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
#include <GxEPD2_7C.h>

#define CS_PIN 18
#define DC_PIN 20
#define RST_PIN 19
#define BUSY_PIN 1
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))

class Display
{

public:
    GxEPD2_7C<GxEPD2_730c_ACeP_730, GxEPD2_730c_ACeP_730::HEIGHT> display = GxEPD2_730c_ACeP_730(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN);
    size_t height;
    size_t width;
    boolean hasMultiColors;
    uint16_t numPages;
    uint16_t pageHeight;
    uint16_t curPage;

    Display();
    void initDisplay();
    void reset();
    void clear();
    void refresh();
    void drawPixel(size_t x, size_t y, uint16_t color);
    boolean nextPage();
    size_t getPageHeight();
    size_t getPageHeight(size_t pageIdx);
};

#endif // __DISPLAY_H__
