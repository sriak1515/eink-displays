#include <Renderer.h>

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

Renderer::Renderer(Display &display) : display(display)
{
    u8g2Fonts.begin(display.display);
    u8g2Fonts.setFontMode(1);                  // use u8g2 transparent mode (this is default)
    u8g2Fonts.setFontDirection(0);             // left to right (this is default)
    u8g2Fonts.setForegroundColor(GxEPD_BLACK); // apply Adafruit GFX color
    u8g2Fonts.setBackgroundColor(GxEPD_WHITE); // apply Adafruit GFX color
}

uint16_t Renderer::getStringWidth(const String &text, const uint8_t *font)
{
    u8g2Fonts.setFont(font);
    return u8g2Fonts.getUTF8Width(text.c_str());
}

uint16_t Renderer::getStringWidth(const String &text)
{
    return getStringWidth(text, defaultFont);
}


uint16_t Renderer::getStringHeight(const String &text, const uint8_t *font)
{
    u8g2Fonts.setFont(font);
    uint16_t h = u8g2Fonts.getFontAscent();
    if (hasDescender(text))
    {
        h -= u8g2Fonts.getFontDescent();
    }
    return h;
}

uint16_t Renderer::getStringHeight(const String &text)
{
    return getStringHeight(text, defaultFont);
}

void Renderer::getStringBounds(Bounds &bounds, int16_t x, int16_t y, const String &text, const uint8_t *font, horizontal_alignment_t horizontal_alignment, vertical_alignment_t vertical_alignment)
{
    u8g2Fonts.setFont(font);
    uint16_t w, h;
    w = u8g2Fonts.getUTF8Width(text.c_str());
    h = u8g2Fonts.getFontAscent();
    if (horizontal_alignment == RIGHT)
    {
        x = x - w;
    }
    else if (horizontal_alignment == CENTER)
    {
        x = x - w / 2;
    }

    if (vertical_alignment == BOTTOM)
    {
        y = y - h;
    }
    else if (vertical_alignment == MIDDLE)
    {
        y = y - h / 2;
    }

    if (hasDescender(text))
    {
        h -= u8g2Fonts.getFontDescent();
    }


    bounds.h = h;
    bounds.w = w;
    bounds.x = x;
    bounds.y = y;
}

void Renderer::getStringBounds(Bounds &bounds, int16_t x, int16_t y, const String &text, horizontal_alignment_t horizontal_alignment, vertical_alignment_t vertical_alignment)
{
    getStringBounds(bounds, x, y, text, defaultFont, horizontal_alignment, vertical_alignment);
}


void Renderer::drawString(int16_t x, int16_t y, const String &text, const uint8_t *font, uint16_t color, horizontal_alignment_t horizontal_alignment, vertical_alignment_t vertical_alignment)
{
    u8g2Fonts.setFont(font);
    Bounds *bounds = new Bounds();
    getStringBounds(*bounds, x, y, text, font, horizontal_alignment, vertical_alignment);
    u8g2Fonts.setForegroundColor(color);
    u8g2Fonts.setCursor(bounds->x, bounds->y + u8g2Fonts.getFontAscent());
    u8g2Fonts.print(text);
    return;
}

void Renderer::drawString(int16_t x, int16_t y, const String &text, uint16_t color, horizontal_alignment_t horizontal_alignment, vertical_alignment_t vertical_alignment)
{
    drawString(x, y, text, defaultFont, color, horizontal_alignment, vertical_alignment);
}

void Renderer::drawBounds(const Bounds &bounds, uint16_t color)
{
    display.display.drawLine(bounds.x, bounds.y, bounds.x + bounds.w, bounds.y, color);
    display.display.drawLine(bounds.x + bounds.w, bounds.y, bounds.x + bounds.w, bounds.y + bounds.h, color);
    display.display.drawLine(bounds.x + bounds.w, bounds.y + bounds.h, bounds.x, bounds.y + bounds.h, color);
    display.display.drawLine(bounds.x, bounds.y + bounds.h, bounds.x, bounds.y, color);
}

void Renderer::drawCheckboard(const Bounds &bounds, uint16_t squareSize, uint16_t color1, uint16_t color2)
{
    uint16_t x, y;
    for (x = bounds.x; x < bounds.x + bounds.w; x += squareSize)
    {
        for (y = bounds.y; y < bounds.y + bounds.h; y += squareSize)
        {
            if ((x / squareSize + y / squareSize) % 2 == 0)
            {
                display.display.fillRect(x, y, squareSize, squareSize, color1);
            }
            else
            {
                display.display.fillRect(x, y, squareSize, squareSize, color2);
            }
        }
    }
}

bool Renderer::hasDescender(const String &text)
{
    bool hasDescender = false;
    for (size_t i = 0; i < text.length(); i++)
    {
        char c = text.charAt(i);
        if (c == 'g' || c == 'j' || c == 'p' || c == 'q' || c == 'y')
        {
            hasDescender = true;
            break;
        }
    }
    return hasDescender;
}