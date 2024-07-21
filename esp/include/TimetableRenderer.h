#ifndef __TIMETABLE_RENDERER_H__
#define __TIMETABLE_RENDERER_H__

#include <map>
#include <vector>
#include <Arduino.h>

#include <Renderer.h>

class TimetableEntry
{
private:
    String lineNumber;
    String destination;
    String time;
    String delay;
public:
    TimetableEntry(String lineNumber, String destination, String time, String delay);

    String getFormattedDestination();
    String getFormattedTime();
};

typedef std::map<String, std::vector<TimetableEntry*>> timetable_t;

class TimetableRenderer
{
private:
    static const uint16_t minimumPadding = 2;
    static const uint16_t minimumCellsPerStop = 4;
    const uint8_t *headerFont = u8g2_font_helvR12_tf;
    const uint8_t *tableFont = u8g2_font_helvR10_tf;

    Renderer &renderer;
    timetable_t &timetable;

    String getLongestStopName();
    String getLongestDestinationName();

public:
    TimetableRenderer(Renderer &renderer, timetable_t &timetable);
    bool drawTimetable(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    void setTimetable(timetable_t &timetable);
};
#endif