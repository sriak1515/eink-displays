#include <FS.h>

#include <display.h>
#include <Reader.h>

Display::Display() : curPage(0)
{
  width = display.width();
  height = display.height();
  hasMultiColors = ((display.epd2.panel == GxEPD2::ACeP730) || display.epd2.panel == GxEPD2::ACeP565) || (display.epd2.panel == GxEPD2::GDEY073D46) || (display.epd2.panel == GxEPD2::GDEM037F51);
  numPages = display.pages();
  pageHeight = display.pageHeight();
}

void Display::initDisplay()
{
  display.init(115200, true, 10, false);
  //display.init(115200, true, 2, false);
  reset();
  return;
}

void Display::reset()
{
  curPage = 0;
  display.setRotation(0);
  display.setTextSize(1);
  display.setTextColor(GxEPD_BLACK);
  display.setTextWrap(false);
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
}

void Display::clear()
{
  display.clearScreen();
}

void Display::refresh()
{
  display.refresh();
}

void Display::drawPixel(size_t x, size_t y, uint16_t color)
{
  display.drawPixel(x, y, color);
}

size_t Display::getPageHeight(size_t pageIdx)
{
  if (pageIdx >= numPages)
  {
    return -1;
  }
  else if (pageIdx == numPages - 1 && height % pageHeight != 0)
  {
    return height % pageHeight;
  }
  else
  {
    return pageHeight;
  }
}

size_t Display::getPageHeight()
{
  return getPageHeight(curPage);
}

boolean Display::nextPage()
{
  if (display.nextPage())
  {
    curPage++;
    return true;
  }
  else
  {
    return false;
  }
}
