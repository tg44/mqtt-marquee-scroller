#pragma once
#include "Arduino.h"

class StatusLed {
    public:
      StatusLed(int pin): externalLight(pin){}
      void init(){
        pinMode(externalLight, OUTPUT);
      }
      void on() {
        digitalWrite(externalLight, LOW);
      }
      void off(){
        digitalWrite(externalLight, HIGH);
      }
      void flashLED(int number, int delayTime) {
        for (int inx = 0; inx < number; inx++) {
            delay(delayTime);
            on();
            delay(delayTime);
            off();
            delay(delayTime);
        }
      }
    private:
      int externalLight;
};