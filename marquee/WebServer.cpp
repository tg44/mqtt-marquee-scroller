#include "WebServer.h"

WebServer::WebServer(int prt, Storage& st, StatusLed& proc, String appver, Display& disp, TimeService& tc): 
  server(prt), s(st), port(prt), processing(proc), version(appver), display(disp), timeClient(tc) {
}

void WebServer::handleClient(){
    server.handleClient();
}

void WebServer::init(){
    server.on("/", std::bind(&WebServer::handleConfigure, this));
    server.on("/pull", std::bind(&WebServer::handlePull, this));
    server.on("/locations", std::bind(&WebServer::handleLocations, this));
    server.on("/savewideclock", std::bind(&WebServer::handleSaveWideClock, this));
    server.on("/systemreset", std::bind(&WebServer::handleSystemReset, this));
    server.on("/forgetwifi", std::bind(&WebServer::handleForgetWifi, this));
    server.on("/configure", std::bind(&WebServer::handleConfigure, this));
    server.on("/configurewideclock", std::bind(&WebServer::handleWideClockConfigure, this));
    server.on("/display", std::bind(&WebServer::handleDisplay, this));
    server.on("/savemqtt", std::bind(&WebServer::handleSaveMqtt, this));
    server.on("/configuremqtt", std::bind(&WebServer::handleConfigureMqtt, this));
    server.onNotFound(std::bind(&WebServer::redirectHome, this));
    serverUpdater.setup(&server, "/update", s.www_username, s.www_password);
    // Start the server
    server.begin();
    Serial.println("Server started");
    // Print the IP address
    String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(port) + "/";
    Serial.println("Use this URL : " + webAddress);
}

void WebServer::handleSaveWideClock() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  if (display.numberOfHorizontalDisplays >= 8) {
    s.wide_Clock_Style = server.arg("wideclockformat");
    s.serialize();
    display.clear();
  }
  redirectHome();
}


void WebServer::handleLocations() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  s.flashOnSeconds = server.hasArg("flashseconds");
  s.IS_24HOUR = server.hasArg("is24hour");
  s.IS_PM = server.hasArg("isPM");
  s.timeDisplayTurnsOn = decodeHtmlString(server.arg("startTime"));
  s.timeDisplayTurnsOff = decodeHtmlString(server.arg("endTime"));
  s.displayIntensity = server.arg("ledintensity").toInt();
  s.minutesBetweenScrolling = server.arg("refreshDisplay").toInt();
  s.displayScrollSpeed = server.arg("scrollspeed").toInt();
  s.IS_BASIC_AUTH = server.hasArg("isBasicAuth");
  s.www_username = server.arg("userid");
  s.www_password = server.arg("stationpassword");
  display.clear();
  s.serialize();
  timeClient.updateTime(); // this will force a data pull for new weather
  redirectHome();
  display.setBrightness(s.displayIntensity);
}

void WebServer::handleConfigureMqtt() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  processing.on();

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(MQTT_FORM);
  String isMqttChecked = "";
  if (s.ENABLE_MQTT) {
    isMqttChecked = "checked='checked'";
  }
  form.replace("%MQTTCHECKED%", isMqttChecked);
  form.replace("%MQTTDEVICE%", s.mqttDeviceId);
  form.replace("%MQTTURL%", s.mqttUrl);
  form.replace("%MQTTPORT%", String(s.mqttPort));
  form.replace("%MQTTTOPIC%", s.mqttTopic);
  server.sendContent(form);

  server.sendContent(createFooter(getWifiQuality(), version));

  server.sendContent("");
  server.client().stop();
  processing.off();
}


void WebServer::handlePull() {
  timeClient.updateTime();
}

void WebServer::handleSystemReset() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  Serial.println("Reset System Configuration");
  if (s.resetToDefault()) {
    redirectHome();
    ESP.restart();
  }
}

void WebServer::handleForgetWifi() {
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

void WebServer::handleWideClockConfigure() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  processing.on();

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  if (display.numberOfHorizontalDisplays >= 8) {
    // Wide display options
    String form = FPSTR(WIDECLOCK_FORM);
    String clockOptions = "<option value='2'>HH:MM:SS</option><option value='3'>HH:MM</option>";
    clockOptions.replace(s.wide_Clock_Style + "'", s.wide_Clock_Style + "' selected");
    form.replace("%WIDECLOCKOPTIONS%", clockOptions);
    server.sendContent(form);
  }

  server.sendContent(createFooter(getWifiQuality(), version));

  server.sendContent("");
  server.client().stop();
  processing.off();
}

