/** The MIT License (MIT)

  Copyright (c) 2018 David Payne

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/**********************************************
  Edit Settings.h for personalization
***********************************************/

#include "Settings.h"

#define VERSION "2.16"

#define HOSTNAME "CLOCK-"
#define CONFIG "/conf.txt"
#define BUZZER_PIN  D2

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)

//declairing prototypes
void configModeCallback (WiFiManager *myWiFiManager);
int8_t getWifiQuality();

// LED Settings
const int offset = 1;
int refresh = 0;
String message = "hello";
int spacer = 1;  // dots between letters
int width = 5 + spacer; // The font width is 5 pixels + spacer
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
String Wide_Clock_Style = "1";  //1="hh:mm Temp", 2="hh:mm:ss", 3="hh:mm"
float UtcOffset;  //time zone offsets that correspond with the CityID above (offset from GMT)

// Time
TimeService timeClient = TimeService();
String lastMinute = "xx";
int displayRefreshCount = 1;
long displayOffEpoch = 0;
boolean displayOn = true;

//MQTT client
MqttClient mqttClient = MqttClient();

ESP8266WebServer server(WEBSERVER_PORT);
ESP8266HTTPUpdateServer serverUpdater;

const int TIMEOUT = 500; // 500 = 1/2 second
int timeoutCount = 0;

// Change the externalLight to the pin you wish to use if other than the Built-in LED
int externalLight = LED_BUILTIN; // LED_BUILTIN is is the built in LED on the Wemos

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  //SPIFFS.remove(CONFIG);
  delay(10);

  // Initialize digital pin for LED
  pinMode(externalLight, OUTPUT);

  //New Line to clear from start garbage
  Serial.println();

  readCityIds();

  Serial.println("Number of LED Displays: " + String(numberOfHorizontalDisplays));
  // initialize dispaly
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness

  int maxPos = numberOfHorizontalDisplays * numberOfVerticalDisplays;
  for (int i = 0; i < maxPos; i++) {
    matrix.setRotation(i, ledRotation);
    matrix.setPosition(i, maxPos - i - 1, 0);
  }

  Serial.println("matrix created");
  matrix.fillScreen(LOW); // show black
  centerPrint("hello");

  tone(BUZZER_PIN, 415, 500);
  delay(500 * 1.3);
  tone(BUZZER_PIN, 466, 500);
  delay(500 * 1.3);
  tone(BUZZER_PIN, 370, 1000);
  delay(1000 * 1.3);
  noTone(BUZZER_PIN);

  for (int inx = 0; inx <= 15; inx++) {
    matrix.setIntensity(inx);
    delay(100);
  }
  for (int inx = 15; inx >= 0; inx--) {
    matrix.setIntensity(inx);
    delay(60);
  }
  delay(1000);
  matrix.setIntensity(displayIntensity);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setAPCallback(configModeCallback);

  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  if (!wifiManager.autoConnect((const char *)hostname.c_str())) {// new addition
    delay(3000);
    WiFi.disconnect(true);
    ESP.reset();
    delay(5000);
  }

  // print the received signal strength:
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(getWifiQuality());
  Serial.println("%");

  if (ENABLE_OTA) {
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.setHostname((const char *)hostname.c_str());
    if (OTA_Password != "") {
      ArduinoOTA.setPassword(((const char *)OTA_Password.c_str()));
    }
    ArduinoOTA.begin();
  }

  if (WEBSERVER_ENABLED) {
    server.on("/", handleConfigure);
    server.on("/pull", handlePull);
    server.on("/locations", handleLocations);
    server.on("/savewideclock", handleSaveWideClock);
    server.on("/systemreset", handleSystemReset);
    server.on("/forgetwifi", handleForgetWifi);
    server.on("/configure", handleConfigure);
    server.on("/configurewideclock", handleWideClockConfigure);
    server.on("/display", handleDisplay);
    server.on("/savemqtt", handleSaveMqtt);
    server.on("/configuremqtt", handleConfigureMqtt);
    server.onNotFound(redirectHome);
    serverUpdater.setup(&server, "/update", www_username, www_password);
    // Start the server
    server.begin();
    Serial.println("Server started");
    // Print the IP address
    String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
    Serial.println("Use this URL : " + webAddress);
    scrollMessage(" v" + String(VERSION) + "  IP: " + WiFi.localIP().toString() + "  ");
  } else {
    Serial.println("Web Interface is Disabled");
    scrollMessage("Web Interface is Disabled");
  }
  timeClient.updateTime();
  if(ENABLE_MQTT){
    Serial.println("MQTT connect");
    scrollMessage("MQTT...");
    mqttClient.connect(mqttUrl, mqttPort, mqttTopic, mqttDeviceId);
    scrollMessage("...connected");
  }

  flashLED(1, 500);
}

