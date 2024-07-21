#ifndef __RENDERER_H__
#define __RENDERER_H__
#include <Arduino.h>

#include <Display.h>
#include <U8g2_for_Adafruit_GFX.h>

typedef enum horzional_alignment
{
  LEFT,
  RIGHT,
  CENTER
} horizontal_alignment_t;

typedef enum vertical_alignment
{
  TOP,
  BOTTOM,
  MIDDLE
} vertical_alignment_t;

struct Bounds
{
  int16_t x;
  int16_t y;
  uint16_t w;
  uint16_t h;
};

class Renderer
{
private:
  const uint8_t *defaultFont = u8g2_font_helvR14_tf;

public:
  Display &display;
  Renderer(Display &display);
  uint16_t getStringWidth(const String &text, const uint8_t *font);
  uint16_t getStringWidth(const String &text);
  uint16_t getStringHeight(const String &text, const uint8_t *font);
  uint16_t getStringHeight(const String &text);
  void drawString(int16_t x, int16_t y, const String &text, const uint8_t *font, uint16_t color = GxEPD_BLACK, horizontal_alignment_t horizontal_alignment = LEFT, vertical_alignment_t vertical_alignment = TOP);
  void drawString(int16_t x, int16_t y, const String &text, uint16_t color = GxEPD_BLACK, horizontal_alignment_t horizontal_alignment = LEFT, vertical_alignment_t vertical_alignment = TOP);
  void getStringBounds(Bounds &bounds, int16_t x, int16_t y, const String &text, const uint8_t *font, horizontal_alignment_t horizontal_alignment = LEFT, vertical_alignment_t vertical_alignment = TOP);
  void getStringBounds(Bounds &bounds, int16_t x, int16_t y, const String &text, horizontal_alignment_t horizontal_alignment = LEFT, vertical_alignment_t vertical_alignment = TOP);
  void drawCheckboard(const Bounds &bounds, uint16_t squareSize = 1, uint16_t color1 = GxEPD_WHITE, uint16_t color2 = GxEPD_BLACK);
  void drawBounds(const Bounds &bounds, uint16_t color = GxEPD_BLACK);
};
#endif
