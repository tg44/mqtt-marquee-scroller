// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "MqttClient.h"

MqttClient::MqttClient(): client(wifiClient) {}

void MqttClient::connect(String url, int port, String _topic, String _faceTopic, String deviceId) {
    topic = _topic;
    faceTopic = _faceTopic;
    url.trim();
    topic.trim();
    deviceId.trim();
    deviceId += "-" + String(random(0xffff), HEX);
    mqttDeviceId = deviceId;
    Serial.println("MQTT with domain as " + mqttDeviceId);
    client.setServer(url.c_str(), port);
    client.setSocketTimeout(60);
    int i = 0;
    while (!client.connected() && i < 5) {
        delay(20);
        if (!client.connect(mqttDeviceId.c_str())) {
            i++;
            Serial.println("MQTT connection error! on " + url + ":" + String(port) + " state=" + String(client.state()));
            delay(2000);
        } else {
            std::function<void(char*, uint8_t*, unsigned int)> f = std::bind(&MqttClient::callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            client.setCallback(f);
            client.subscribe(topic.c_str());
            client.subscribe(faceTopic.c_str());
            connected = true;
            Serial.println("MQTT CONNECTED!!! topic: " + topic);
        }
    }
}

void MqttClient::callback(char* in_topic, uint8_t* payload, unsigned int length) {
  const size_t bufferSize = 710;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (!root.success()) {
    Serial.println("MQTT json parse error!");
    return;
  }
  if (strcmp(in_topic,topic.c_str())==0){
    String newMsg = String((const char*)root["message"]);
    msg += " " + newMsg;
    Serial.println("MQTT msg json parse finished! " + newMsg);
    return;
  }
  if (strcmp(in_topic,faceTopic.c_str())==0){
    for(byte i=0; i<8; i++){
       panelFace[i].faceType = (byte)root["panels"][i]["t"];
       panelFace[i].percent = (byte)root["panels"][i]["p"];
       String str = root["panels"][i]["c"].as<String>();;
       panelFace[i].character = str.length() > 0 ? str.charAt(0) : ' ';
    }
    faceModified = true;
    Serial.println("MQTT face json parse finished!");
    return;
  }
  
}

void MqttClient::loop() {
    if(connected){
        if(!client.loop()) {
            Serial.println("MQTT DC :(");
        }
    }
}

String MqttClient::getMessage() {
    if(!client.connected()){
        if(client.connect(mqttDeviceId.c_str())) {
            client.subscribe(topic.c_str());
            client.subscribe(faceTopic.c_str());
            Serial.println("MQTT Reconnected");
        }
    }
    loop();
    String ret = msg;
    msg = "";
    return ret;
}