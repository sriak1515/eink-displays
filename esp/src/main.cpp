#include <Arduino.h>
#include <LittleFS.h>

#include <config.h>
#include <network.h>
#include <display.h>
#include <filesystem.h>
#include <BufferedWifiClientReader.h>
#include <Reader.h>
#include <FileSystemReader.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting app");
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);

  if (!LittleFS.begin(true))
  {
    Serial.println("Could not initialize filesystem");
    return;
  }
  FileSystem fs = FileSystem(LittleFS);

  Config *config = new Config();

  // fs.listDir("/", 0);

  Serial.println("Loading config.");
  if (!loadConfig("/config.json", fs, *config))
  {
    Serial.println("Could not load config");
    return;
  }

  connectToWiFi(*config);

  //BufferedWifiClientReader client("192.168.0.10", 3001, "/matteo3.bmp", 1024);
  Serial.println("Downloading image.");
  if (!downloadFile("192.168.0.10", 3001, "/matteo3.bmp", "/image.bmp", fs))
  {
   Serial.println("Could not download image.");
   return;
  };
  Serial.println("Initializing display.");
  initDisplay();
  int totalHeight = display.height();
  int pageHeight = display.pageHeight();
  int numPages = display.pages();

  Serial.println("Display infos:");
  Serial.print("Total height:");
  Serial.println(totalHeight);
  Serial.print("Num pages:");
  Serial.println(numPages);
  Serial.print("Page height:");
  Serial.println(pageHeight);


  for (int i = 0; i < numPages; i++)
  {
    int pageHeightToPrint = (i == numPages - 1) ? totalHeight % pageHeight : pageHeight;
    Serial.print("Page number: ");
    Serial.print(i + 1);
    Serial.print(", Page height: ");
    Serial.println(pageHeightToPrint);
  }

  Serial.println("Displaying image.");
  FileSystemReader reader(fs, "/image.bmp");
  //BufferedWifiClientReader reader("192.168.0.10", 3001, "/matteo3.bmp", 128);
  drawBitmapFromSpiffs_Buffered(reader);
  // fs.removeFile("/image.bmp");
}

void loop()
{
  Serial.println("In the loop.");
  delay(2000);
}
