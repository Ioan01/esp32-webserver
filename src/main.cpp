#include <Arduino.h>
#include <WiFi.h>
#include <AP.h>


void setup()
{
  // put your setup code here, to run once:
  
  Serial.begin(115200);


  Serial.println("config");
  CONFIG::initialize();
  if (!CONFIG::connected)
    AP::start();
  else
    MQTT::start();
}

void loop()
{
}