//************************************************************
// Main Looop
//************************************************************
void loop() {
  checkDisplay(); // this will see if we need to turn it on or off for night mode.

  if (lastMinute != timeClient.zeroPad(timeClient.minute())) {
    lastMinute = timeClient.zeroPad(timeClient.minute());
    if(timeClient.minute() % 15 == 0){
      timeClient.updateTime();
    }
    if (displayOn) {
      matrix.shutdown(false);
    }
    matrix.fillScreen(LOW); // show black
    displayRefreshCount --;
    // Check to see if we need to Scroll some Data
    if (displayRefreshCount <= 0) {
      displayRefreshCount = minutesBetweenScrolling;
      String msg;
      msg += " ";
      if (ENABLE_MQTT) {
        msg += mqttClient.getMessage();
      }
      if(msg.length() > 1) {
        scrollMessage(msg);
      }
    }
  }

  String currentTime = hourMinutes(false);

  if (numberOfHorizontalDisplays >= 8) {
    if (Wide_Clock_Style == "2") {
      currentTime += secondsIndicator(false) + timeClient.zeroPad(timeClient.second());
      matrix.fillScreen(LOW); // show black
    }
    if (Wide_Clock_Style == "3") {
      // No change this is normal clock display
    }
  }
  matrix.fillScreen(LOW);
  centerPrint(currentTime, true);

  if (WEBSERVER_ENABLED) {
    server.handleClient();
  }
  if (ENABLE_OTA) {
    ArduinoOTA.handle();
  }
  if (ENABLE_MQTT) {
    mqttClient.loop();  //we need to loop as much as we can
  }
}

String hourMinutes(boolean isRefresh) {
  if (IS_24HOUR) {
    return timeClient.hour() + secondsIndicator(isRefresh) + timeClient.zeroPad(timeClient.minute());
  } else {
    return timeClient.get12hHourFormat() + secondsIndicator(isRefresh) + timeClient.zeroPad(timeClient.minute());
  }
}

String secondsIndicator(boolean isRefresh) {
  String rtnValue = ":";
  if (isRefresh == false && (flashOnSeconds && (timeClient.second() % 2) == 0)) {
    rtnValue = " ";
  }
  return rtnValue;
}

boolean athentication() {
  if (IS_BASIC_AUTH) {
    return server.authenticate(www_username.c_str(), www_password.c_str());
  }
  return true; // Authentication not required
}

void handlePull() {
  timeClient.updateTime();
}

void handleSaveWideClock() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  if (numberOfHorizontalDisplays >= 8) {
    Wide_Clock_Style = server.arg("wideclockformat");
    writeCityIds();
    matrix.fillScreen(LOW); // show black
  }
  redirectHome();
}

void handleLocations() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  flashOnSeconds = server.hasArg("flashseconds");
  IS_24HOUR = server.hasArg("is24hour");
  IS_PM = server.hasArg("isPM");
  timeDisplayTurnsOn = decodeHtmlString(server.arg("startTime"));
  timeDisplayTurnsOff = decodeHtmlString(server.arg("endTime"));
  displayIntensity = server.arg("ledintensity").toInt();
  minutesBetweenScrolling = server.arg("refreshDisplay").toInt();
  displayScrollSpeed = server.arg("scrollspeed").toInt();
  IS_BASIC_AUTH = server.hasArg("isBasicAuth");
  www_username = server.arg("userid");
  www_password = server.arg("stationpassword");
  matrix.fillScreen(LOW); // show black
  writeCityIds();
  timeClient.updateTime(); // this will force a data pull for new weather
  redirectHome();
}

