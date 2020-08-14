// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Arduino.h"
#define VERSION "2.16"
#define HOSTNAME "CLOCK-"

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
// Change the externalLight to the pin you wish to use if other than the Built-in LED
const int STATUS_LED = LED_BUILTIN; // LED_BUILTIN is is the built in LED on the Wemos

const int WEBSERVER_PORT = 80; // The port you can access this device on over HTTP
const boolean WEBSERVER_ENABLED = true;  // Device will provide a web interface via http://[ip]:[port]/

boolean ENABLE_OTA = true;    // this will allow you to load firmware to the device over WiFi (see OTA for ESP8266)
String OTA_Password = "";     // Set an OTA password here -- leave blank if you don't want to be prompted for password
    