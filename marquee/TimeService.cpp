#include "TimeService.h"

#define MANY_YEARS 1500000000

TimeService::TimeService() {}

void TimeService::updateTime(){
  if(!started) {
      timeClient.begin();
      started = true;
  }
  Serial.println("==== update time ===");
  updateOffsetData();
  timeClient.setTimeOffset(offset);
  timeClient.forceUpdate();
}

int TimeService::hour() {
    long epoch = getEpoch();
    if(epoch > MANY_YEARS) {
      return timeClient.getHours();
    } else {
      return (((lastEpochFromApi + epoch + offset) % 86400L) / 3600);
    }
}

int TimeService::minute() {
    long epoch = getEpoch();
    if(epoch > MANY_YEARS) {
      return timeClient.getMinutes();
    } else {
      return (((lastEpochFromApi + epoch + offset) % 3600) / 60);
    }
}

int TimeService::second() {
    long epoch = getEpoch();
    if(epoch > MANY_YEARS) {
      return timeClient.getSeconds();
    } else {
      return ((lastEpochFromApi + epoch + offset) % 60);
    }
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
  return timeClient.getEpochTime() - offset; //bcs some reason they ruined epoch with the offset...
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

void TimeService::updateOffsetData()
{
  WiFiClient client;
  String apiGetData = "GET /api/ip HTTP/1.1";
  Serial.println("Getting Time Data");
  Serial.println(apiGetData);
  String result = "";
  if (client.connect(servername, 80)) {  //starts client connection, checks for connection
    client.println(apiGetData);
    client.println("Host: " + servername);
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("connection for time data failed"); //error message if no client connect
    Serial.println();
    return;
  }

  while (client.connected() && !client.available()) delay(1); //waits for data

  Serial.println("Waiting for data");

  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  Serial.println("Response Header: " + String(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  const size_t capacity = 1024;
  DynamicJsonBuffer jsonBuffer(capacity);
  JsonObject& root = jsonBuffer.parseObject(client);
  if (!root.success()) {
    Serial.println(F("Time Data Parsing failed!"));
    return;
  }

  client.stop(); //stop client
  
  int rawOffset = (int)root["raw_offset"];
  int dstOffset = (int)root["dst_offset"];
  bool isDst = (bool)root["dst"];
  unsigned long unixtime  = (long)root["unixtime"];
  Serial.println("unixtime: " + String(unixtime));
  Serial.println("isDst: " + String(isDst));
  Serial.println("dstOffset: " + String(dstOffset));
  Serial.println("rawOffset: " + String(rawOffset));

  offset = isDst ? rawOffset + dstOffset : rawOffset;
  lastEpochFromApi = unixtime;
}