// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class TimeService
{
  public:
    TimeService();
    void updateTime(int fallbackOffsetSeconds);
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
    bool updateOffsetFromApi();
    int offset = 0;
    bool apiSuccess = false;
    bool started = false;
    WiFiUDP ntpUDP;
    NTPClient timeClient = NTPClient(ntpUDP, "2.europe.pool.ntp.org");
};
