#ifndef __TIMETABLE_H__
#define __TIMETABLE_H__

#include <map>
#include <vector>
#include <Arduino.h>

#include <Renderer.h>

typedef struct TimetableEntry {
    String lineNumber;
    String destination;
    String time;
    String delay;
} TimetableEntry;

class Timetable
{
private:
    static const uint16_t numStops = 3;
    static const uint16_t numRowPerStop = 2;
    static const uint16_t paddingLR = 2;
    static const uint16_t numColPerStop = 3;
    static const uint16_t stopRowSize = 100;
    static const uint16_t timeRowSize = 50;
    const uint8_t *font8pt = u8g2_font_helvR08_tf;
    const uint8_t *font10pt = u8g2_font_helvR10_tf;
    const uint8_t *font12pt = u8g2_font_helvR12_tf;
    const uint8_t *font14pt = u8g2_font_helvR14_tf;

    Renderer &renderer;

public:
    Timetable(Renderer &renderer);
    void drawTimetable(uint16_t x, uint16_t y, std::map<String, std::vector<TimetableEntry>> &timetable);
};

#endif