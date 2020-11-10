#include "weatherAPI.h"

String weatherServerName = "api.openweathermap.org";
const char *forecastURL = "/data/2.5/forecast?units=metric";
const char *weatherURL = "/data/2.5/weather?units=metric";
const char *apiKey = "";
const char *cityID = "";

/*******************************************************

 getWeatherCurrent

 *******************************************************/
void getWeatherCurrent(weather &current)
{

  String finalWeatherURL = buildURL(weatherURL, cityID, apiKey);
  getRequest(current, weatherServerName, finalWeatherURL.c_str());
}

/*******************************************************

 getWeatherForecast

 *******************************************************/
void getWeatherForecast(weather &current)
{

  String finalForecastURL = buildURL(forecastURL, cityID, apiKey);
  getRequest(current, weatherServerName, finalForecastURL);
}
/*******************************************************

 buildURL

 *******************************************************/
String buildURL(const char *baseURL, const char *city, const char *appID)
{

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
void getRequest(weather &current, String hostServer, String path)
{

  if ((WiFi.status() == WL_CONNECTED))
  {

    WiFiClient client;
    HTTPClient http;
    Serial.println("[HTTP] begin...");
    http.begin(client, hostServer, 80, path);
    Serial.print("[HTTP] GET ");
    Serial.println(path);

    int httpCode = http.GET();

    if (httpCode)
    {
      Serial.printf("[HTTP] Response code - %d\n", httpCode);

      // Response OK
      if (httpCode == 200)
      {
        StaticJsonDocument<2048> jsonDoc;

        WiFiClient *stream = http.getStreamPtr(); // get tcp stream

        // read all data from server
        while (http.connected())
        {
          if (stream->available())
          {
            String data = stream->readStringUntil('\r');
            data.trim();
            const char *lineChars = data.c_str();

            Serial.println(data);

            DeserializationError err = deserializeJson(jsonDoc, lineChars);

            if (!err)
            {

              Serial.println("weather data request sucess");

              JsonObject main = jsonDoc["main"];
              current.temp = lrint(main["temp"]);

              JsonObject wind = jsonDoc["wind"];

              current.windSpeed = wind["speed"];
              current.windDegree = wind["deg"];

              JsonObject weather = jsonDoc["weather"][0];
              current.conditions = weather["main"];
              break;
            }
            else
            {
              Serial.println("parse fail");
            }
          }
          delay(0);
        }

        delay(0);
        Serial.println(ESP.getFreeHeap());
        Serial.println();
        Serial.print("[HTTP] connection closed.\n");
      }
      else if (httpCode == -1)
      {
        Serial.println("connect rejected?");
      }
    }
    else
    {
      Serial.print("[HTTP] GET... failed, no connection or no HTTP server\n");
    }
    http.end();
  }
}