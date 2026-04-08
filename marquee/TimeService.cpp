// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TimeService.h"
#include <ArduinoJson.h>

TimeService::TimeService() {}

void TimeService::updateTime(int fallbackOffsetSeconds){
  if(!started) {
      timeClient.begin();
      started = true;
  }
  Serial.println("==== update time ===");

  if (!updateOffsetFromApi()) {
    if (!apiSuccess) {
      offset = fallbackOffsetSeconds;
      Serial.println("Using fallback offset: " + String(offset) + "s");
    } else {
      Serial.println("API failed, keeping last known offset: " + String(offset) + "s");
    }
  }

  timeClient.setTimeOffset(offset);
  timeClient.forceUpdate();
}

bool TimeService::updateOffsetFromApi() {
  WiFiClient client;
  Serial.println("Getting timezone from ip-api.com");

  if (!client.connect("ip-api.com", 80)) {
    Serial.println("API connection failed");
    return false;
  }

  client.println("GET /json/?fields=status,timezone,offset HTTP/1.1");
  client.println("Host: ip-api.com");
  client.println("User-Agent: ArduinoWiFi/1.1");
  client.println("Connection: close");
  client.println();

  unsigned long timeout = millis();
  while (client.connected() && !client.available()) {
    if (millis() - timeout > 5000) {
      Serial.println("API timeout");
      client.stop();
      return false;
    }
    delay(10);
  }

  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    client.stop();
    return false;
  }

  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    client.stop();
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, client);
  client.stop();

  if (error) {
    Serial.println(F("Timezone parsing failed!"));
    return false;
  }

  const char* apiStatus = doc["status"];
  if (!apiStatus || strcmp(apiStatus, "success") != 0) {
    Serial.println(F("API returned failure status"));
    return false;
  }

  offset = (int)doc["offset"];
  const char* tz = doc["timezone"];
  Serial.println("Timezone: " + String(tz ? tz : "unknown"));
  Serial.println("API offset: " + String(offset) + "s");

  apiSuccess = true;
  return true;
}

int TimeService::hour() {
    return timeClient.getHours();
}

int TimeService::minute() {
    return timeClient.getMinutes();
}

int TimeService::second() {
    return timeClient.getSeconds();
}

int TimeService::get12hHourFormat() {
    int h = hour();
    if (h == 12) {
        return h;
    } else {
        return h % 12;
    }
}

long TimeService::getEpoch() {
  return timeClient.getEpochTime();
}

String TimeService::timeToConsole() {
    return timeClient.getFormattedTime();
}

bool TimeService::isPM() {
    return timeClient.getHours() > 11;
}

String TimeService::getAmPm() {
  String ampmValue = "AM";
  if (isPM()) {
    ampmValue = "PM";
  }
  return ampmValue;
}

String TimeService::zeroPad(int number) {
  if (number < 10) {
    return "0" + String(number);
  } else {
    return String(number);
  }
}
