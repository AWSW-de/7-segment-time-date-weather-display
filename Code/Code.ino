// ###########################################################################################################################################
// #
// # Code for the printables "7-segment time + date + weather display" project:
// # https://www.printables.com/de/model/507876-multi-7-segment-display-clock
// #
// # Code by https://github.com/AWSW-de
// #
// # Released under license: GNU General Public License v3.0: https://github.com/AWSW-de/7-segment-time-date-weather-display/blob/main/LICENSE
// #
// ###########################################################################################################################################

// Version: V1.0.0

// ###########################################################################################################################################
// # Includes:
// ###########################################################################################################################################
#include <WiFi.h>          // Built-in
#include "LedControl.h"    // Built-in
#include <HTTPClient.h>    // Built-in
#include <Arduino_JSON.h>  // https://github.com/arduino-libraries/Arduino_JSON
#include "settings.h"      // Settings file --> Change settings in this file only!

// ###########################################################################################################################################
// Hardware settings:
// ###########################################################################################################################################
LedControl lcA = LedControl(27, 14, 12, 1);  // DataIn pin, CLK pin, CS load pin, amount of MAX72XX displays = 1! --> Display A
LedControl lcB = LedControl(33, 25, 26, 1);  // DataIn pin, CLK pin, CS load pin, amount of MAX72XX displays = 1! --> Display B
LedControl lcC = LedControl(15, 13, 32, 1);  // DataIn pin, CLK pin, CS load pin, amount of MAX72XX displays = 1! --> Display C

// ###########################################################################################################################################
// Init values:
// ###########################################################################################################################################
unsigned long previousMillis = 0;
const long interval = 1000;
int display_brightness = 4;
int iHour, iMinute, iSecond, iDay, iMonth, iYear, iTempInt1, iTempInt2;
double iTemp, iHum;

// ###########################################################################################################################################
// Weather data values:
// ###########################################################################################################################################
unsigned long lastTime = 0;
unsigned long timerDelay = 600000;  // Timer set to 10 minutes (600000) --> Do not set it to low or you might get banned from the service!
String jsonBuffer;

// ###########################################################################################################################################
// Startup actions:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(1000);

  // The MAX72XX is in power-saving mode on startup, we have to do a wakeup call:
  lcA.shutdown(0, false);
  lcB.shutdown(0, false);
  lcC.shutdown(0, false);
  // Set the brightness of the display:
  lcA.setIntensity(0, display_brightness);
  lcB.setIntensity(0, display_brightness);
  lcC.setIntensity(0, display_brightness);

  // Connect to WiFi:
  WiFi.mode(WIFI_STA);  //Optional
  WiFi.begin(ssid, password);
  Serial.println("\nConnecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // Clear the display:
    lcA.clearDisplay(0);
    lcB.clearDisplay(0);
    lcC.clearDisplay(0);
    // Startup animation / Display test:
    for (int i = 0; i <= 7; i++) {
      lcA.setChar(0, 7 - i, 7 - i, true);
      lcB.setChar(0, i, i, true);
      lcC.setChar(0, 7 - i, 7 - i, true);
      delay(100);
    }
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  // NTP time:
  configNTPTime();

  // Clear the display:
  lcA.clearDisplay(0);
  lcB.clearDisplay(0);
  lcC.clearDisplay(0);

  // Get weather data:
  if ((city == "SetYourCityHere") || (openWeatherMapApiKey == "Your_API_Key")) {
    Serial.println("Weather data is not configured yet...");
  } else {
    getCurrentWeather();
  }
}

// ###########################################################################################################################################
// Loop actions during runtime:
// ###########################################################################################################################################
void loop() {
  // Update the displays:
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Show time and date each second:
    printLocalTime();  // Locally get the time (NTP server requests done 1x per hour)

    // Write on the display:
    writeArduinoOn7Segment();
  }
  // Get weather data:
  if ((millis() - lastTime) > timerDelay) getCurrentWeather();
}

