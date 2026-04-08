// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

#pragma once
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

struct PanelFace {
  // 0 - empty, 1 - clock, 2 - binary, 3 - percent, 4 - char, 5 - raw
  byte faceType = 0;
  byte percent = 0;
  char character = ' ';
  byte rawPixels[8] = {0}; // 8 rows, each byte = 8 cols (bit per pixel)
};

class MqttClient
{
  public:
    MqttClient();
    void connect(String url, int port, String topic, String faceTopic, String deviceId);
    String getMessage();
    String getLastMessage();
    String getAccumulatedMessage();
    boolean isConnected();
    void loop();
    PanelFace panelFace[8];
    boolean faceModified = false;

  private:
    void callback(char* topic, byte* payload, unsigned int length);
    void reconnect();
    PubSubClient client;
    WiFiClient wifiClient;
    String msg = "";
    String lastMsg = "";
    boolean connected = false;
    String url = "";
    int port = 0;
    String deviceId = "";
    String mqttDeviceId = "";
    String topic = "";
    String faceTopic = "";
};
