#include <Arduino.h>
#include <LittleFS.h>

#include <wifi.h>
#include <Display.h>
#include <Timetable.h>
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
  display->clear();

  Serial.println("Displaying text.");
  display->display.setRotation(1);
  uint32_t startTime = millis();
  uint16_t color;
  Renderer renderer(*display);
  Bounds *bounds = new Bounds();
  String text = "Raphaël";
  // String text = "10:00+2";
  vertical_alignment_t v_alignemnt = BOTTOM;
  horizontal_alignment_t h_alignment = RIGHT;
  int16_t x = display->height / 2;
  int16_t y = display->width / 2;
  // const uint8_t *font = u8g2_font_helvB10_tf;
  const uint8_t *font = u8g2_font_helvB24_tf;

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

  // Create a Timetable object
  Timetable timetable(renderer);

  // Create a sample timetable
  std::map<String, std::vector<TimetableEntry>> sampleTimetable = {
      {"Stop 1", {{"Line 1", "Destination 1", "10:00", "2"}, {"Line 2", "Destination 2", "10:15", "0"}, {"Line 3", "Destination 3", "10:30", "5"}}},
      {"Stop 2", {{"Line 4", "Destination 4", "11:00", "0"}, {"Line 5", "Destination 5", "11:15", "3"}, {"Line 6", "Destination 6", "11:30", "1"}}}};

  // Draw the timetable at position (10, 10)
  do
  {
    timetable.drawTimetable(0, 0, sampleTimetable);
  } while (display->nextPage());

  // display->display.epd2.enableFastPartialMode();
  // display->display.setPartialWindow(bounds->x-10, bounds->y-10, bounds->w + 20, bounds->h+20);
  // for (int i = 0; i < 5; i++)
  //{
  // do
  //{
  //   display->display.fillRect(bounds->x-10, bounds->y-10, bounds->w + 20, bounds->h+20, GxEPD_WHITE);
  //   renderer.drawString(x, y, text, font, GxEPD_BLACK, h_alignment, v_alignemnt);
  //   //renderer.drawBounds(*bounds, GxEPD_BLACK);
  //   //renderer.drawCheckboard(*bounds);
  //   //display->display.drawLine(0, y, display->height, y, GxEPD_BLACK);
  //   //display->display.drawLine(x, 0, x, display->width, GxEPD_BLACK);
  // } while (display->nextPage());
  // do
  //{
  //   display->display.fillRect(bounds->x-10, bounds->y-10, bounds->w + 20, bounds->h+20, GxEPD_WHITE);
  //   renderer.drawString(x, y, "Lysiane", font, GxEPD_BLACK, h_alignment, v_alignemnt);
  //   //renderer.drawBounds(*bounds, GxEPD_BLACK);
  //   //renderer.drawCheckboard(*bounds);
  //   //display->display.drawLine(0, y, display->height, y, GxEPD_BLACK);
  //   //display->display.drawLine(x, 0, x, display->width, GxEPD_BLACK);
  // } while (display->nextPage());
  // do
  //{
  //   display->display.fillRect(bounds->x-10, bounds->y-10, bounds->w + 20, bounds->h+20, GxEPD_WHITE);
  //   renderer.drawString(x, y, "Matteo", font, GxEPD_BLACK, h_alignment, v_alignemnt);
  //   //renderer.drawBounds(*bounds, GxEPD_BLACK);
  //   //renderer.drawCheckboard(*bounds);
  //   //display->display.drawLine(0, y, display->height, y, GxEPD_BLACK);
  //   //display->display.drawLine(x, 0, x, display->width, GxEPD_BLACK);
  // } while (display->nextPage());
  // }
  // display->display.epd2.disableFastPartialMode();
  Serial.print("Text displayed in ");
  Serial.print(millis() - startTime);
  Serial.println(" ms");
  display->display.hibernate();
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
