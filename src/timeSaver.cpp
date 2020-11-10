#include "timeSaver.h"

/*******************************************************

 year

 *******************************************************/
int year(int tm_year)
{
  return tm_year + 1900;
}

/*******************************************************

 month

 *******************************************************/
int month(int tm_month)
{
  return tm_month + 1;
}

/*******************************************************

 minute

 *******************************************************/
void minute(int tm_minute, char buffer[])
{

  buffer[0] = 0;

  if (tm_minute < 10)
  {
    sprintf(buffer, "0%d", tm_minute);
  }
  else
  {
    sprintf(buffer, "%d", tm_minute);
  }
}

/*******************************************************

 hour12

 *******************************************************/
int hour12(int tm_hour)
{ // the hour for the given time in 12 hour format

  if (tm_hour == 0)
  {
    return 12; // 12 midnight
  }
  else if (tm_hour > 12)
  {
    return tm_hour - 12;
  }
  else
  {
    return tm_hour;
  }
}

/*******************************************************

 day

 *******************************************************/
void day(int dayNum, char buffer[])
{
  buffer[0] = 0;
  switch (dayNum)
  {
  case 0:
    sprintf(buffer, "Sunday");

    break;
  case 1:
    sprintf(buffer, "Monday");

    break;
  case 2:
    sprintf(buffer, "Tuesday");

    break;
  case 3:
    sprintf(buffer, "Wednesday");

    break;
  case 4:
    sprintf(buffer, "Thursday");

    break;
  case 5:
    sprintf(buffer, "Friday");

    break;
  case 6:
    sprintf(buffer, "Saturday");

    break;
  default:
    sprintf(buffer, "-");
  }
}

/*******************************************************

 setClockTime

 *******************************************************/
void setClockTime(String &dest, struct tm *timeinfo)
{

  String displayTime;
  displayTime += hour12(timeinfo->tm_hour);
  displayTime += ":";

  char minuteBuffer[9];
  minute(timeinfo->tm_min, minuteBuffer);
  displayTime += String(minuteBuffer);

  dest = displayTime;
}
/*******************************************************

 setClockDate

 *******************************************************/
void setClockDate(String &dest, struct tm *timeinfo)
{

  String displayDate;
  displayDate += month(timeinfo->tm_mon);
  displayDate += "-";
  displayDate += timeinfo->tm_mday;
  displayDate += "-";
  displayDate += year(timeinfo->tm_year);

  dest = displayDate;
}
/*******************************************************

 setClockDay

 *******************************************************/
void setClockDay(String &dest, struct tm *timeinfo)
{

  String displayDay;
  char dayBuffer[11];
  day(timeinfo->tm_wday, dayBuffer);
  displayDay += String(dayBuffer);

  dest = displayDay;
}