// ###########################################################################################################################################
// Write on the display:
// ###########################################################################################################################################
void writeArduinoOn7Segment() {
  // Display A:
  lcA.setChar(0, 7, ' ', false);
  lcA.setChar(0, 6, iHour / 10, false);
  lcA.setChar(0, 5, iHour % 10, true);
  lcA.setChar(0, 4, iMinute / 10, false);
  lcA.setChar(0, 3, iMinute % 10, true);
  lcA.setChar(0, 2, iSecond / 10, false);
  lcA.setChar(0, 1, iSecond % 10, false);
  lcA.setChar(0, 0, ' ', false);
  // Display B:
  lcB.setChar(0, 7, iDay / 10, false);
  lcB.setChar(0, 6, iDay % 10, true);
  lcB.setChar(0, 5, iMonth / 10, false);
  lcB.setChar(0, 4, iMonth % 10, true);
  lcB.setChar(0, 3, (iYear / 1000) % 10, false);
  lcB.setChar(0, 2, (iYear / 100) % 10, false);
  lcB.setChar(0, 1, (iYear / 10) % 10, false);
  lcB.setChar(0, 0, iYear % 10, false);
  // Display C:
  if ((city == "SetYourCityHere") || (openWeatherMapApiKey == "Your_API_Key")) {
    // Serial.println("Weather data is not configured yet...");
  } else {
    if ((int)iTempInt1 > 99) lcC.setChar(0, 7, (((int)iTempInt1 / 100) % 10), false);
    else lcC.setChar(0, 7, ' ', false);
    if ((int)iTempInt1 > 9) lcC.setChar(0, 6, (((int)iTempInt1 / 10) % 10), false);
    else lcC.setChar(0, 6, ' ', false);
    lcC.setChar(0, 5, ((int)iTempInt1 % 10), true);
    lcC.setChar(0, 4, (((int)iTempInt2 / 10) % 10), false);
    lcC.setChar(0, 3, ' ', false);  // Split to seperate numbers
    if ((int)iHum > 99) lcC.setChar(0, 2, (((int)iHum / 100) % 10), false);
    else lcC.setChar(0, 2, ' ', false);
    if ((int)iHum > 9) lcC.setChar(0, 1, (((int)iHum / 10) % 10), false);
    else lcC.setChar(0, 1, ' ', false);
    lcC.setChar(0, 0, ((int)iHum % 10), false);
  }
}

// ###########################################################################################################################################
// # NTP time functions:
// ###########################################################################################################################################
void configNTPTime() {
  initTime(Timezone);
  printLocalTime();
}
// ###########################################################################################################################################
void setTimezone(String timezone) {
  Serial.printf("Setting timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}
// ###########################################################################################################################################
int TimeResetCounter = 1;
void initTime(String timezone) {
  struct tm timeinfo;
  Serial.println("Setting up time");
  configTime(0, 0, NTPserver);
  while (!getLocalTime(&timeinfo)) {
    delay(250);
    Serial.println("! Failed to obtain time - Time server could not be reached ! --> Try: " + String(TimeResetCounter) + " of 15...");
    TimeResetCounter = TimeResetCounter + 1;
    if (TimeResetCounter == 16) {
      Serial.println("! Failed to obtain time - Time server could not be reached ! --> RESTART THE DEVICE NOW...");
      ESP.restart();
    }
  }
  Serial.println("Got the time from NTP");
  setTimezone(timezone);
}
// ###########################################################################################################################################
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  iHour = timeinfo.tm_hour;
  iMinute = timeinfo.tm_min;
  iSecond = timeinfo.tm_sec;
  iDay = timeinfo.tm_mday;
  iMonth = timeinfo.tm_mon + 1;
  iYear = timeinfo.tm_year + 1900;
  // Serial.print("Time: ");
  // Serial.print(iHour);
  // Serial.print(":");
  // Serial.print(iMinute);
  // Serial.print(":");
  // Serial.print(iSecond);
  // Serial.print(" - Date: ");
  // Serial.print(iDay);
  // Serial.print(".");
  // Serial.print(iMonth);
  // Serial.print(".");
  // Serial.println(iYear);
}
// ###########################################################################################################################################
void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst) {
  struct tm tm;
  tm.tm_year = yr - 1900;  // Set date
  tm.tm_mon = month - 1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;  // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}
// ###########################################################################################################################################

// ###########################################################################################################################################
// # Get waether data:
// ###########################################################################################################################################
void getCurrentWeather() {
  if ((city == "SetYourCityHere") || (openWeatherMapApiKey == "Your_API_Key")) {
    // Serial.println("Weather data is not configured yet...");
  } else {
    // Send an HTTP GET request
    Serial.println("Get current weather data...");

    // Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      String serverPath = "";
      if (weatherunits == "M") serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=metric";    // Celsius
      if (weatherunits == "I") serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey + "&units=imperial";  // Fahrenheit

      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);

      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      Serial.print("JSON object = ");
      Serial.println(myObject);

      // Temperature:
      Serial.print("Temperature: ");
      Serial.print(myObject["main"]["temp"]);
      if (weatherunits == "M") Serial.println(" °C");
      if (weatherunits == "I") Serial.println(" °F");
      // Show the temperature number with its decimal part on the display:
      iTemp = double(myObject["main"]["temp"]);
      String dataTemp = String(iTemp);
      byte dotPositionTemp = dataTemp.indexOf('.');
      long integralPartTemp = dataTemp.toInt();
      long decimalPartTemp = dataTemp.substring(dotPositionTemp + 1).toInt();
      iTempInt1 = integralPartTemp;
      iTempInt2 = decimalPartTemp;

      // Humidity:
      Serial.print("Humidity: ");
      Serial.println(myObject["main"]["humidity"]);
      iHum = (long)(myObject["main"]["humidity"]);

      // Pressure:
      // Serial.print("Pressure: ");
      // Serial.println(myObject["main"]["pressure"]);

      // Wind Speed:
      // Serial.print("Wind Speed: ");
      // Serial.println(myObject["wind"]["speed"]);

    } else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}