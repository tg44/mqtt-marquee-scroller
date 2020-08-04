
#include "MqttClient.h"

MqttClient::MqttClient() {}

void MqttClient::connect(String url, int port, String topic, String deviceId) {
    url.trim();
    topic.trim();
    deviceId.trim();
    client = PubSubClient(wifiClient);
    deviceId += "-" + String(random(0xffff), HEX);
    Serial.println("MQTT with domain as " + deviceId);
    client.setServer(url.c_str(), port);
    int i = 0;
    while (!client.connected() && i < 5) {
        delay(20);
        if (!client.connect(deviceId.c_str())) {
            i++;
            Serial.println("MQTT connection error! on " + url + ":" + String(port) + " state=" + String(client.state()));
            delay(2000);
        } else {
            std::function<void(char*, uint8_t*, unsigned int)> f = std::bind(&MqttClient::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            client.setCallback(f);
            client.subscribe(topic.c_str());
            connected = true;
            Serial.println("MQTT CONNECTED!!! topic: " + topic);
        }
    }
}

void MqttClient::callback(char* topic, uint8_t* payload, unsigned int length) {
  const size_t bufferSize = 710;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("MQTT json parse error!");
    return;
  }
  String newMsg = String((const char*)root["message"]);
  msg += " " + newMsg;
  Serial.println("MQTT json parse finished! " + newMsg);
}

void MqttClient::loop() {
    if(connected){
        if(!client.loop()) {
            Serial.println("MQTT DC :(");
        }
    }
}

String MqttClient::getMessage() {
    loop();
    String ret = msg;
    msg = "";
    return ret;
}