// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

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
    void showPercentage(byte panel, byte perc, boolean showLast);
    void showTime(byte panel, boolean large, boolean IS_24HOUR, boolean IS_PM, boolean flashOnSeconds);
    void showLargeBinaryTime(byte panel);
    void showSmallBinaryTime(byte panel);
    void showCharacter(byte panel, char c);
    int numberOfHorizontalDisplays;

  private:
    void centerPrint(String msg, boolean extraDot, byte startPanel, byte awailablePanels);
    String hourMinutes(boolean IS_PM, boolean flashOnSeconds);
    String secondsIndicator(boolean flashOnSeconds);
    boolean flashIndicatorInactive(boolean flashOnSeconds);
    void drawCol(byte panel, byte col, byte num, boolean doubleH);
    Max72xxPanel matrix;
    StatusLed& statusLed;
    TimeService& timeClient;
    std::function<void(void)> loop;
    boolean displayOn = true;
    int pinCS;
    int spacer = 1;  // dots between letters
    int width = 5 + spacer; // The font width is 5 pixels + spacer
    int refresh = 0;

};