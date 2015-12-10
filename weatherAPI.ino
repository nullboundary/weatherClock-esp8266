#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* weatherServerName = "api.openweathermap.org";
const char* forecastURL = "/data/2.5/forecast?units=metric";
const char* weatherURL = "/data/2.5/weather?units=metric";

/*******************************************************

 getWeatherCurrent

 *******************************************************/
void getWeatherCurrent() {

  String seoulWeatherURL = buildURL(weatherURL, cityID, apiKey);
  getRequest(weatherServerName, seoulWeatherURL.c_str());

}


/*******************************************************

 getWeatherForecast

 *******************************************************/
void getWeatherForecast() {

  String seoulForecastURL = buildURL(forecastURL, cityID, apiKey);
  getRequest(weatherServerName, seoulForecastURL.c_str());

}
/*******************************************************

 buildURL

 *******************************************************/
String buildURL(const char* baseURL, const char* city, const char* appID) {

  String urlStr = String(baseURL);
  urlStr += "&id=";
  urlStr += city;
  urlStr += "&APPID=";
  urlStr += appID;

  return urlStr;
}

/*******************************************************

  getRequest

 *******************************************************/
void getRequest(const char* hostServer, const char* path) {
 
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;
    Serial.println("[HTTP] begin...");
    http.begin(hostServer, 80, path);
    Serial.print("[HTTP] GET ");
    Serial.println(path);

    int httpCode = http.GET();

    if (httpCode) {
      Serial.printf("[HTTP] Response code - %d\n", httpCode);

      // Response OK
      if (httpCode == 200) {

        StaticJsonBuffer<2048> jsonBuffer;    //create buffer for parsing
        WiFiClient * stream = http.getStreamPtr(); // get tcp stream

        // read all data from server
        while (http.connected()) {

          if (stream->available()) {
            if (stream->find("\n")) {
              String data = stream->readStringUntil('\n');
              data.trim();
              //Serial.println(data);
              const char* lineChars = data.c_str();
              JsonObject& root = jsonBuffer.parseObject(lineChars);

              if (root.success()) {
                root.prettyPrintTo(Serial);
                break;
              } else {
                Serial.println("parse fail");
              }
            }

          }
        }
        Serial.println();
        Serial.print("[HTTP] connection closed.\n");
      }

    } else {
      Serial.print("[HTTP] GET... failed, no connection or no HTTP server\n");
    }

  }
}
