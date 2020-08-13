// (c) 2020 Gergo Torcsvari
// This code is licensed under MIT license (see LICENSE.txt for details)

#include "TimeService.h"

/* Useful Constants */
#define MANY_YEARS 1500000000
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

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
      return numberOfHours(lastEpochFromApi + epoch + offset);
    }
}

int TimeService::minute() {
    long epoch = getEpoch();
    if(epoch > MANY_YEARS) {
      return timeClient.getMinutes();
    } else {
      return numberOfMinutes(lastEpochFromApi + epoch + offset);
    }
}

int TimeService::second() {
    long epoch = getEpoch();
    if(epoch > MANY_YEARS) {
      return timeClient.getSeconds();
    } else {
      return numberOfSeconds(lastEpochFromApi + epoch + offset);
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