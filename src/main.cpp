#include "main.h"

const int timezone = 0;
int dst = 0;

bool updateTime = false;    //check if ticker has fired
bool updateWeather = false; //check if ticker has fired

//set color tones
unsigned int bgColor = TFT_BLACK;
unsigned int brightColor = TFT_WHITE; //brightRedColor;
unsigned int darkColor = darkRedColor;

weather currentWeather;
TFT_eSPI tft = TFT_eSPI();

/*******************************************************

 configModeCallback

 *******************************************************/
void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

  tft.setTextColor(brightColor, TFT_BLACK);
  tft.loadFont(LatoRegular28);

  tft.setCursor(15, 25);
  tft.print("Cannot connect to Wifi");

  tft.setCursor(15, 66);
  tft.print("Join WeatherClock Wifi AP");

  tft.setCursor(15, 89);
  tft.print("to configure connection.");
}

/*******************************************************

 setup

 *******************************************************/
void setup()
{

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("Setup LCD");
  delay(1000);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  delay(500);

  WiFiManager wifiManager;
  //wifiManager.resetSettings(); //for testing
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("WeatherClock");

  configTime("PST8PDT,M3.2.0/02:00:00,M11.1.0/02:00:00", "pool.ntp.org", "time.nist.gov");

  Serial.println("\nWaiting for time sync");
  while (!time(nullptr))
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  tft.fillScreen(TFT_BLACK);

  tft.drawRoundRect(2, 224, 476, 94, 12, brightColor);    //time rect
  tft.drawRoundRect(242, 2, 237, 112, 12, brightColor);   //location rect
  tft.drawRoundRect(242, 113, 237, 112, 12, brightColor); //weather cond rect
  tft.drawRoundRect(2, 2, 241, 223, 12, brightColor);     //temp rect

  //initialize text rendering false
  timeText.updated = false;
  dateText.updated = false;
  dayText.updated = false;
  tempText.updated = false;
  condText.updated = false;
  windText.updated = false;
  windDegText.updated = false;
  rectUpdated = false;

  delay(500);

  timeTick.attach(1, tick);

  getWeather();

  weatherCheck.attach(360, check);
}

/*******************************************************

 loop

 *******************************************************/
void loop()
{
  if (updateTime)
  {
    updateTime = false;
    clockDisplay();
  }
  else if (updateWeather)
  {
    updateWeather = false;
    getWeather();
  }

  render();
}

/*******************************************************

 tick - 1 sec clock interval

 *******************************************************/
void tick()
{
  updateTime = true;
}
/*******************************************************

 check - weather update interval

 *******************************************************/
void check()
{
  updateWeather = true;
}

/*******************************************************

 render

 *******************************************************/
void render()
{

  //draw time
  if (timeText.updated || rectUpdated)
  {
    timeText.updated = false;
    tft.setTextColor(brightColor, TFT_BLACK);
    tft.loadFont(LatoRegular72);

    int widthOldX = tft.textWidth(timeText.oldStr);
    int widthNewX = tft.textWidth(timeText.str);

    tft.fillRect(tft.width() - widthOldX - 15, 245, widthOldX, tft.fontHeight(), TFT_BLACK);
    tft.drawString(timeText.str, tft.width() - widthNewX - 15, 245);
  }

  //draw date
  if (dateText.updated || rectUpdated)
  {
    //update colors for rain
    dateText.updated = false;
    tft.setTextColor(brightColor, TFT_BLACK);
    tft.loadFont(LatoRegular28);

    int widthOldX = tft.textWidth(dateText.oldStr);

    tft.fillRect(15, 278, widthOldX, tft.fontHeight(), TFT_BLACK);
    tft.drawString(dateText.str, 15, 278);
  }

  //draw day
  if (dayText.updated || rectUpdated)
  {
    dayText.updated = false;
    tft.setTextColor(brightColor, TFT_BLACK);
    tft.loadFont(LatoRegular28);

    int widthOldX = tft.textWidth(dayText.oldStr);

    tft.fillRect(15, 245, widthOldX, tft.fontHeight(), TFT_BLACK);
    tft.drawString(dayText.str, 15, 245);
  }

  //draw temperature
  if (tempText.updated || rectUpdated)
  {
    tempText.updated = false;

    tft.setTextColor(brightColor, TFT_BLACK);
    tft.loadFont(LatoRegular28);
    String tempCStr = "°C";

    tft.drawString(tempCStr, 196, 66);
    tft.loadFont(LatoRegular72);

    String tempMinusStr = "-";
    if (currentWeather.temp < 0)
    {
      //minus temperatures
      tft.drawString(tempMinusStr, 15, 122);
    }
    else
    {

      int widthOldX = tft.textWidth(tempMinusStr);
      tft.fillRect(15, 122, widthOldX, tft.fontHeight(), TFT_BLACK);
    }

    tft.loadFont(LatoRegular112);
    int widthOldTempX = tft.textWidth(tempText.oldStr);
    int widthTempX = tft.textWidth(tempText.str);

    tft.fillRect(110 - widthOldTempX / 2, 69, widthOldTempX, tft.fontHeight(), TFT_BLACK);
    tft.drawString(tempText.str, 110 - widthTempX / 2, 69);
  }

  //draw weather condition
  if (condText.updated || rectUpdated)
  {
    condText.updated = false;
    tft.loadFont(LatoRegular28);
    tft.setTextColor(brightColor, TFT_BLACK);

    int widthOldCondX = tft.textWidth(condText.oldStr);
    int widthCondX = tft.textWidth(condText.str);

    tft.fillRect(362 - widthOldCondX / 2, 154, widthOldCondX, tft.fontHeight(), TFT_BLACK);
    tft.drawString(condText.str, 362 - widthCondX / 2, 154);
  }

  if (windText.updated || rectUpdated)
  {
    windText.updated = false;
    tft.loadFont(LatoRegular28);
    tft.setTextColor(brightColor, TFT_BLACK);

    int widthOldWind = tft.textWidth(windText.oldStr);
    int widthWind = tft.textWidth(windText.str);

    tft.fillRect(362 - widthOldWind / 2, 61, widthOldWind, tft.fontHeight(), TFT_BLACK);
    tft.drawString(windText.str, 362 - widthWind / 2, 61);
  }

  if (windDegText.updated || rectUpdated)
  {

    windDegText.updated = false;
    tft.loadFont(LatoRegular28);
    tft.setTextColor(brightColor, TFT_BLACK);

    int widthOldWindDeg = tft.textWidth(windDegText.oldStr);
    int widthWindDeg = tft.textWidth(windDegText.str);

    tft.fillRect(362 - widthOldWindDeg / 2, 26, widthOldWindDeg, tft.fontHeight(), TFT_BLACK);
    tft.drawString(windDegText.str, 362 - widthWindDeg / 2, 26);
  }

  //redraw rectangles usually for color change
  if (rectUpdated)
  {
    rectUpdated = false;
    tft.drawRoundRect(2, 224, 476, 94, 12, brightColor);    //time rect
    tft.drawRoundRect(242, 2, 237, 112, 12, brightColor);   //location rect
    tft.drawRoundRect(242, 113, 237, 112, 12, brightColor); ///weather cond rect
    tft.drawRoundRect(2, 2, 241, 223, 12, brightColor);     //temp rect
  }
}

