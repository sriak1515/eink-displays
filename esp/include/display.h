#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define ENABLE_GxEPD2_GFX 0

#include <Arduino.h>
#include <GxEPD2_7C.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_4C.h>

#define CS_PIN 18
#define DC_PIN 20
#define RST_PIN 19
#define BUSY_PIN 1
//#define GxEPD2_DISPLAY_CLASS GxEPD2_7C
//#define GxEPD2_DRIVER_CLASS GxEPD2_730c_ACeP_730
//#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
//#define GxEPD2_DRIVER_CLASS GxEPD2_750c_Z08
#define GxEPD2_DISPLAY_CLASS GxEPD2_4C
#define GxEPD2_DRIVER_CLASS GxEPD2_370c_GDEM037F51
#define MAX_DISPLAY_BUFFER_SIZE 65536ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE) / (EPD::WIDTH / 2))

class Display
{

public:
    GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, GxEPD2_DRIVER_CLASS::HEIGHT> display = GxEPD2_DRIVER_CLASS(CS_PIN, DC_PIN, RST_PIN, BUSY_PIN);
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