void handleSystemReset() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  Serial.println("Reset System Configuration");
  if (SPIFFS.remove(CONFIG)) {
    redirectHome();
    ESP.restart();
  }
}

void handleForgetWifi() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  redirectHome();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

void handleWideClockConfigure() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  if (numberOfHorizontalDisplays >= 8) {
    // Wide display options
    String form = FPSTR(WIDECLOCK_FORM);
    String clockOptions = "<option value='2'>HH:MM:SS</option><option value='3'>HH:MM</option>";
    clockOptions.replace(Wide_Clock_Style + "'", Wide_Clock_Style + "' selected");
    form.replace("%WIDECLOCKOPTIONS%", clockOptions);
    server.sendContent(form);
  }

  server.sendContent(createFooter(getWifiQuality(), VERSION));

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleConfigure() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(CHANGE_FORM1);
  
  String is24hourChecked = "";
  if (IS_24HOUR) {
    is24hourChecked = "checked='checked'";
  }
  form.replace("%IS_24HOUR_CHECKED%", is24hourChecked);
  
  server.sendContent(form);

  form = FPSTR(CHANGE_FORM2);
  String isPmChecked = "";
  if (IS_PM) {
    isPmChecked = "checked='checked'";
  }
  form.replace("%IS_PM_CHECKED%", isPmChecked);
  String isFlashSecondsChecked = "";
  if (flashOnSeconds) {
    isFlashSecondsChecked = "checked='checked'";
  }
  form.replace("%FLASHSECONDS%", isFlashSecondsChecked);
  form.replace("%STARTTIME%", timeDisplayTurnsOn);
  form.replace("%ENDTIME%", timeDisplayTurnsOff);
  form.replace("%INTENSITYOPTIONS%", String(displayIntensity));
  String dSpeed = String(displayScrollSpeed);
  String scrollOptions = "<option value='35'>Slow</option><option value='25'>Normal</option><option value='15'>Fast</option><option value='10'>Very Fast</option>";
  scrollOptions.replace(dSpeed + "'", dSpeed + "' selected" );
  form.replace("%SCROLLOPTIONS%", scrollOptions);
  form.replace("%REFRESH_DISPLAY%", String(minutesBetweenScrolling));

  server.sendContent(form); //Send another chunk of the form

  form = FPSTR(CHANGE_FORM3);
  String isUseSecurityChecked = "";
  if (IS_BASIC_AUTH) {
    isUseSecurityChecked = "checked='checked'";
  }
  form.replace("%IS_BASICAUTH_CHECKED%", isUseSecurityChecked);
  form.replace("%USERID%", String(www_username));
  form.replace("%STATIONPASSWORD%", String(www_password));

  server.sendContent(form); // Send the second chunk of Data

  server.sendContent(createFooter(getWifiQuality(), VERSION));

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void handleDisplay() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  enableDisplay(!displayOn);
  String state = "OFF";
  if (displayOn) {
    state = "ON";
  }
  displayMessage("Display is now " + state);
}

//***********************************************************************
void displayMessage(String message) {
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();
  server.sendContent(message);
  server.sendContent(createFooter(getWifiQuality(), VERSION));
  server.sendContent("");
  server.client().stop();

  digitalWrite(externalLight, HIGH);
}

void handleSaveMqtt() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  ENABLE_MQTT = server.hasArg("mqttenable");
  mqttDeviceId = server.arg("mqttDeviceName");
  mqttUrl = server.arg("mqttUrl");
  mqttPort = server.arg("mqttPort").toInt();
  mqttTopic = server.arg("mqttTopic");
  writeCityIds();
  redirectHome();
  ESP.restart();
}


