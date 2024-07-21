#include <TimetableRenderer.h>

TimetableRenderer::TimetableRenderer(Renderer &renderer, timetable_t &timetable) : renderer(renderer), timetable(timetable)
{
}

void TimetableRenderer::setTimetable(timetable_t &timetable)
{
    timetable = timetable;
}

bool TimetableRenderer::drawTimetable(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t numStops = timetable.size();
    String longestStopName = getLongestStopName();
    String longestDestinationName = getLongestDestinationName();

    uint16_t maxHeaderHeight = renderer.getStringHeight("Ay", headerFont) + 4 * minimumPadding;
    uint16_t maxHeaderWidth = renderer.getStringWidth(longestStopName, headerFont) + 4 * minimumPadding;

    if (maxHeaderWidth > w)
    {
        Serial.print("Could not draw a table with a width that does not fit destination. Max width is ");
        Serial.print(maxHeaderWidth);
        Serial.print(" and given width is ");
        Serial.println(w);
        return false;
    }

    uint16_t maxEntryHeight = renderer.getStringHeight("Ay", tableFont) + 2 * minimumPadding;
    uint16_t maxDestinationWidth = renderer.getStringWidth(longestDestinationName, tableFont);
    uint16_t maxTimeWidth = renderer.getStringWidth("23:45 +59", tableFont);
    float destinationTimeWidthRatio = (float)maxDestinationWidth / (float)maxTimeWidth;

    uint16_t maxEntryWidth = maxDestinationWidth + 2 * minimumPadding + maxTimeWidth + 2 * minimumPadding;

    uint16_t maxRows = (h - maxHeaderHeight * numStops) / maxEntryHeight;
    uint16_t numRowsPerStop = maxRows / numStops;
    uint16_t numColsPerStop = w / maxEntryWidth;

    if (numRowsPerStop * numColsPerStop < minimumCellsPerStop)
    {
        Serial.print("Could not draw a table with a width that does not fit minimum cells per stop. Maximum cells per stop is ");
        Serial.print(numRowsPerStop * numColsPerStop);
        Serial.print(" and minimum cells per stop is ");
        Serial.println(minimumCellsPerStop);
        return false;
    }

    uint16_t headerHeight = maxHeaderHeight;
    uint16_t rowWidth = w / numColsPerStop;
    uint16_t destinationWidth = rowWidth - maxDestinationWidth;
    uint16_t timeWidth = rowWidth - destinationWidth;
    uint16_t rowHeight = (h - maxHeaderHeight * numStops) / (numRowsPerStop * numStops);

    Serial.print("headerHeight: ");
    Serial.println(headerHeight);
    Serial.print("rowWidth: ");
    Serial.println(rowWidth);
    Serial.print("destinationWidth: ");
    Serial.println(destinationWidth);
    Serial.print("timeWidth: ");
    Serial.println(timeWidth);
    Serial.print("rowHeight: ");
    Serial.println(rowHeight);

    uint16_t newWidth = rowWidth * numColsPerStop;
    uint16_t newHeight = (headerHeight + rowHeight * numRowsPerStop) * numStops;
    uint16_t newX = x + (w - newWidth) / 2;
    uint16_t newY = y + (h - newHeight) / 2;

    Serial.print("newWidth: ");
    Serial.println(newWidth);
    Serial.print("newHeight: ");
    Serial.println(newHeight);
    Serial.print("newX: ");
    Serial.println(newX);
    Serial.print("newY: ");
    Serial.println(newY);

    uint16_t currentY = newX;
    for (auto const &stopEntry : timetable)
    {
        // Draw stop
        renderer.drawString(newX + newWidth / 2, currentY + headerHeight / 2, stopEntry.first, headerFont, GxEPD_BLACK, CENTER, MIDDLE);
        currentY += headerHeight;

        // Draw horizontal lines
        for (size_t i = 0; i <= numRowsPerStop; i++)
        {
            uint16_t lineY = currentY + i * rowHeight;
            renderer.display.display.drawLine(newX, lineY, newX + newWidth, lineY, GxEPD_BLACK);
        }

        // Draw vertical lines
        for (size_t i = 0; i <= numColsPerStop; i++)
        {
            uint16_t lineX = newX + i * rowWidth;
            renderer.display.display.drawLine(lineX, currentY, lineX, currentY + rowHeight * numRowsPerStop, GxEPD_BLACK);
        }

        // Draw entries
        for (size_t i = 0; i < stopEntry.second.size() && i < numRowsPerStop * numColsPerStop; i++)
        {
            uint16_t row = i / numColsPerStop;
            uint16_t col = i % numColsPerStop;

            uint16_t entryX = newX + col * rowWidth;
            uint16_t entryY = currentY + row * rowHeight;

            // Draw destination
            renderer.drawString(entryX + destinationWidth / 2, entryY + rowHeight / 2, stopEntry.second[i]->getFormattedDestination(), tableFont, GxEPD_BLACK, CENTER, MIDDLE);

            // Draw time
            renderer.drawString(entryX + destinationWidth + timeWidth / 2, entryY + rowHeight / 2, stopEntry.second[i]->getFormattedTime(), tableFont, GxEPD_BLACK, CENTER, MIDDLE);
            // Draw separation line
            uint16_t lineX = entryX + destinationWidth;
            renderer.display.display.drawLine(lineX, entryY, lineX, entryY + rowHeight, GxEPD_BLACK);
        }

        currentY += rowHeight * numRowsPerStop;
    }
    return true;
}

String TimetableRenderer::getLongestDestinationName()
{
    String longest;
    int longestSize = 0;
    for (auto const stopEntry : timetable)
    {
        for (auto const destinationEntry : stopEntry.second)
        {
            String currentDestination = destinationEntry->getFormattedDestination();
            if (currentDestination.length() > longestSize)
            {
                longestSize = currentDestination.length();
                longest = currentDestination;
            }
        }
    }
    return longest;
}

String TimetableRenderer::getLongestStopName()
{
    String longest;
    int longestSize = 0;
    for (auto const stopEntry : timetable)
    {
        if (stopEntry.first.length() > longestSize)
        {
            longestSize = stopEntry.first.length();
            longest = stopEntry.first;
        }
    }
    return longest;
}

TimetableEntry::TimetableEntry(String lineNumber, String destination, String time, String delay) : lineNumber(lineNumber), destination(destination), time(time), delay(delay)
{
}

String TimetableEntry::getFormattedDestination()
{
    return lineNumber + " - " + destination;
}

String TimetableEntry::getFormattedTime()
{
    String formattedDelay = "";
    if (delay.length() > 0)
    {
        formattedDelay = " +" + delay;
    }
    return time + formattedDelay;
}
