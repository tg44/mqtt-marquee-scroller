#pragma once
#include "libs/ArduinoJson/ArduinoJson.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class TimeService
{
  public:
    TimeService();
    void updateTime();
    bool isPM();
    int minute();
    int hour();
    int second();
    int get12hHourFormat();
    String timeToConsole();
    String getAmPm();
    String zeroPad(int number);
    long getEpoch();
    
  private:
    void updateOffsetData();
    int offset;
    long lastEpochFromApi;
    bool started = false;
    String servername = "worldtimeapi.org";
    WiFiUDP ntpUDP;
    NTPClient timeClient = NTPClient(ntpUDP, "2.europe.pool.ntp.org");
};