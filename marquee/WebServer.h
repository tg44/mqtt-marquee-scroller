// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include "Storage.h"
#include "StatusLed.h"
#include "Display.h"
#include "TimeService.h"
#include "MqttClient.h"

class WebServer{

  public:
    WebServer(int prt, Storage& st, StatusLed& proce, String ver, Display& disp, TimeService& tc, MqttClient& mqtt);
    void init();
    void handleClient();
    int8_t getWifiQuality();

  private:
    ESP8266WebServer server;
    ESP8266HTTPUpdateServer serverUpdater;
    Storage& s;
    int port;
    StatusLed& processing;
    String version;
    Display& display;
    TimeService& timeClient;
    MqttClient& mqttClient;

    // SPA
    void handleRoot();

    // JSON API
    void handleApiGetSettings();
    void handleApiPostSettings();
    void handleApiGetMqtt();
    void handleApiPostMqtt();
    void handleApiGetStatus();
    void handleApiGetDashboard();
    void handleApiDisplayToggle();
    void handleApiGetDisplayPixels();
    void handleApiPostBrightness();
    void handleApiPostFaces();
    void handleApiPostRaw();
    void handleApiNtpRefresh();
    void handleApiReset();
    void handleApiForgetWifi();

    // Utils
    boolean athentication();
    void sendJson(int code, JsonDocument& doc);
    void sendOk();
};
