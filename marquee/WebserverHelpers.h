#include "Settings.h"


static const char WEB_ACTIONS1[] PROGMEM = "<a class='w3-bar-item w3-button' href='/'><i class='fas fa-home'></i> Home</a>"
                        "<a class='w3-bar-item w3-button' href='/configure'><i class='fas fa-cog'></i> Configure</a>";

static const char WEB_ACTIONS2[] PROGMEM = "<a class='w3-bar-item w3-button' href='/configuremqtt'><i class='fab fa-stream'></i> MQTT</a>"
                        "<a class='w3-bar-item w3-button' href='/pull'><i class='fas fa-cloud-download-alt'></i> Refresh Data</a>"
                        "<a class='w3-bar-item w3-button' href='/display'>";

static const char WEB_ACTION3[] PROGMEM = "</a><a class='w3-bar-item w3-button' href='/systemreset' onclick='return confirm(\"Do you want to reset to default weather settings?\")'><i class='fas fa-undo'></i> Reset Settings</a>"
                       "<a class='w3-bar-item w3-button' href='/forgetwifi' onclick='return confirm(\"Do you want to forget to WiFi connection?\")'><i class='fas fa-wifi'></i> Forget WiFi</a>"
                       "<a class='w3-bar-item w3-button' href='/update'><i class='fas fa-wrench'></i> Firmware Update</a>"
                       "<a class='w3-bar-item w3-button' href='https://github.com/Qrome/marquee-scroller' target='_blank'><i class='fas fa-question-circle'></i> About</a>";

static const char CHANGE_FORM1[] PROGMEM = "<form class='w3-container' action='/locations' method='get'><h2>Configure:</h2>"
                      "<p><input name='is24hour' class='w3-check w3-margin-top' type='checkbox' %IS_24HOUR_CHECKED%> Use 24 Hour Clock (military time)</p>";

static const char CHANGE_FORM2[] PROGMEM = "<p><input name='isPM' class='w3-check w3-margin-top' type='checkbox' %IS_PM_CHECKED%> Show PM indicator (only 12h format)</p>"
                      "<p><input name='flashseconds' class='w3-check w3-margin-top' type='checkbox' %FLASHSECONDS%> Flash : in the time</p>"
                      "<p><label>Start Time </label><input name='startTime' type='time' value='%STARTTIME%'></p>"
                      "<p><label>End Time </label><input name='endTime' type='time' value='%ENDTIME%'></p>"
                      "<p>Display Brightness <input class='w3-border w3-margin-bottom' name='ledintensity' type='number' min='0' max='15' value='%INTENSITYOPTIONS%'></p>"
                      "<p>Display Scroll Speed <select class='w3-option w3-padding' name='scrollspeed'>%SCROLLOPTIONS%</select></p>"
                      "<p>Minutes Between Scrolling Data <input class='w3-border w3-margin-bottom' name='refreshDisplay' type='number' min='1' max='10' value='%REFRESH_DISPLAY%'></p>";

static const char CHANGE_FORM3[] PROGMEM = "<hr><p><input name='isBasicAuth' class='w3-check w3-margin-top' type='checkbox' %IS_BASICAUTH_CHECKED%> Use Security Credentials for Configuration Changes</p>"
                      "<p><label>Marquee User ID (for this web interface)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='userid' value='%USERID%' maxlength='20'></p>"
                      "<p><label>Marquee Password </label><input class='w3-input w3-border w3-margin-bottom' type='password' name='stationpassword' value='%STATIONPASSWORD%'></p>"
                      "<p><button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Save</button></p></form>"
                      "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";

static const char WIDECLOCK_FORM[] PROGMEM = "<form class='w3-container' action='/savewideclock' method='get'><h2>Wide Clock Configuration:</h2>"
                          "<p>Wide Clock Display Format <select class='w3-option w3-padding' name='wideclockformat'>%WIDECLOCKOPTIONS%</select></p>"
                          "<button class='w3-button w3-block w3-grey w3-section w3-padding' type='submit'>Save</button></form>";

static const char MQTT_FORM[] PROGMEM = "<form class='w3-container' action='/savemqtt' method='get'><h2>MQTT Configuration:</h2>"
                        "<p><input name='mqttenable' class='w3-check w3-margin-top' type='checkbox' %MQTTCHECKED%> Enable MQTT</p>"
                        "<label>MQTT device name</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='mqttDeviceName' value='%MQTTDEVICE%' maxlength='60'>"
                        "<label>MQTT URL (do not include http://)</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='mqttUrl' value='%MQTTURL%' maxlength='60'>"
                        "<label>MQTT Port</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='mqttPort' value='%MQTTPORT%' maxlength='5'  onkeypress='return isNumberKey(event)'>"
                        "<label>MQTT topic</label><input class='w3-input w3-border w3-margin-bottom' type='text' name='mqttTopic' value='%MQTTTOPIC%'>"
                        "<button class='w3-button w3-block w3-green w3-section w3-padding' type='submit'>Save</button></form>"
                        "<script>function isNumberKey(e){var h=e.which?e.which:event.keyCode;return!(h>31&&(h<48||h>57))}</script>";


String createFooter(int8_t rssi, const char* version) {
  Serial.print("Signal Strength (RSSI): ");
  Serial.print(rssi);
  Serial.println("%");
  String html = "<br><br><br>";
  html += "</div>";
  html += "<footer class='w3-container w3-bottom w3-theme w3-margin-top'>";
  html += "<i class='far fa-paper-plane'></i> Version: " + String(version) + "<br>";
  html += "<i class='fas fa-rss'></i> Signal Strength: ";
  html += String(rssi) + "%";
  html += "</footer>";
  html += "</body></html>";
  return html;
}

String decodeHtmlString(String msg) {
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

