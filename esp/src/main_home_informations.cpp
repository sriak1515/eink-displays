#include <Arduino.h>
#include <LittleFS.h>

#include <wifi.h>
#include <Display.h>
#include <Renderer.h>
#include <BitmapDrawer.h>
#include <BufferedWifiClientReader.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  delay(1000);
  Serial.println("Starting app");

  Serial.println("Initializing display.");
  Display *display = new Display();
  display->initDisplay();
  // display->clear();

  Serial.println("Displaying text.");
  display->display.setRotation(1);
  uint32_t startTime = millis();
  uint16_t color;
  Renderer renderer(*display);
  Bounds *bounds = new Bounds();
  String text = "Raphaël";
  //String text = "10:00+2";
  vertical_alignment_t v_alignemnt = MIDDLE;
  horizontal_alignment_t h_alignment = CENTER;
  int16_t x = display->height / 2;
  int16_t y = display->width / 2;
  const uint8_t *font = u8g2_font_helvB10_tf;

  renderer.getStringBounds(*bounds, x, y, text, font, h_alignment, v_alignemnt);
  Serial.println("Bounds:");
  Serial.print("  x: ");
  Serial.println(bounds->x);
  Serial.print("  y: ");
  Serial.println(bounds->y);
  Serial.print("  w: ");
  Serial.println(bounds->w);
  Serial.print("  h: ");
  Serial.println(bounds->h);
  do
  {
    renderer.drawString(x, y, text, font, GxEPD_BLACK, h_alignment, v_alignemnt);
    renderer.drawBounds(*bounds, GxEPD_COLORED);
    //renderer.drawCheckboard(*bounds);
    display->display.drawLine(0, y, display->height, y, GxEPD_COLORED);
    display->display.drawLine(x, 0, x, display->width, GxEPD_COLORED);
  } while (display->nextPage());
  Serial.print("Text displayed in ");
  Serial.print(millis() - startTime);
  Serial.println(" ms");

  Serial.println("Refreshing display.");
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
