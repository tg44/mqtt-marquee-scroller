// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

/**********************************************
  Edit Settings.h for HW
  Edit Storage.h for custom default values
***********************************************/

#include "Settings.h"
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include "MqttClient.h"
#include "TimeService.h"
#include "Storage.h"
#include "StatusLed.h"
#include "WebServer.h"

//declairing prototypes
void configModeCallback (WiFiManager *myWiFiManager);
void smallLoop();
void otaInit();

//global vars
String lastMinute = "xx";
int displayRefreshCount = 1;
String hostname = HOSTNAME + String(ESP.getChipId(), HEX);
//global utility classes
TimeService timeClient = TimeService();
MqttClient mqttClient = MqttClient();
StatusLed processing(STATUS_LED);
Storage s;
Display display(pinCS, numberOfHorizontalDisplays, processing, timeClient, smallLoop);
WebServer server(WEBSERVER_PORT, s, processing, String(VERSION), display, timeClient);

void setup() {
  Serial.begin(115200);
  delay(10);
  processing.init();
  //New Line to clear from start garbage
  Serial.println();
  //load storage data
  s.reload();
  //init led matrix
  display.init(s.ledRotation);
  display.welcome(s.displayIntensity);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect((const char *)hostname.c_str())) {
    delay(3000);
    WiFi.disconnect(true);
    ESP.reset();
    delay(5000);
  }

  // print the received signal strength:
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(server.getWifiQuality());
  Serial.println("%");

  if (ENABLE_OTA) {
    otaInit();
  }

  if (WEBSERVER_ENABLED) {
    server.init();
    display.scrollMessage(" v" + String(VERSION) + "  IP: " + WiFi.localIP().toString() + "  ", s.displayScrollSpeed);
  } else {
    Serial.println("Web Interface is Disabled");
    display.scrollMessage("Web Interface is Disabled", s.displayScrollSpeed);
  }

  timeClient.updateTime();

  if(s.ENABLE_MQTT){
    Serial.println("MQTT connect");
    display.scrollMessage("MQTT...", s.displayScrollSpeed);
    mqttClient.connect(s.mqttUrl, s.mqttPort, s.mqttTopic, s.mqttFaceTopic, s.mqttDeviceId);
    display.scrollMessage("...connected", s.displayScrollSpeed);
  }
  processing.flashLED(5, 80);
}

void loop() {
  checkDisplay(); // this will see if we need to turn it on or off for night mode.

  if (lastMinute != timeClient.zeroPad(timeClient.minute())) {
    lastMinute = timeClient.zeroPad(timeClient.minute());
    if(timeClient.minute() % 15 == 0){
      timeClient.updateTime();
    }
    if (display.isOn()) {
      display.enableDisplay(true);
    }
    display.clear();
    displayRefreshCount --;
    // Check to see if we need to Scroll some Data
    if (displayRefreshCount <= 0) {
      displayRefreshCount = s.minutesBetweenScrolling;
      String msg;
      msg += " ";
      if (s.ENABLE_MQTT) {
        msg += mqttClient.getMessage();
      }
      if(msg.length() > 1) {
        display.scrollMessage(msg, s.displayScrollSpeed);
      }
    }
  }

  showTimePlate();
  smallLoop();
}

void showTimePlate() {
  if(mqttClient.faceModified){
    display.clear();
    mqttClient.faceModified = false;
  }
  boolean empty = true;
  for(byte i=0; i<8; i++){
    if(mqttClient.panelFace[i].faceType!=0){
      empty = false;
      break;
    }
  }
  if(empty){
    if(numberOfHorizontalDisplays >= 8){
      display.showTime(1, true, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
    } else if(numberOfHorizontalDisplays >= 6) {
      display.showTime(0, true, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
    } else if(numberOfHorizontalDisplays >= 4) {
      display.showTime(0, false, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
    }
    return;
  }
  for(byte i=0; i<8; i++){
    if(mqttClient.panelFace[i].faceType==0){
      continue;
    } else if(mqttClient.panelFace[i].faceType==1){
      if(i+4 >= numberOfHorizontalDisplays){
        display.showTime(i, false, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
        i+=3;
      } else if(mqttClient.panelFace[i+4].faceType != 0) {
        display.showTime(i, false, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
        i+=3;
      } else if(i+6 <= numberOfHorizontalDisplays) {
        if(mqttClient.panelFace[i+5].faceType == 0 && mqttClient.panelFace[i+6].faceType == 0) {
          display.showTime(i, true, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
          i+=5;
        } else {
          display.showTime(i, false, s.IS_24HOUR, s.IS_PM, s.flashOnSeconds);
          i+=3;
        }
      }
    } else if(mqttClient.panelFace[i].faceType==2) {
      if(i+1 >= numberOfHorizontalDisplays || mqttClient.panelFace[i+1].faceType != 0){
        display.showSmallBinaryTime(i);
      } else {
        display.showLargeBinaryTime(i);
        i++;
      }
    } else if(mqttClient.panelFace[i].faceType==3) {
      display.showPercentage(i, mqttClient.panelFace[i].percent, s.flashOnSeconds);
    } else if(mqttClient.panelFace[i].faceType==4) {
      display.showCharacter(i, mqttClient.panelFace[i].character);
    } else {
      continue;
    }
  }
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println("Wifi Manager");
  Serial.println("Please connect to AP");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("To setup Wifi Configuration");
  display.scrollMessage("Please Connect to AP: " + String(myWiFiManager->getConfigPortalSSID()), s.displayScrollSpeed);
  display.centerPrint("wifi");
}

void smallLoop(){
  if (WEBSERVER_ENABLED) {
      server.handleClient();
    }
    if (ENABLE_OTA) {
      ArduinoOTA.handle();
    }
    if (s.ENABLE_MQTT) {
      mqttClient.loop();
    }
}

void checkDisplay() {
  if (s.timeDisplayTurnsOn == "" || s.timeDisplayTurnsOff == "") {
    return; // nothing to do
  }
  String currentTime = timeClient.zeroPad(timeClient.hour()) + ":" + timeClient.zeroPad(timeClient.minute());

  if (currentTime == s.timeDisplayTurnsOn && !display.isOn()) {
    Serial.println("Time to turn display on: " + currentTime);
    processing.flashLED(1, 500);
    display.enableDisplay(true);
  }

  if (currentTime == s.timeDisplayTurnsOff && display.isOn()) {
    Serial.println("Time to turn display off: " + currentTime);
    processing.flashLED(2, 500);
    display.enableDisplay(false);
  }
}

void otaInit() {
  ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.setHostname((const char *)hostname.c_str());
    if (OTA_Password != "") {
      ArduinoOTA.setPassword(((const char *)OTA_Password.c_str()));
    }
    ArduinoOTA.begin();
}
