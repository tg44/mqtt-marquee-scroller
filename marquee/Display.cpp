// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Display.h"
#define PIXEL_PERCENT (100.0 / 64.01)

Display::Display(int _pinCS, int _numberOfHorizontalDisplays, StatusLed& _statusLed, TimeService& _timeClient, std::function<void(void)> _loop): 
  matrix(_pinCS, _numberOfHorizontalDisplays, 1), statusLed(_statusLed), timeClient(_timeClient), loop(_loop)
  {
     pinCS = pinCS;
     numberOfHorizontalDisplays = _numberOfHorizontalDisplays;
  }

void Display::clear(){
  matrix.fillScreen(LOW);
}

void Display::setBrightness(int intensity){
  if(intensity < 0){
    matrix.setIntensity(0);
  } else if (intensity > 15) {
    matrix.setIntensity(15);
  } else {
    matrix.setIntensity(intensity);
  }
}

void Display::switchState(){
  enableDisplay(!displayOn);
}
boolean Display::isOn(){
    return displayOn;
}

void Display::scrollMessage(String msg, int scrollSpeed) {
  msg += " "; // add a space at the end
  for ( unsigned int i = 0 ; i < width * msg.length() + matrix.width() - 1 - spacer; i++ ) {
    loop();
    if (refresh == 1) i = 0;
    refresh = 0;
    matrix.fillScreen(LOW);

    unsigned int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < msg.length() ) {
        matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display
    delay(scrollSpeed);
  }
  matrix.setCursor(0, 0);
}

void Display::centerPrint(String msg) {
  centerPrint(msg, false, 0, numberOfHorizontalDisplays);
}

void Display::centerPrint(String msg, boolean extraDot, byte startPanel, byte awailablePanels) {
  if(msg.length() * width > awailablePanels*8){
    msg = msg.substring(0, awailablePanels*8/width);
  }
  int x = (awailablePanels*8 - (msg.length() * width)) / 2;

  // Print the static portions of the display before the main Message
  if (extraDot) {
    matrix.drawPixel(startPanel*8+awailablePanels*8 - 1, 6, HIGH);
  }
  
  matrix.setCursor(startPanel*8 + x, 0);
  matrix.print(msg);

  matrix.write();
}

void Display::init(int ledRotation){
  Serial.println("Number of LED Displays: " + String(numberOfHorizontalDisplays));
  // initialize dispaly
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

  int maxPos = numberOfHorizontalDisplays * 1;
  for (int i = 0; i < maxPos; i++) {
    matrix.setRotation(i, ledRotation);
    matrix.setPosition(i, maxPos - i - 1, 0);
  }

  Serial.println("matrix created");
  clear();
}
void Display::welcome(int intensity){
  centerPrint("hello");

  for (int inx = 0; inx <= 15; inx++) {
    setBrightness(inx);
    delay(100);
  }
  for (int inx = 15; inx >= 0; inx--) {
    setBrightness(inx);
    delay(60);
  }
  delay(1000);
  setBrightness(intensity);
}

void Display::enableDisplay(boolean enable) {
  if(displayOn != enable){
    if (enable) {
      matrix.shutdown(false);
      matrix.fillScreen(LOW); // show black
      //Serial.println("Display was turned ON: " + timeClient.timeToConsole());
    } else {
      matrix.shutdown(true);
      Serial.println("Display was turned OFF: " + timeClient.timeToConsole());
    }
    displayOn = enable;
  }
}


void Display::showPercentage(byte panel, byte perc, boolean flashOnSeconds) {
  int p = perc / PIXEL_PERCENT;
  if(p > 0 && flashIndicatorInactive(flashOnSeconds) && flashOnSeconds) {
    p -= 1;
  }
  for(int i=0; i<8; i++) {
    for(int j=0; j<8; j++) {
      if(i*8+j <= p) {
        matrix.drawPixel(panel*8+j, 7-i, HIGH);
      } else {
        matrix.drawPixel(panel*8+j, 7-i, LOW);
      }
    }
  }
  matrix.write();
}

void Display::showLargeBinaryTime(byte panel){
  byte h = timeClient.hour();
  drawCol(panel, 0, h/10, true);
  drawCol(panel, 1, h/10, true);
  drawCol(panel, 2, h%10, true);
  drawCol(panel, 3, h%10, true);
  byte m = timeClient.minute();
  drawCol(panel, 6, m/10, true);
  drawCol(panel, 7, m/10, true);
  drawCol(panel+1, 0, m%10, true);
  drawCol(panel+1, 1, m%10, true);
  byte s = timeClient.second();
  drawCol(panel+1, 4, s/10, true);
  drawCol(panel+1, 5, s/10, true);
  drawCol(panel+1, 6, s%10, true);
  drawCol(panel+1, 7, s%10, true);
  matrix.write();
}

void Display::showSmallBinaryTime(byte panel) {
  byte h = timeClient.hour();
  drawCol(panel, 0, h/10, false);
  drawCol(panel, 1, h%10, false);
  byte m = timeClient.minute();
  drawCol(panel, 3, m/10, false);
  drawCol(panel, 4, m%10, false);
  byte s = timeClient.second();
  drawCol(panel, 6, s/10, false);
  drawCol(panel, 7, s%10, false);
  matrix.write();
}
void Display::drawCol(byte panel, byte col, byte num, boolean doubleH){
  for(byte i=0; i<4; i++){
    if((num >> i) % 2 == 1){
      if(!doubleH){
        matrix.drawPixel(panel*8+col, 7-i, HIGH);
      } else {
        matrix.drawPixel(panel*8+col, 7-i*2, HIGH);
        matrix.drawPixel(panel*8+col, 7-i*2-1, HIGH);
      }
    } else {
      if(!doubleH){
        matrix.drawPixel(panel*8+col, 7-i, LOW);
      } else {
        matrix.drawPixel(panel*8+col, 7-i*2, LOW);
        matrix.drawPixel(panel*8+col, 7-i*2-1, LOW);
      }
    }
  }
}

void Display::showCharacter(byte panel, char c) {
  matrix.setCursor(panel*8 + 1, 0);
  matrix.print(c);

  matrix.write();
}

void Display::showTime(byte panel, boolean large, boolean IS_24HOUR, boolean IS_PM, boolean flashOnSeconds) {
  String currentTime = hourMinutes(IS_24HOUR, flashOnSeconds);
  if (large) {
    currentTime += secondsIndicator(flashOnSeconds) + timeClient.zeroPad(timeClient.second());
  }
  clear();
  boolean extraDot = !IS_24HOUR && IS_PM && timeClient.isPM();
  centerPrint(currentTime, extraDot, panel, large ? 6 : 4);
}

String Display::hourMinutes(boolean IS_24HOUR, boolean flashOnSeconds) {
  if (IS_24HOUR) {
    return timeClient.hour() + secondsIndicator(flashOnSeconds) + timeClient.zeroPad(timeClient.minute());
  } else {
    return timeClient.get12hHourFormat() + secondsIndicator(flashOnSeconds) + timeClient.zeroPad(timeClient.second());
  }
}

String Display::secondsIndicator(boolean flashOnSeconds) {
  String rtnValue = ":";
  if (flashIndicatorInactive(flashOnSeconds)) {
    rtnValue = " ";
  }
  return rtnValue;
}

boolean Display::flashIndicatorInactive(boolean flashOnSeconds) {
  return (flashOnSeconds && ((timeClient.second() % 2) == 0));
}