/*******************************************************

 clockDisplay

 *******************************************************/
void clockDisplay()
{

  time_t now = time(nullptr);
  Serial.println(ctime(&now));

  struct tm *timeinfo;
  timeinfo = localtime(&now);

  String time;
  setClockTime(time, timeinfo);
  updateText(time, timeText);

  String date;
  setClockDate(date, timeinfo);
  updateText(date, dateText);

  String day;
  setClockDay(day, timeinfo);
  updateText(day, dayText);
}
/*******************************************************

 getWeather

 *******************************************************/
void getWeather()
{

  getWeatherCurrent(currentWeather);

  String weatherCond = String(currentWeather.conditions);
  if (weatherCond.length() < 1)
  {
    weatherCond = "-";
  }

  //set colors to blue if rainy
  if (rainCond(weatherCond))
  {
    brightColor = TFT_BLUE;
    rectUpdated = true;
  }
  else
  {
    brightColor = TFT_WHITE;
    rectUpdated = true;
  }

  // Serial.println(weatherCond);

  String weatherTemp = String(currentWeather.temp);
  if (weatherTemp.length() < 1)
  {
    weatherTemp = "-";
  }
  // Serial.println(weatherTemp);

  //convert to km/h
  float kmhspeed = currentWeather.windSpeed * 3.6;
  static char outstr[8];
  dtostrf(kmhspeed, 4, 1, outstr);

  String windSpeed = String(outstr);
  if (windSpeed.length() < 1)
  {
    windSpeed = "-";
  }
  windSpeed.concat(" km/h");
  // Serial.println(windSpeed);

  String windDir = getWindDirection(currentWeather.windDegree);

  updateText(weatherCond, condText);

  updateText(weatherTemp, tempText);

  updateText(windSpeed, windText);

  updateText(windDir, windDegText);
}

/*******************************************************

 rainCond

 *******************************************************/
bool rainCond(String &weatherCond)
{

  if (weatherCond == "Rain")
  {
    return true;
  }
  else if (weatherCond == "Drizzle")
  {
    return true;
  }
  else if (weatherCond == "Thunderstorm")
  {
    return true;
  }
  else if (weatherCond == "Snow")
  {
    return true;
  }

  return false;
}

/*******************************************************

 getWindDirection

 *******************************************************/
String getWindDirection(int windDeg)
{

  if (windDeg > 337 || windDeg <= 22)
  {
    return "North";
  }
  else if (windDeg > 22 && windDeg <= 67)
  {
    return "North East";
  }
  else if (windDeg > 67 && windDeg <= 112)
  {
    return "East";
  }
  else if (windDeg > 112 && windDeg <= 157)
  {
    return "South East";
  }
  else if (windDeg > 157 && windDeg <= 202)
  {
    return "South";
  }
  else if (windDeg > 202 && windDeg <= 247)
  {
    return "South West";
  }
  else if (windDeg > 247 && windDeg <= 292)
  {
    return "West";
  }
  else if (windDeg > 292 && windDeg <= 337)
  {
    return "North West";
  }

  return "";
}

/*******************************************************

 updateText

 *******************************************************/
void updateText(String &newText, struct text &textStruct)
{

  if (newText != textStruct.str)
  {
    textStruct.oldStr = textStruct.str; //save old str so we can clear it from lcd.
    textStruct.str = newText;
    textStruct.updated = true;
  }
}
