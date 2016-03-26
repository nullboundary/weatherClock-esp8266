#include <ESP8266WiFi.h>
#include <time.h>
#include <Ticker.h>
#include <SPI.h>
#include <Pixels_Antialiasing.h>
#include <Pixels_SPIhw.h>
#include <Pixels_ILI9341.h>

char ssid[] = "";  //  your network SSID (name)
char pass[] = "";       // your network password
const char* apiKey = ""; //openweathermap.org
const char* cityID = ""; //CityID on openweathermap

const int timezone = 9;     //KST
int dst = 0;


int currentTemp; //current temperature
int currentHumid; //current humidity
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

RGB bgColor  = RGB(0,0,0);

//red tones
RGB brightColor  = RGB(254, 41, 27);
RGB darkColor  = RGB(110, 23, 20);

//blue tones
//RGB brightColor  = RGB(0,197,169);
//RGB darkColor  = RGB(16,47,42);

//green tones
//RGB brightColor  = RGB(117,204,38);
//RGB darkColor  = RGB(26,89,33);


/*******************************************************

 setup

 *******************************************************/
void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  connectWifi();

  configTime(timezone * 3600, dst, "pool.ntp.org", "time.nist.gov"); //configtime is esp8266 function

  Serial.println("\nWaiting for time sync");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");

  Serial.println("Setup LCD");
  pxs.setSpiPins(TFT_CLK, TFT_MOSI, TFT_CS, TFT_RST, TFT_DC);
  pxs.init();
  pxs.setBackground(&bgColor);
  pxs.clear();
  pxs.setOrientation(LANDSCAPE_FLIP);
  pxs.enableAntialiasing(true);

  delay(500);

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
  if (timeText.updated) {
    timeText.updated = false;
    pxs.setColor(&brightColor);
    pxs.setFont(Roboto48a);
    int widthOldX = pxs.getTextWidth(timeText.oldStr);
    pxs.cleanText((pxs.getWidth() - widthOldX) - 15, 179, timeText.oldStr);
    int widthNewX = pxs.getTextWidth(timeText.str);
    pxs.print((pxs.getWidth() - widthNewX) - 15, 179, timeText.str);
  }

  //draw date
  if (dateText.updated) {
    dateText.updated = false;
    pxs.setColor(&brightColor);
    pxs.setFont(Roboto18a);
    pxs.cleanText(15, 205, dateText.oldStr);
    pxs.print(15, 205, dateText.str);
  }

  //draw day
  if (dayText.updated) {
    dayText.updated = false;
    pxs.setColor(&brightColor);
    pxs.setFont(Roboto18a);
    pxs.cleanText(15, 180, dayText.oldStr);
    pxs.print(15, 180, dayText.str);
  }

  //draw temperature
  if (tempText.updated) {
    tempText.updated = false;
    pxs.setColor(&darkColor);
    pxs.fillRoundRectangle(2, 2, 160, 166, 12);
    pxs.setColor(&brightColor);
    pxs.drawRoundRectangle(2, 2, 160, 166, 12);

    pxs.setColor(&bgColor);
    pxs.setFont(Roboto18a);
    String tempCStr = "C";
    pxs.print(132, 98, tempCStr);

    pxs.setFont(Roboto48a);
    if (currentTemp < 0) { //minus temperatures
      String tempMinusStr = "-";
      pxs.print(10, 88, tempMinusStr);
    }

    pxs.setFont(Roboto72a);
    int widthTempX = pxs.getTextWidth(tempText.str);
    //pxs.cleanText(28, 48, tempText.oldStr);
    pxs.print(80 - widthTempX /2 , 48, tempText.str);
    //pxs.print(28, 48, tempText.str);
  }

  //draw weather condition
  if (condText.updated) {
    condText.updated = false;
    pxs.setFont(Roboto18a);
    pxs.setColor(&brightColor);
    //pxs.drawRoundRectangle(163, 2, 156, 83, 12);

    String locStr = "Seoul"; //Todo: maybe this shouldn't be hardcoded?
    int widthlocX = pxs.getTextWidth(locStr);
    pxs.cleanText(241 - widthlocX / 2, 28, locStr);
    pxs.print(241 - widthlocX / 2, 28, locStr);

    //pxs.drawRoundRectangle(163, 85, 156, 83, 12);
    int widthOldCondX = pxs.getTextWidth(condText.oldStr);
    int widthCondX = pxs.getTextWidth(condText.str);
    pxs.cleanText(241 - widthOldCondX / 2,  112, condText.oldStr);
    pxs.print(241 - widthCondX / 2, 112, condText.str);
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

 connectWifi

 *******************************************************/
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
  Serial.println(weatherCond);

  String weatherTemp = String(currentTemp);
  if (weatherTemp.length() < 1) {
    weatherTemp = "-";
  }
  Serial.println(weatherTemp);

  updateText(weatherCond, condText);

  updateText(weatherTemp, tempText);


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

