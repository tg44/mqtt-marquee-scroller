// (c) 2018 David Payne
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "WebServer.h"
#include "webapp.h"

WebServer::WebServer(int prt, Storage& st, StatusLed& proc, String appver, Display& disp, TimeService& tc):
  server(prt), s(st), port(prt), processing(proc), version(appver), display(disp), timeClient(tc) {
}

void WebServer::handleClient(){
    server.handleClient();
}

void WebServer::init(){
    // SPA
    server.on("/", HTTP_GET, std::bind(&WebServer::handleRoot, this));

    // JSON API
    server.on("/api/settings", HTTP_GET, std::bind(&WebServer::handleApiGetSettings, this));
    server.on("/api/settings", HTTP_POST, std::bind(&WebServer::handleApiPostSettings, this));
    server.on("/api/mqtt", HTTP_GET, std::bind(&WebServer::handleApiGetMqtt, this));
    server.on("/api/mqtt", HTTP_POST, std::bind(&WebServer::handleApiPostMqtt, this));
    server.on("/api/status", HTTP_GET, std::bind(&WebServer::handleApiGetStatus, this));
    server.on("/api/display/toggle", HTTP_POST, std::bind(&WebServer::handleApiDisplayToggle, this));
    server.on("/api/ntp/refresh", HTTP_POST, std::bind(&WebServer::handleApiNtpRefresh, this));
    server.on("/api/reset", HTTP_POST, std::bind(&WebServer::handleApiReset, this));
    server.on("/api/forgetwifi", HTTP_POST, std::bind(&WebServer::handleApiForgetWifi, this));

    // Catch-all: serve SPA for any unknown path
    server.onNotFound(std::bind(&WebServer::handleRoot, this));

    // OTA firmware update
    serverUpdater.setup(&server, "/update", s.www_username, s.www_password);

    server.begin();
    Serial.println("Server started");
    String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(port) + "/";
    Serial.println("Use this URL : " + webAddress);
}

// --- SPA ---

void WebServer::handleRoot() {
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Cache-Control", "max-age=86400");
  server.send_P(200, "text/html", (const char*)WEBAPP_HTML_GZ, WEBAPP_HTML_GZ_LEN);
}

// --- Utilities ---

boolean WebServer::athentication() {
  if (s.IS_BASIC_AUTH) {
    return server.authenticate(s.www_username.c_str(), s.www_password.c_str());
  }
  return true;
}

void WebServer::sendJson(int code, JsonDocument& doc) {
  String output;
  serializeJson(doc, output);
  server.send(code, "application/json", output);
}

void WebServer::sendOk() {
  server.send(200, "application/json", "{\"ok\":true}");
}

// --- JSON API ---

void WebServer::handleApiGetSettings() {
  if (!athentication()) return server.requestAuthentication();

  JsonDocument doc;
  doc["utcOffsetHours"] = s.utcOffsetSeconds / 3600;
  doc["is24Hour"] = s.IS_24HOUR;
  doc["isPM"] = s.IS_PM;
  doc["flashSeconds"] = s.flashOnSeconds;
  doc["startTime"] = s.timeDisplayTurnsOn;
  doc["endTime"] = s.timeDisplayTurnsOff;
  doc["brightness"] = s.displayIntensity;
  doc["scrollSpeed"] = s.displayScrollSpeed;
  doc["refreshMinutes"] = s.minutesBetweenScrolling;
  doc["basicAuth"] = s.IS_BASIC_AUTH;
  doc["username"] = s.www_username;
  doc["password"] = s.www_password;
  sendJson(200, doc);
}

void WebServer::handleApiPostSettings() {
  if (!athentication()) return server.requestAuthentication();

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }

  s.utcOffsetSeconds = (int)doc["utcOffsetHours"] * 3600;
  s.IS_24HOUR = doc["is24Hour"] | false;
  s.IS_PM = doc["isPM"] | false;
  s.flashOnSeconds = doc["flashSeconds"] | false;
  s.timeDisplayTurnsOn = doc["startTime"] | "06:30";
  s.timeDisplayTurnsOff = doc["endTime"] | "23:00";
  s.displayIntensity = doc["brightness"] | 1;
  s.displayScrollSpeed = doc["scrollSpeed"] | 25;
  s.minutesBetweenScrolling = doc["refreshMinutes"] | 1;
  s.IS_BASIC_AUTH = doc["basicAuth"] | false;
  s.www_username = doc["username"] | "admin";
  s.www_password = doc["password"] | "password";

  display.clear();
  s.serialize();
  timeClient.updateTime(s.utcOffsetSeconds);
  display.setBrightness(s.displayIntensity);

  sendOk();
}

void WebServer::handleApiGetMqtt() {
  if (!athentication()) return server.requestAuthentication();

  JsonDocument doc;
  doc["enabled"] = s.ENABLE_MQTT;
  doc["deviceId"] = s.mqttDeviceId;
  doc["url"] = s.mqttUrl;
  doc["port"] = s.mqttPort;
  doc["topic"] = s.mqttTopic;
  doc["faceTopic"] = s.mqttFaceTopic;
  sendJson(200, doc);
}

void WebServer::handleApiPostMqtt() {
  if (!athentication()) return server.requestAuthentication();

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    server.send(400, "application/json", "{\"error\":\"invalid json\"}");
    return;
  }

  s.ENABLE_MQTT = doc["enabled"] | false;
  s.mqttDeviceId = doc["deviceId"] | "marquee-scroller";
  s.mqttUrl = doc["url"] | "";
  s.mqttPort = doc["port"] | 1883;
  s.mqttTopic = doc["topic"] | "";
  s.mqttFaceTopic = doc["faceTopic"] | "tele/marquee-scroller/face";
  s.serialize();

  server.send(200, "application/json", "{\"ok\":true,\"restarting\":true}");
  delay(100);
  ESP.restart();
}

void WebServer::handleApiGetStatus() {
  JsonDocument doc;
  doc["version"] = version;
  doc["wifiSignal"] = getWifiQuality();
  doc["displayOn"] = display.isOn();
  sendJson(200, doc);
}

void WebServer::handleApiDisplayToggle() {
  if (!athentication()) return server.requestAuthentication();

  display.switchState();
  JsonDocument doc;
  doc["displayOn"] = display.isOn();
  sendJson(200, doc);
}

void WebServer::handleApiNtpRefresh() {
  if (!athentication()) return server.requestAuthentication();

  timeClient.updateTime(s.utcOffsetSeconds);
  sendOk();
}

void WebServer::handleApiReset() {
  if (!athentication()) return server.requestAuthentication();

  if (s.resetToDefault()) {
    server.send(200, "application/json", "{\"ok\":true,\"restarting\":true}");
    delay(100);
    ESP.restart();
  }
  server.send(500, "application/json", "{\"error\":\"reset failed\"}");
}

void WebServer::handleApiForgetWifi() {
  if (!athentication()) return server.requestAuthentication();

  server.send(200, "application/json", "{\"ok\":true,\"restarting\":true}");
  delay(100);
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

int8_t WebServer::getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100) {
    return 0;
  } else if (dbm >= -50) {
    return 100;
  } else {
    return 2 * (dbm + 100);
  }
}
