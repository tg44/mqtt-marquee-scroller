// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "Storage.h"

Storage::Storage(){
    LittleFS.begin();
}

void Storage::reload() {
  if(!LittleFS.exists("/conf.txt")){
    Serial.println("Settings File does not yet exists.");
    serialize();
    return;
  } else {
    File fr = LittleFS.open("/conf.txt", "r");
    String line;
    while (fr.available()) {
        line = fr.readStringUntil('\n');
        if (line.indexOf("isFlash=") >= 0) {
            flashOnSeconds = line.substring(line.lastIndexOf("isFlash=") + 8).toInt();
            Serial.println("flashOnSeconds=" + String(flashOnSeconds));
        }
        if (line.indexOf("is24hour=") >= 0) {
            IS_24HOUR = line.substring(line.lastIndexOf("is24hour=") + 9).toInt();
            Serial.println("IS_24HOUR=" + String(IS_24HOUR));
        }
        if (line.indexOf("isPM=") >= 0) {
            IS_PM = line.substring(line.lastIndexOf("isPM=") + 5).toInt();
            Serial.println("IS_PM=" + String(IS_PM));
        }
        if (line.indexOf("wideclockformat=") >= 0) {
            wide_Clock_Style = line.substring(line.lastIndexOf("wideclockformat=") + 16);
            wide_Clock_Style.trim();
            Serial.println("Wide_Clock_Style=" + wide_Clock_Style);
        }
        if (line.indexOf("minutesBetweenScrolling=") >= 0) {
            minutesBetweenScrolling = line.substring(line.lastIndexOf("minutesBetweenScrolling=") + 24).toInt();
            Serial.println("minutesBetweenScrolling=" + String(minutesBetweenScrolling));
        }
        if (line.indexOf("timeDisplayTurnsOn=") >= 0) {
            timeDisplayTurnsOn = line.substring(line.lastIndexOf("timeDisplayTurnsOn=") + 19);
            timeDisplayTurnsOn.trim();
            Serial.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
        }
        if (line.indexOf("timeDisplayTurnsOff=") >= 0) {
            timeDisplayTurnsOff = line.substring(line.lastIndexOf("timeDisplayTurnsOff=") + 20);
            timeDisplayTurnsOff.trim();
            Serial.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
        }
        if (line.indexOf("ledIntensity=") >= 0) {
            displayIntensity = line.substring(line.lastIndexOf("ledIntensity=") + 13).toInt();
            Serial.println("displayIntensity=" + String(displayIntensity));
        }
        if (line.indexOf("scrollSpeed=") >= 0) {
            displayScrollSpeed = line.substring(line.lastIndexOf("scrollSpeed=") + 12).toInt();
            Serial.println("displayScrollSpeed=" + String(displayScrollSpeed));
        }
        if (line.indexOf("www_username=") >= 0) {
            www_username = line.substring(line.lastIndexOf("www_username=") + 13);
            www_username.trim();
            Serial.println("www_username=" + String(www_username));
        }
        if (line.indexOf("www_password=") >= 0) {
            www_password = line.substring(line.lastIndexOf("www_password=") + 13);
            www_password.trim();
            Serial.println("www_password=" + String(www_password));
        }
        if (line.indexOf("IS_BASIC_AUTH=") >= 0) {
            IS_BASIC_AUTH = line.substring(line.lastIndexOf("IS_BASIC_AUTH=") + 14).toInt();
            Serial.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
        }
        if (line.indexOf("ENABLE_MQTT=") >= 0) {
            ENABLE_MQTT = line.substring(line.lastIndexOf("ENABLE_MQTT=") + 12).toInt();
            Serial.println("ENABLE_MQTT=" + String(ENABLE_MQTT));
        }
        if (line.indexOf("mqttUrl=") >= 0) {
            mqttUrl = line.substring(line.lastIndexOf("mqttUrl=") + 8);
            Serial.println("mqttUrl=" + String(mqttUrl));
        }
        if (line.indexOf("mqttPort=") >= 0) {
            mqttPort = line.substring(line.lastIndexOf("mqttPort=") + 9).toInt();
            Serial.println("mqttPort=" + String(mqttPort));
        }
        if (line.indexOf("mqttTopic=") >= 0) {
            mqttTopic = line.substring(line.lastIndexOf("mqttTopic=") + 10);
            Serial.println("mqttTopic=" + String(mqttTopic));
        }
        if (line.indexOf("mqttDeviceId=") >= 0) {
            mqttDeviceId = line.substring(line.lastIndexOf("mqttDeviceId=") + 13);
            Serial.println("mqttDeviceId=" + String(mqttDeviceId));
        }
    }
    fr.close();
  }
}

void Storage::serialize() {
  File f = LittleFS.open("/conf.txt", "w");
  if (!f) {
    Serial.println("File open failed!");
  } else {
    Serial.println("Saving settings now...");
    f.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
    f.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
    f.println("ledIntensity=" + String(displayIntensity));
    f.println("scrollSpeed=" + String(displayScrollSpeed));
    f.println("isFlash=" + String(flashOnSeconds));
    f.println("is24hour=" + String(IS_24HOUR));
    f.println("isPM=" + String(IS_PM));
    f.println("wideclockformat=" + wide_Clock_Style);
    f.println("minutesBetweenScrolling=" + String(minutesBetweenScrolling));
    f.println("www_username=" + String(www_username));
    f.println("www_password=" + String(www_password));
    f.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
    f.println("ENABLE_MQTT=" + String(ENABLE_MQTT));
    f.println("mqttDeviceId=" + mqttDeviceId);
    f.println("mqttUrl=" + mqttUrl);
    f.println("mqttPort=" + String(mqttPort));
    f.println("mqttTopic=" + mqttTopic);
  }
  f.close();
  reload();
}

boolean Storage::resetToDefault() {
  return LittleFS.remove("/conf.txt");
}