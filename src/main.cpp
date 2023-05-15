#include <Arduino.h>
#include <WiFi.h>
#include <AP.h>
#include <config.h>
// put function declarations here:

void setup()
{
  // put your setup code here, to run once:

  Serial.begin(115200);
  // CONFIG::initialize();
  AP::start();
}

void loop()
{
}