void handleConfigureMqtt() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  digitalWrite(externalLight, LOW);

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(MQTT_FORM);
  String isMqttChecked = "";
  if (ENABLE_MQTT) {
    isMqttChecked = "checked='checked'";
  }
  form.replace("%MQTTCHECKED%", isMqttChecked);
  form.replace("%MQTTDEVICE%", mqttDeviceId);
  form.replace("%MQTTURL%", mqttUrl);
  form.replace("%MQTTPORT%", String(mqttPort));
  form.replace("%MQTTTOPIC%", mqttTopic);
  server.sendContent(form);

  server.sendContent(createFooter(getWifiQuality(), VERSION));

  server.sendContent("");
  server.client().stop();
  digitalWrite(externalLight, HIGH);
}

void redirectHome() {
  // Send them back to the Root Directory
  server.sendHeader("Location", String("/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");
  server.client().stop();
  delay(1000);
}

void sendHeader() {
  String html = "<!DOCTYPE HTML>";
  html += "<html><head><title>Marquee Scroller</title><link rel='icon' href='data:;base64,='>";
  html += "<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/w3css/4/w3.css'>";
  html += "<link rel='stylesheet' href='https://www.w3schools.com/lib/w3-theme-blue-grey.css'>";
  html += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.8.1/css/all.min.css'>";
  html += "</head><body>";
  server.sendContent(html);
  html = "<nav class='w3-sidebar w3-bar-block w3-card' style='margin-top:88px' id='mySidebar'>";
  html += "<div class='w3-container w3-theme-d2'>";
  html += "<span onclick='closeSidebar()' class='w3-button w3-display-topright w3-large'><i class='fas fa-times'></i></span>";
  html += "<div class='w3-padding'>Menu</div></div>";
  server.sendContent(html);

  server.sendContent(FPSTR(WEB_ACTIONS1));
  Serial.println("Displays: " + String(numberOfHorizontalDisplays));
  if (numberOfHorizontalDisplays >= 8) {
    server.sendContent("<a class='w3-bar-item w3-button' href='/configurewideclock'><i class='far fa-clock'></i> Wide Clock</a>");
  }
  server.sendContent(FPSTR(WEB_ACTIONS2));
  if (displayOn) {
    server.sendContent("<i class='fas fa-eye-slash'></i> Turn Display OFF");
  } else {
    server.sendContent("<i class='fas fa-eye'></i> Turn Display ON");
  }
  server.sendContent(FPSTR(WEB_ACTION3));

  html = "</nav>";
  html += "<header class='w3-top w3-bar w3-theme'><button class='w3-bar-item w3-button w3-xxxlarge w3-hover-theme' onclick='openSidebar()'><i class='fas fa-bars'></i></button><h2 class='w3-bar-item'>Weather Marquee</h2></header>";
  html += "<script>";
  html += "function openSidebar(){document.getElementById('mySidebar').style.display='block'}function closeSidebar(){document.getElementById('mySidebar').style.display='none'}closeSidebar();";
  html += "</script>";
  html += "<br><div class='w3-container w3-large' style='margin-top:88px'>";
  server.sendContent(html);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println("Wifi Manager");
  Serial.println("Please connect to AP");
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("To setup Wifi Configuration");
  scrollMessage("Please Connect to AP: " + String(myWiFiManager->getConfigPortalSSID()));
  centerPrint("wifi");
}

void flashLED(int number, int delayTime) {
  for (int inx = 0; inx < number; inx++) {
    tone(BUZZER_PIN, 440, delayTime);
    delay(delayTime);
    digitalWrite(externalLight, LOW);
    delay(delayTime);
    digitalWrite(externalLight, HIGH);
    delay(delayTime);
  }
  noTone(BUZZER_PIN);
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100) {
    return 0;
  } else if (dbm >= -50) {
    return 100;
  } else {
    return 2 * (dbm + 100);
  }
}

void enableDisplay(boolean enable) {
  displayOn = enable;
  if (enable) {
    matrix.shutdown(false);
    matrix.fillScreen(LOW); // show black
    Serial.println("Display was turned ON: " + timeClient.timeToConsole());
  } else {
    matrix.shutdown(true);
    Serial.println("Display was turned OFF: " + timeClient.timeToConsole());
  }
}

