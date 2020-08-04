/** The MIT License (MIT)

Copyright (c) 2018 David Payne

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h> // --> https://github.com/tzapu/WiFiManager
#include <ArduinoOTA.h>
#include <Adafruit_GFX.h> // --> https://github.com/adafruit/Adafruit-GFX-Library
#include <Max72xxPanel.h> // --> https://github.com/markruys/arduino-Max72xxPanel
#include "MqttClient.h"
#include "WebserverHelpers.h"
#include "TimeService.h"
#include "Storage.h"


/******************************************************************************
 * This is designed for the Wemos D1 ESP8266
 * Wemos D1 Mini:  https://amzn.to/2qLyKJd
 * MAX7219 Dot Matrix Module 4-in-1 Display For Arduino
 * Matrix Display:  https://amzn.to/2HtnQlD
 ******************************************************************************/
// Display Settings
// CLK -> D5 (SCK)  
// CS  -> D6 
// DIN -> D7 (MOSI)
const int pinCS = D6; // Attach CS to this pin, DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4; // default 4 for standard 4 x 1 display Max size of 16
const int numberOfVerticalDisplays = 1; // default 1 for a single row height

const int WEBSERVER_PORT = 80; // The port you can access this device on over HTTP
const boolean WEBSERVER_ENABLED = true;  // Device will provide a web interface via http://[ip]:[port]/

boolean ENABLE_OTA = true;    // this will allow you to load firmware to the device over WiFi (see OTA for ESP8266)
String OTA_Password = "";     // Set an OTA password here -- leave blank if you don't want to be prompted for password
    