// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "MqttClient.h"

const size_t MQTT_BUFFER_SIZE=2046;

MqttClient::MqttClient(): client(wifiClient) {}

void MqttClient::connect(String _url, int _port, String _topic, String _faceTopic, String _deviceId) {
    deviceId = _deviceId;
    topic = _topic;
    faceTopic = _faceTopic;
    url = _url;
    port = _port;
    url.trim();
    topic.trim();
    faceTopic.trim();
    deviceId.trim();
    mqttDeviceId = deviceId + "-" + String(random(0xffff), HEX);
    Serial.println("MQTT with domain as " + mqttDeviceId);
    client.setServer(url.c_str(), port);
    client.setSocketTimeout(60);
    client.setKeepAlive(60);
    client.setBufferSize(MQTT_BUFFER_SIZE);
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
            Serial.println(topic);
            Serial.println(client.subscribe(topic.c_str(), 1));
            Serial.println(faceTopic);
            Serial.println(client.subscribe(faceTopic.c_str(), 1));
            connected = true;
            Serial.println("MQTT CONNECTED!!! topic: " + topic);
        }
    }
}

void MqttClient::callback(char* in_topic, uint8_t* payload, unsigned int length) {
  DynamicJsonBuffer jsonBuffer(MQTT_BUFFER_SIZE);
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
            Serial.println("MQTT dc! on " + url + ":" + String(port) + " state=" + String(client.state()));
            reconnect();
        }
    }
}

void MqttClient::reconnect() {
    if(!client.connected()){
        //mqttDeviceId = deviceId + "-" + String(random(0xffff), HEX);
        if(client.connect(mqttDeviceId.c_str())) {
            client.subscribe(topic.c_str());
            client.subscribe(faceTopic.c_str());
            Serial.println("MQTT Reconnected");
        }
        Serial.println(client.state());
    }
}

String MqttClient::getMessage() {
    loop();
    String ret = msg;
    msg = "";
    return ret;
}