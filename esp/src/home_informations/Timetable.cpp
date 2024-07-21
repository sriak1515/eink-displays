#include <Timetable.h>

Timetable::Timetable(Renderer &renderer) : renderer(renderer) {}

void Timetable::drawTimetable(uint16_t x, uint16_t y, std::map<String, std::vector<TimetableEntry>> &timetable) {
    uint16_t stopRowHeight = renderer.getStringHeight("Ay", font12pt) + paddingLR * 2;
    uint16_t entryRowHeight = renderer.getStringHeight("Ay", font10pt) + paddingLR * 2;
    uint16_t stopRowWidth = renderer.display.display.width() - timeRowSize;
    uint16_t entryRowWidth = stopRowWidth / numColPerStop;

    uint16_t currentY = y;
    uint16_t currentX = x;
    for (auto const &stopEntry : timetable) {
        // Draw stop
        renderer.drawString(x, currentY, stopEntry.first, font12pt, GxEPD_BLACK, LEFT, TOP);
        currentY += stopRowHeight;

        // Draw entries
        for (size_t i = 0; i < stopEntry.second.size(); i++) {
            TimetableEntry entry = stopEntry.second[i];
            String lineDestination = entry.lineNumber + " - " + entry.destination;
            String timeDelay = entry.time;
            if (entry.delay.toInt() > 1) {
                timeDelay += " +" + entry.delay;
            }

            // Compute bounds for lineDestination and timeDelay
            Bounds lineDestinationBounds, timeDelayBounds;
            renderer.getStringBounds(lineDestinationBounds, x, currentY, lineDestination, font10pt, LEFT, TOP);
            renderer.getStringBounds(timeDelayBounds, x + stopRowWidth, currentY, timeDelay, font10pt, RIGHT, TOP);

            // Elipse strings if they are too large
            if (lineDestinationBounds.w > entryRowWidth) {
                lineDestination = lineDestination.substring(0, entryRowWidth / renderer.getStringWidth("W", font10pt)) + "...";
            }
            if (timeDelayBounds.w > timeRowSize) {
                timeDelay = timeDelay.substring(0, timeRowSize / renderer.getStringWidth("W", font10pt)) + "...";
            }

            // Draw lineDestination and timeDelay
            renderer.drawString(x, currentY, lineDestination, font10pt, GxEPD_BLACK, LEFT, TOP);
            renderer.drawString(x + stopRowWidth, currentY, timeDelay, font10pt, GxEPD_BLACK, RIGHT, TOP);

            // Draw vertical line
            if ((i + 1) % numColPerStop == 0) {
                currentY += entryRowHeight;
            } else {
                renderer.display.display.drawLine(x + entryRowWidth * ((i + 1) % numColPerStop), currentY, x + entryRowWidth * ((i + 1) % numColPerStop), currentY + entryRowHeight, GxEPD_BLACK);
            }
        }

        // Draw horizontal line
        renderer.display.display.drawLine(x, currentY, x + stopRowWidth, currentY, GxEPD_BLACK);
        currentY += entryRowHeight;
    }
}
