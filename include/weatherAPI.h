#pragma once

#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <math.h>
#include "helpers.h"

void getWeatherCurrent(weather &current);
void getWeatherForecast(weather &current);
String buildURL(const char *baseURL, const char *city, const char *appID);
void getRequest(weather &current, String hostServer, String path);
