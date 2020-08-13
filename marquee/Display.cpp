// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Display.h"

Display::Display(int _pinCS, int _numberOfHorizontalDisplays, int _numberOfVerticalDisplays, StatusLed& _statusLed, TimeService& _timeClient, std::function<void(void)> _loop): 
  matrix(_pinCS, _numberOfHorizontalDisplays, _numberOfVerticalDisplays), statusLed(_statusLed), timeClient(_timeClient), loop(_loop)
  {
     pinCS = pinCS;
     numberOfHorizontalDisplays = _numberOfHorizontalDisplays;
     numberOfVerticalDisplays = _numberOfVerticalDisplays;
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
  centerPrint(msg, false, false, false);
}

void Display::centerPrint(String msg, boolean extraStuff, bool is24h, bool isPM) {
  int x = (matrix.width() - (msg.length() * width)) / 2;

  // Print the static portions of the display before the main Message
  if (extraStuff) {
    if (!is24h && isPM && timeClient.isPM()) {
      matrix.drawPixel(matrix.width() - 1, 6, HIGH);
    }
  }
  
  matrix.setCursor(x, 0);
  matrix.print(msg);

  matrix.write();
}

void Display::init(int ledRotation){
  Serial.println("Number of LED Displays: " + String(numberOfHorizontalDisplays));
  // initialize dispaly
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

  int maxPos = numberOfHorizontalDisplays * numberOfVerticalDisplays;
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
  displayOn = enable;
  if (enable) {
    matrix.shutdown(false);
    matrix.fillScreen(LOW); // show black
    //Serial.println("Display was turned ON: " + timeClient.timeToConsole());
  } else {
    matrix.shutdown(true);
    Serial.println("Display was turned OFF: " + timeClient.timeToConsole());
  }
}