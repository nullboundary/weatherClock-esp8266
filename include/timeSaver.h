#pragma once

#include <Arduino.h>

int year(int tm_year);
int month(int tm_month);
void minute(int tm_minute, char buffer[]);
int hour12(int tm_hour);
void day(int dayNum, char buffer[]);

void setClockTime(String &dest, struct tm *timeinfo);
void setClockDate(String &dest, struct tm *timeinfo);
void setClockDay(String &dest, struct tm *timeinfo);
