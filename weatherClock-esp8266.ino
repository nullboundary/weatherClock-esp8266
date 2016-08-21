#include <ESP8266WiFi.h>
#include <time.h>
#include <Ticker.h>
#include <SPI.h>
#include <Pixels_Antialiasing.h>
#include <Pixels_SPIhw.h>
#include <Pixels_ILI9341.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>    

const char* apiKey = ""; //openweathermap.org
const char* cityID = ""; 

const int timezone = 0;   
int dst = 0;


int currentTemp; //current temperature
float currentWindSpeed;
const char* currentWeather; //string that holds current weather condition

Ticker timeTick; //ticker for a repeated 1sec time update
bool updateTime = false; //check if ticker has fired

Ticker weatherCheck; //ticker for a repeated weather update
bool updateWeather = false; //check if ticker has fired

// Declare which fonts we will be using
extern uint8_t Roboto72a[];
extern uint8_t Roboto48a[];
extern uint8_t Roboto18a[];

//Lcd size
Pixels pxs(240, 320);

//lcd spi defines
#define TFT_CLK D5
#define TFT_MOSI D7
#define TFT_DC D4
#define TFT_CS D1
#define TFT_RST D2

struct text { //text render struct
  bool updated;
  String str;
  String oldStr;
};

text dateText;
text dayText;
text timeText;
text tempText;
text condText;
text windText;
bool rectUpdated;

RGB bgColor  = RGB(0,0,0);

//red tones
RGB brightRedColor  = RGB(254, 41, 27);
RGB darkRedColor  = RGB(110, 23, 20);

//blue tones
RGB brightBlueColor  = RGB(0,197,169);
//RGB darkBlueColor  = RGB(16,47,42);

//green tones
//RGB brightColor  = RGB(117,204,38);
//RGB darkColor  = RGB(26,89,33);

//set color tones
RGB brightColor  = brightRedColor;
RGB darkColor  = darkRedColor;

/*******************************************************

 configModeCallback

 *******************************************************/
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());

   pxs.setColor(&brightColor);
   pxs.setFont(Roboto18a);
   pxs.print(15, 25, "Cannot connect to Wifi");
   pxs.print(15, 66, "Join WeatherClock Wifi AP");
   pxs.print(15,89, "to configure connection.");
}

/*******************************************************

 setup

 *******************************************************/
void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);

  Serial.println("Setup LCD");
  pxs.setSpiPins(TFT_CLK, TFT_MOSI, TFT_CS, TFT_RST, TFT_DC);
  pxs.init();
  pxs.setBackground(&bgColor);
  pxs.clear();
  pxs.setOrientation(LANDSCAPE_FLIP);
  pxs.enableAntialiasing(true);

  delay(500);

  WiFiManager wifiManager;
  //wifiManager.resetSettings(); //for testing
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("WeatherClock");
 
  configTime(timezone * 3600, dst, "pool.ntp.org", "time.nist.gov"); //configtime is esp8266 function

  Serial.println("\nWaiting for time sync");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  
  pxs.clear();
  pxs.setColor(&brightColor);
  pxs.drawRoundRectangle(2, 168, 316, 70, 12); //time rect
  pxs.drawRoundRectangle(162, 2, 157, 83, 12); //location rect
  pxs.drawRoundRectangle(162, 85, 157, 83, 12); //weather cond rect
  pxs.drawRoundRectangle(2, 2, 160, 166, 12); //temp rect


  //initialize text rendering false
  timeText.updated = false;
  dateText.updated = false;
  dayText.updated = false;
  tempText.updated = false;
  condText.updated = false;
  windText.updated = false;
  rectUpdated = false;

  delay(500);

  timeTick.attach(1, tick);

  getWeather();

  weatherCheck.attach(360, check);
}

/*******************************************************

 loop

 *******************************************************/
void loop() {

  if (updateTime) {
    updateTime = false;
    clockDisplay();
  } else if (updateWeather) {
    updateWeather = false;
    getWeather();
  }

  render();

}
/*******************************************************

 tick - 1 sec clock interval

 *******************************************************/
void tick() {
  updateTime = true;
}
/*******************************************************

 check - weather update interval

 *******************************************************/
void check() {
  updateWeather = true;
}

/*******************************************************

 render

 *******************************************************/