// Toggle on and off the display if user defined times
void checkDisplay() {
  if (timeDisplayTurnsOn == "" || timeDisplayTurnsOff == "") {
    return; // nothing to do
  }
  String currentTime = timeClient.zeroPad(timeClient.hour()) + ":" + timeClient.zeroPad(timeClient.minute());

  if (currentTime == timeDisplayTurnsOn && !displayOn) {
    Serial.println("Time to turn display on: " + currentTime);
    flashLED(1, 500);
    enableDisplay(true);
  }

  if (currentTime == timeDisplayTurnsOff && displayOn) {
    Serial.println("Time to turn display off: " + currentTime);
    flashLED(2, 500);
    enableDisplay(false);
  }
}

void writeCityIds() {
  // Save decoded message to SPIFFS file for playback on power up.
  File f = SPIFFS.open(CONFIG, "w");
  if (!f) {
    Serial.println("File open failed!");
  } else {
    Serial.println("Saving settings now...");
    f.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
    f.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
    f.println("ledIntensity=" + String(displayIntensity));
    f.println("scrollSpeed=" + String(displayScrollSpeed));
    f.println("isFlash=" + String(flashOnSeconds));
    f.println("is24hour=" + String(IS_24HOUR));
    f.println("isPM=" + String(IS_PM));
    f.println("wideclockformat=" + Wide_Clock_Style);
    f.println("minutesBetweenScrolling=" + String(minutesBetweenScrolling));
    f.println("www_username=" + String(www_username));
    f.println("www_password=" + String(www_password));
    f.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
    f.println("ENABLE_MQTT=" + String(ENABLE_MQTT));
    f.println("mqttDeviceId=" + mqttDeviceId);
    f.println("mqttUrl=" + mqttUrl);
    f.println("mqttPort=" + String(mqttPort));
    f.println("mqttTopic=" + mqttTopic);
  }
  f.close();
  readCityIds();
}

