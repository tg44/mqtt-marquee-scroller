#pragma once

#include <Adafruit_GFX.h> // --> https://github.com/adafruit/Adafruit-GFX-Library
#include <Max72xxPanel.h> // --> https://github.com/markruys/arduino-Max72xxPanel
#include "StatusLed.h"
#include "TimeService.h"

class Display{

  public:
    Display(
        int pinCS, 
        int numberOfHorizontalDisplays, 
        int numberOfVerticalDisplays, 
        StatusLed& statusLed, 
        TimeService& tc,
        std::function<void(void)> loop
        );
    void init(int ledRotation);
    void welcome(int intensity);
    bool isOn();
    void clear();
    void setBrightness(int intensity);
    void switchState();
    void enableDisplay(boolean enable);
    void scrollMessage(String msg, int scrollSpreed);
    void centerPrint(String msg);
    void centerPrint(String msg, boolean extraStuff, bool is24h, bool isPM);
    int numberOfHorizontalDisplays;

  private:
    Max72xxPanel matrix;
    StatusLed& statusLed;
    TimeService& timeClient;
    std::function<void(void)> loop;
    boolean displayOn = true;
    int pinCS;
    int numberOfVerticalDisplays;
    int spacer = 1;  // dots between letters
    int width = 5 + spacer; // The font width is 5 pixels + spacer
    int refresh = 0;

};