void WebServer::handleConfigure() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  processing.on();
  String html = "";

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  sendHeader();

  String form = FPSTR(CHANGE_FORM1);
  
  String is24hourChecked = "";
  if (s.IS_24HOUR) {
    is24hourChecked = "checked='checked'";
  }
  form.replace("%IS_24HOUR_CHECKED%", is24hourChecked);
  
  server.sendContent(form);

  form = FPSTR(CHANGE_FORM2);
  String isPmChecked = "";
  if (s.IS_PM) {
    isPmChecked = "checked='checked'";
  }
  form.replace("%IS_PM_CHECKED%", isPmChecked);
  String isFlashSecondsChecked = "";
  if (s.flashOnSeconds) {
    isFlashSecondsChecked = "checked='checked'";
  }
  form.replace("%FLASHSECONDS%", isFlashSecondsChecked);
  form.replace("%STARTTIME%", s.timeDisplayTurnsOn);
  form.replace("%ENDTIME%", s.timeDisplayTurnsOff);
  form.replace("%INTENSITYOPTIONS%", String(s.displayIntensity));
  String dSpeed = String(s.displayScrollSpeed);
  String scrollOptions = "<option value='35'>Slow</option><option value='25'>Normal</option><option value='15'>Fast</option><option value='10'>Very Fast</option>";
  scrollOptions.replace(dSpeed + "'", dSpeed + "' selected" );
  form.replace("%SCROLLOPTIONS%", scrollOptions);
  form.replace("%REFRESH_DISPLAY%", String(s.minutesBetweenScrolling));

  server.sendContent(form); //Send another chunk of the form

  form = FPSTR(CHANGE_FORM3);
  String isUseSecurityChecked = "";
  if (s.IS_BASIC_AUTH) {
    isUseSecurityChecked = "checked='checked'";
  }
  form.replace("%IS_BASICAUTH_CHECKED%", isUseSecurityChecked);
  form.replace("%USERID%", s.www_username);
  form.replace("%STATIONPASSWORD%", s.www_password);

  server.sendContent(form); // Send the second chunk of Data

  server.sendContent(createFooter(getWifiQuality(), version));

  server.sendContent("");
  server.client().stop();
  processing.off();
}

void WebServer::handleDisplay() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  display.switchState();
  String state = "OFF";
  if (display.isOn()) {
    state = "ON";
  }
  displayMessage("Display is now " + state);
}


void WebServer::handleSaveMqtt() {
  if (!athentication()) {
    return server.requestAuthentication();
  }
  s.ENABLE_MQTT = server.hasArg("mqttenable");
  s.mqttDeviceId = server.arg("mqttDeviceName");
  s.mqttUrl = server.arg("mqttUrl");
  s.mqttPort = server.arg("mqttPort").toInt();
  s.mqttTopic = server.arg("mqttTopic");
  s.serialize();
  redirectHome();
  ESP.restart();
}


void WebServer::redirectHome() {
  // Send them back to the Root Directory
  server.sendHeader("Location", String("/"), true);
  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");
  server.client().stop();
  delay(1000);
}

boolean WebServer::athentication() {
  if (s.IS_BASIC_AUTH) {
    return server.authenticate(s.www_username.c_str(), s.www_password.c_str());
  }
  return true; // Authentication not required
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

void WebServer::sendHeader() {
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
  Serial.println("Displays: " + String(display.numberOfHorizontalDisplays));
  if (display.numberOfHorizontalDisplays >= 8) {
    server.sendContent("<a class='w3-bar-item w3-button' href='/configurewideclock'><i class='far fa-clock'></i> Wide Clock</a>");
  }
  server.sendContent(FPSTR(WEB_ACTIONS2));
  if (display.isOn()) {
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

void WebServer::displayMessage(String message) {
  processing.on();

  server.sendHeader("Cache-Control", "no-cache, no-store");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  sendHeader();
  server.sendContent(message);
  server.sendContent(createFooter(getWifiQuality(), version));
  server.sendContent("");
  server.client().stop();

  processing.off();
}


String WebServer::createFooter(int8_t rssi, String version) {
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(rssi);
  Serial.println("%");
  String html = "<br><br><br>";
  html += "</div>";
  html += "<footer class='w3-container w3-bottom w3-theme w3-margin-top'>";
  html += "<i class='far fa-paper-plane'></i> Version: " + version + "<br>";
  html += "<i class='fas fa-rss'></i> Signal Strength: ";
  html += String(rssi) + "%";
  html += "</footer>";
  html += "</body></html>";
  return html;
}

String WebServer::decodeHtmlString(String msg) {
  String decodedMsg = msg;
  // Restore special characters that are misformed to %char by the client browser
  decodedMsg.replace("+", " ");
  decodedMsg.replace("%21", "!");
  decodedMsg.replace("%22", "");
  decodedMsg.replace("%23", "#");
  decodedMsg.replace("%24", "$");
  decodedMsg.replace("%25", "%");
  decodedMsg.replace("%26", "&");
  decodedMsg.replace("%27", "'");
  decodedMsg.replace("%28", "(");
  decodedMsg.replace("%29", ")");
  decodedMsg.replace("%2A", "*");
  decodedMsg.replace("%2B", "+");
  decodedMsg.replace("%2C", ",");
  decodedMsg.replace("%2F", "/");
  decodedMsg.replace("%3A", ":");
  decodedMsg.replace("%3B", ";");
  decodedMsg.replace("%3C", "<");
  decodedMsg.replace("%3D", "=");
  decodedMsg.replace("%3E", ">");
  decodedMsg.replace("%3F", "?");
  decodedMsg.replace("%40", "@");
  decodedMsg.toUpperCase();
  decodedMsg.trim();
  return decodedMsg;
}