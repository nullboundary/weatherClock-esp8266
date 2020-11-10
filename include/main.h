#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <time.h>
#include <Ticker.h>
#include <SPI.h>

#include <TFT_eSPI.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include "helpers.h"
#include "weatherAPI.h"
#include "timeSaver.h"
#include "fonts.h"

//colors
//red tones
#define brightRedColor 0xfe291b //RGB(254, 41, 27);
#define darkRedColor 0x6e1714   //RGB(110, 23, 20);
//blue tones
#define brightBlueColor 0x00c5a7 //RGB(0, 197, 169);

//lcd spi defines
#define TFT_CLK D5
#define TFT_MOSI D7
#define TFT_DC D4
#define TFT_CS D1
#define TFT_RST D2

struct text
{ //text render struct
    bool updated;
    String str;
    String oldStr;
};

Ticker timeTick;     //ticker for a repeated 1sec time update
Ticker weatherCheck; //ticker for a repeated weather update

text dateText;
text dayText;
text timeText;
text tempText;
text condText;
text windText;
text windDegText;

bool rectUpdated;

void configModeCallback(WiFiManager *myWiFiManager);
void setup();
void loop();
void tick();
void check();
void render();
void clockDisplay();
void getWeather();
bool rainCond(String &weatherCond);
String getWindDirection(int windDeg);
void updateText(String &newText, struct text &textStruct);