void render() {

  //draw time
  if (timeText.updated || rectUpdated) {
    timeText.updated = false;
    pxs.setColor(&brightColor);
    pxs.setFont(Roboto48a);
    int widthOldX = pxs.getTextWidth(timeText.oldStr);
    pxs.cleanText((pxs.getWidth() - widthOldX) - 15, 179, timeText.oldStr);
    int widthNewX = pxs.getTextWidth(timeText.str);
    pxs.print((pxs.getWidth() - widthNewX) - 15, 179, timeText.str);
  }

  //draw date
  if (dateText.updated || rectUpdated) { //update colors for rain
    dateText.updated = false;
    pxs.setColor(&brightColor);
    pxs.setFont(Roboto18a);
    pxs.cleanText(15, 205, dateText.oldStr);
    pxs.print(15, 205, dateText.str);
  }

  //draw day
  if (dayText.updated || rectUpdated) {
    dayText.updated = false;
    pxs.setColor(&brightColor);
    pxs.setFont(Roboto18a);
    pxs.cleanText(15, 180, dayText.oldStr);
    pxs.print(15, 180, dayText.str);
  }

  //draw temperature
  if (tempText.updated || rectUpdated) {
    tempText.updated = false;

    pxs.setColor(&brightColor);
    pxs.setFont(Roboto18a);
    String tempCStr = "C";
    pxs.print(134, 46, tempCStr);

    pxs.setFont(Roboto48a);
    String tempMinusStr = "-";
    if (currentTemp < 0) { //minus temperatures
      pxs.print(10, 88, tempMinusStr);
    } else {
      pxs.cleanText(10, 88, tempMinusStr);
    }

    pxs.setFont(Roboto72a);
    int widthOldTempX = pxs.getTextWidth(tempText.oldStr);
    int widthTempX = pxs.getTextWidth(tempText.str);
    pxs.cleanText(80 - widthOldTempX /2, 48, tempText.oldStr);
    pxs.print(80 - widthTempX /2 , 48, tempText.str);
    
  }

  //draw weather condition
  if (condText.updated || rectUpdated) {
    condText.updated = false;
    pxs.setFont(Roboto18a);
    pxs.setColor(&brightColor);

    int widthOldCondX = pxs.getTextWidth(condText.oldStr);
    int widthCondX = pxs.getTextWidth(condText.str);
    pxs.cleanText(241 - widthOldCondX / 2,  112, condText.oldStr);
    pxs.print(241 - widthCondX / 2, 112, condText.str);
  }

  if (windText.updated || rectUpdated) {
    windText.updated = false;
    pxs.setFont(Roboto18a);
    pxs.setColor(&brightColor);

    String windLabelStr = "Wind Speed";
    int labelWidth = pxs.getTextWidth(windLabelStr);
    pxs.print(241 - labelWidth / 2, 16, windLabelStr);

    int widthOldWind = pxs.getTextWidth(windText.oldStr);
    int widthWind = pxs.getTextWidth(windText.str);
    
    pxs.cleanText(241 - widthOldWind / 2, 42, windText.oldStr);
    pxs.print(241 - widthWind / 2, 42, windText.str);

  }

  //redraw rectangles usually for color change
  if (rectUpdated) {
    rectUpdated = false;
    pxs.setColor(&brightColor);
    pxs.drawRoundRectangle(2, 168, 316, 70, 12); //time rect
    pxs.drawRoundRectangle(162, 2, 157, 83, 12); //location rect
    pxs.drawRoundRectangle(162, 85, 157, 83, 12); //weather cond rect
    pxs.drawRoundRectangle(2, 2, 160, 166, 12); //temp rect
  }

}

/*******************************************************

 clockDisplay

 *******************************************************/
void clockDisplay() {

  time_t now = time(nullptr);
  Serial.println(ctime(&now));

  struct tm * timeinfo;
  timeinfo = localtime (&now);

  setClockTime(timeinfo);
  setClockDate(timeinfo);
  setClockDay(timeinfo);

}
/*******************************************************

 getWeather

 *******************************************************/
void getWeather() {

  getWeatherCurrent();

  String weatherCond = String(currentWeather);
  if (weatherCond.length() < 1) {
    weatherCond = "-";
  }

  //set colors to blue if rainy
  if (rainCond(weatherCond)){
    brightColor  = brightBlueColor;
    rectUpdated = true;
  } else {
    brightColor  = brightRedColor;
    rectUpdated = true;
  }
  
  Serial.println(weatherCond);

  String weatherTemp = String(currentTemp);
  if (weatherTemp.length() < 1) {
    weatherTemp = "-";
  }
  Serial.println(weatherTemp);
  
  //convert to km/h
  float kmhspeed = currentWindSpeed * 3.6;
  static char outstr[8];
  dtostrf(kmhspeed,4, 1, outstr);
  
  String windSpeed = String(outstr);
  if (windSpeed.length() < 1) {
    windSpeed = "-";
  }
  windSpeed.concat(" km/h");
  Serial.println(windSpeed);

  updateText(weatherCond, condText);

  updateText(weatherTemp, tempText);

  updateText(windSpeed, windText);


}

/*******************************************************

 rainCond

 *******************************************************/
bool rainCond(String &weatherCond) {

  if (weatherCond == "Rain"){
    return true;
  } else if (weatherCond == "Drizzle") {
    return true;
  } else if (weatherCond == "Thunderstorm") {
    return true;
  } else if (weatherCond == "Snow") {
    return true;
  }

  return false;
  
}

/*******************************************************

 updateText

 *******************************************************/
void updateText(String &newText, struct text &textStruct) {

  if (newText != textStruct.str) {
    textStruct.oldStr = textStruct.str; //save old str so we can clear it from lcd.
    textStruct.str = newText;
    textStruct.updated = true;
  }
}
