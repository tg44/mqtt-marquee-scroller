# MQTT Marquee Scroller

## Features include:
* Accurate Clock refresh off Internet Time Servers
* Configured through Web Interface
* Basic Authorization around Configuration web interface
* Support for OTA (loading firmware over WiFi)
* Update firmware through web interface
* Configurable scroll speed
* Configurable scrolling frequency
* Configurable number of LED panels
* Send scroll-messages on MQTT
* Send clock-face settings over MQTT
* Panel based clockface settings (hh:mm, binary, double-binary, percentage, single character, hh:mm:ss)

# Software

## Compiling and Loading to Wemos D1
With installed PlatformIO; `pio run -t upload -t monitor`

## Initial Configuration
For hardware defaults, you may have modify **Settings.h**.
All other settings are managed in the Web Interface. (However Settings.h should be work out of the box for the linked wemos + matrix.)

NOTE: The settings in the Storage.h are the default settings for the first loading. After loading you will manage changes to the settings via the Web Interface. If you want to change settings again in the Storage.h, you will need to erase the file system on the Wemos or use the “Reset Settings” option in the Web Interface.  

## Web Interface
The MQTT Marquee Scroller uses the **WiFiManager** so when it can't find the last network it was connected to 
it will become a **AP Hotspot** -- connect to it with your phone and you can then enter your WiFi connection information.

After connected to your WiFi network it will display the IP addressed assigned to it and that can be 
used to open a browser to the Web Interface.

## MQTT
If you enable MQTT there are two topics that you can set; topic and faceTopic. I will describe the protocol and behavior below.

#### Topic protocol
```json
{
  "message": "message"
}
```
The scroller will subscribe the given topic, and will concat the messages `message` attribute. Once the scroll period is reached, it will show the agregated message, and empties the buffer. This means if you have a 1 minute scroll frequency, and you send messages twice in a minute, the messages will duplicate. (Working solution below!)

#### Face Topic protocol
```json
{
  "panels": [
    { "t":2 },
    { "t":0 },
    { "t":3, "p":70},
    { "t":3, "p":10}
  ]
}
```
When the scroller is not scrolling messages, it usually show the time. Sennding messages to the faceTopic can modify this behaviour. The panels array can contain panel type (`t`) settings.

Types;
 - 0 placeholder (1 panel)
 - 1 clock face (4 or 6 panels)
 - 2 binary clock face (1 or 2 panels)
 - 3 percentage (1 panel) needs a `p` parameter with a 0-100 value
 - 4 character (1 panel) needs a `c` parameter with a 1 char long string

 The abowe example will show a binary clock in the first two panels, a 70% panel on third and a 10% panel as fourth. This can be useful if you want to switch some progress indication for example when you printing on your 3d printer, or you count down externally.

#### Streamig data
The original project did it stuffs in the MCU. This fork do its stuff with "microservices". For example, if you want weather data, you can use [weather2mqtt](https://github.com/tg44/weather2mqtt) to streaming weather data to a topic, and you can convert and repeat it with [mqtt-transformer](https://github.com/tg44/mqtt-transformer). Adding new capabilities can be added in any language rather fast.

I will try to progress on the mqtt-transformer so it could be use to face chage too. Also there will be examples for my setup.

# Hardware

## Required Parts:
* Wemos D1 Mini: https://amzn.to/2qLyKJd
* Dot Matrix Module: https://amzn.to/2HtnQlD  

Note: Using the links provided here help to support the original project (by David Payne).

Note: Currently the code only handles 4-8 panels comfortly (less then 4 will ruin the original clock face, more than 8 is not handled by software). The linked module has 4 panels.

## Wiring for the Wemos D1 Mini to the Dot Matrix Display
CLK -> D5 (SCK)  
CS  -> D6  
DIN -> D7 (MOSI)  
VCC -> 5V+  
GND -> GND-  

![Marquee Scroller Wiring](/images/marquee_scroller_pins.png)  

## 3D Printed Cases:  
* Original Single Panel version (by David Payne): https://www.thingiverse.com/thing:2867294  
* Double Wide LED version (by David Payne): https://www.thingiverse.com/thing:2989552 
* OpenScad version in the openscad folder!

![Marquee Parts](/images/1ffa0c835554d280258c13be5513c4fe_preview_featured.jpg)

# Acknowledgement

This repo based on a fork of; https://github.com/Qrome/marquee-scroller

## Donation or Tip
If you want to, you can support the original authors work onn [paypal](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=A82AT6FLN2MPY) or with buying something from his [Amazon Wishlist](https://www.amazon.com/hz/wishlist/ls/GINC2PHRNEY3).

If you really want to support me instead, you can drop me an email ;)

## Contributors
David Payne  
Nathan Glaus  
Daniel Eichhorn -- Author of the TimeClient class (in older versions)  
yanvigdev  
nashiko-s  
magnum129  
tg44  

Contributing to this software is warmly welcomed. If you have any idea, or feature request, feel free to open an issue!