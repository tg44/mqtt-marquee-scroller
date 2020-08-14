// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h> // --> https://github.com/tzapu/WiFiManager
#include "Storage.h"
#include "StatusLed.h"
#include "WebserverHelpers.h"
#include "Display.h"
#include "TimeService.h"

class WebServer{

  public:
    WebServer(int prt, Storage& st, StatusLed& proce, String ver, Display& disp, TimeService& tc);
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
    //handlers
    void handleConfigureMqtt();
    void handleSaveMqtt();
    void handleLocations();
    void handleDisplay();
    void handleConfigure();
    void handleForgetWifi();
    void handleSystemReset();
    void handlePull();

    //utils
    boolean athentication();
    void sendHeader();
    void redirectHome();
    void displayMessage(String message);
    String decodeHtmlString(String msg);
    String createFooter(int8_t rssi, String version);

};