#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>

// #if defined(GxEPD2_DISPLAY_CLASS) && GxEPD2_DISPLAY_CLASS == GxEPD2_7C
#if defined(DISP_7C)
#include <GxEPD2_7C.h>
#elif defined(DISP_3C)
#include <GxEPD2_3C.h>
#else
#error "Unsupported display class"
#endif

#define CS_PIN 18
#define DC_PIN 20
#define RST_PIN 19
#define BUSY_PIN 1
#define MAX_DISPLAY_BUFFER_SIZE 655360ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))

class Display
{

public:
    GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display = GxEPD2_DRIVER_CLASS(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN);
    boolean hasMultiColors;
    uint16_t curPage;

    Display();
    void initDisplay();
    void reset();
    void clear();
    void refresh();
    void drawPixel(size_t x, size_t y, uint16_t color);
    boolean nextPage();
    size_t height();
    size_t width();
    uint16_t pageHeight();
    uint16_t numPages();
    size_t getPageHeight();
    size_t getPageHeight(size_t pageIdx);
};

#endif // __DISPLAY_H__
