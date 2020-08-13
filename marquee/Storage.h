// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include "Arduino.h"
#include "LittleFS.h"

/******************************************************************************
 * NOTE: The settings here are the default settings for the first loading.  
 * After loading you will manage changes to the settings via the Web Interface.  
 * If you want to change settings again in the settings.h, you will need to 
 * erase the file system on the Wemos or use the “Reset Settings” option in 
 * the Web Interface.
 ******************************************************************************/

class Storage {
  public:
    boolean IS_24HOUR = false; // 23:00 millitary 24 hour clock
    boolean IS_PM = true; // Show PM indicator on Clock when in AM/PM mode
    String wide_Clock_Style = "1";  //1="hh:mm Temp", 2="hh:mm:ss", 3="hh:mm"
    boolean IS_BASIC_AUTH = false;  // Use Basic Authorization for Configuration security on Web Interface
    String www_username = "admin";  // User account for the Web Interface
    String www_password = "password";  // Password for the Web Interface
    int minutesBetweenScrolling = 1; // Time in minutes between scrolling data (default 1 minutes and max is 10)
    int displayScrollSpeed = 25; // In milliseconds -- Configurable by the web UI (slow = 35, normal = 25, fast = 15, very fast = 5)
    int displayIntensity = 1;  //(This can be set from 0 - 15)
    boolean flashOnSeconds = true; // when true the : character in the time will flash on and off as a seconds indicator
    /* set ledRotation for LED Display panels (3 is default)
    0: no rotation
    1: 90 degrees clockwise
    2: 180 degrees
    3: 90 degrees counter clockwise (default)
    */
    int ledRotation = 3;
    String timeDisplayTurnsOn = "06:30";  // 24 Hour Format HH:MM -- Leave blank for always on. (ie 05:30)
    String timeDisplayTurnsOff = "23:00"; // 24 Hour Format HH:MM -- Leave blank for always on. Both must be set to work.
    /*
    MQTT settings
    */
    boolean ENABLE_MQTT = false;
    String mqttUrl = "";
    int mqttPort = 1883;
    String mqttTopic = ""; 
    String mqttDeviceId = "marquee-scroller";

    void serialize();
    void reload();
    boolean resetToDefault();
    Storage();

};