void readCityIds() {
  if (SPIFFS.exists(CONFIG) == false) {
    Serial.println("Settings File does not yet exists.");
    writeCityIds();
    return;
  }
  File fr = SPIFFS.open(CONFIG, "r");
  String line;
  while (fr.available()) {
    line = fr.readStringUntil('\n');
    if (line.indexOf("isFlash=") >= 0) {
      flashOnSeconds = line.substring(line.lastIndexOf("isFlash=") + 8).toInt();
      Serial.println("flashOnSeconds=" + String(flashOnSeconds));
    }
    if (line.indexOf("is24hour=") >= 0) {
      IS_24HOUR = line.substring(line.lastIndexOf("is24hour=") + 9).toInt();
      Serial.println("IS_24HOUR=" + String(IS_24HOUR));
    }
    if (line.indexOf("isPM=") >= 0) {
      IS_PM = line.substring(line.lastIndexOf("isPM=") + 5).toInt();
      Serial.println("IS_PM=" + String(IS_PM));
    }
    if (line.indexOf("wideclockformat=") >= 0) {
      Wide_Clock_Style = line.substring(line.lastIndexOf("wideclockformat=") + 16);
      Wide_Clock_Style.trim();
      Serial.println("Wide_Clock_Style=" + Wide_Clock_Style);
    }
    if (line.indexOf("minutesBetweenScrolling=") >= 0) {
      displayRefreshCount = 1;
      minutesBetweenScrolling = line.substring(line.lastIndexOf("minutesBetweenScrolling=") + 24).toInt();
      Serial.println("minutesBetweenScrolling=" + String(minutesBetweenScrolling));
    }
    if (line.indexOf("timeDisplayTurnsOn=") >= 0) {
      timeDisplayTurnsOn = line.substring(line.lastIndexOf("timeDisplayTurnsOn=") + 19);
      timeDisplayTurnsOn.trim();
      Serial.println("timeDisplayTurnsOn=" + timeDisplayTurnsOn);
    }
    if (line.indexOf("timeDisplayTurnsOff=") >= 0) {
      timeDisplayTurnsOff = line.substring(line.lastIndexOf("timeDisplayTurnsOff=") + 20);
      timeDisplayTurnsOff.trim();
      Serial.println("timeDisplayTurnsOff=" + timeDisplayTurnsOff);
    }
    if (line.indexOf("ledIntensity=") >= 0) {
      displayIntensity = line.substring(line.lastIndexOf("ledIntensity=") + 13).toInt();
      Serial.println("displayIntensity=" + String(displayIntensity));
    }
    if (line.indexOf("scrollSpeed=") >= 0) {
      displayScrollSpeed = line.substring(line.lastIndexOf("scrollSpeed=") + 12).toInt();
      Serial.println("displayScrollSpeed=" + String(displayScrollSpeed));
    }
    if (line.indexOf("www_username=") >= 0) {
      String temp = line.substring(line.lastIndexOf("www_username=") + 13);
      temp.trim();
      www_username = temp;
      Serial.println("www_username=" + String(www_username));
    }
    if (line.indexOf("www_password=") >= 0) {
      String temp = line.substring(line.lastIndexOf("www_password=") + 13);
      temp.trim();
      www_password = temp;
      Serial.println("www_password=" + String(www_password));
    }
    if (line.indexOf("IS_BASIC_AUTH=") >= 0) {
      IS_BASIC_AUTH = line.substring(line.lastIndexOf("IS_BASIC_AUTH=") + 14).toInt();
      Serial.println("IS_BASIC_AUTH=" + String(IS_BASIC_AUTH));
    }
    if (line.indexOf("ENABLE_MQTT=") >= 0) {
      ENABLE_MQTT = line.substring(line.lastIndexOf("ENABLE_MQTT=") + 12).toInt();
      Serial.println("ENABLE_MQTT=" + String(ENABLE_MQTT));
      //ENABLE_MQTT = false;
    }
    if (line.indexOf("mqttUrl=") >= 0) {
      mqttUrl = line.substring(line.lastIndexOf("mqttUrl=") + 8);
      Serial.println("mqttUrl=" + String(mqttUrl));
    }
    if (line.indexOf("mqttPort=") >= 0) {
      mqttPort = line.substring(line.lastIndexOf("mqttPort=") + 9).toInt();
      Serial.println("mqttPort=" + String(mqttPort));
    }
    if (line.indexOf("mqttTopic=") >= 0) {
      mqttTopic = line.substring(line.lastIndexOf("mqttTopic=") + 10);
      Serial.println("mqttTopic=" + String(mqttTopic));
    }
    if (line.indexOf("mqttDeviceId=") >= 0) {
      mqttDeviceId = line.substring(line.lastIndexOf("mqttDeviceId=") + 13);
      Serial.println("mqttDeviceId=" + String(mqttDeviceId));
    }
  }
  fr.close();
  matrix.setIntensity(displayIntensity);
}

void scrollMessage(String msg) {
  msg += " "; // add a space at the end
  for ( unsigned int i = 0 ; i < width * msg.length() + matrix.width() - 1 - spacer; i++ ) {
    if (WEBSERVER_ENABLED) {
      server.handleClient();
    }
    if (ENABLE_OTA) {
      ArduinoOTA.handle();
    }
    if (ENABLE_MQTT) {
      mqttClient.loop();
    }
    if (refresh == 1) i = 0;
    refresh = 0;
    matrix.fillScreen(LOW);

    unsigned int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < msg.length() ) {
        matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= width;
    }

    matrix.write(); // Send bitmap to display
    delay(displayScrollSpeed);
  }
  matrix.setCursor(0, 0);
}

void centerPrint(String msg) {
  centerPrint(msg, false);
}

void centerPrint(String msg, boolean extraStuff) {
  int x = (matrix.width() - (msg.length() * width)) / 2;

  // Print the static portions of the display before the main Message
  if (extraStuff) {
    if (!IS_24HOUR && IS_PM && timeClient.isPM()) {
      matrix.drawPixel(matrix.width() - 1, 6, HIGH);
    }
  }
  
  matrix.setCursor(x, 0);
  matrix.print(msg);

  matrix.write();
}
