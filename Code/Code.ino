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
/*
 ______              _______  _______  _______  _______  _______  _       _________  __________________ _______  _______   
/ ___  \            (  ____ \(  ____ \(  ____ \(       )(  ____ \( (    /|\__   __/  \__   __/\__   __/(       )(  ____ \  
\/   )  )           | (    \/| (    \/| (    \/| () () || (    \/|  \  ( |   ) (        ) (      ) (   | () () || (    \/  
    /  /    _____   | (_____ | (__    | |      | || || || (__    |   \ | |   | |        | |      | |   | || || || (__      
   /  /    (_____)  (_____  )|  __)   | | ____ | |(_)| ||  __)   | (\ \) |   | |        | |      | |   | |(_)| ||  __)     
  /  /                    ) || (      | | \_  )| |   | || (      | | \   |   | |        | |      | |   | |   | || (        
 /  /               /\____) || (____/\| (___) || )   ( || (____/\| )  \  |   | |        | |   ___) (___| )   ( || (____/\  
 \_/                \_______)(_______/(_______)|/     \|(_______/|/    )_)   )_(        )_(   \_______/|/     \|(_______/  
                                                                                                                           
    _       ______   _______ _________ _______       _                _______  _______ _________          _______  _______ 
   ( )     (  __  \ (  ___  )\__   __/(  ____ \     ( )     |\     /|(  ____ \(  ___  )\__   __/|\     /|(  ____ \(  ____ )
   | |     | (  \  )| (   ) |   ) (   | (    \/     | |     | )   ( || (    \/| (   ) |   ) (   | )   ( || (    \/| (    )|
 __| |__   | |   ) || (___) |   | |   | (__       __| |__   | | _ | || (__    | (___) |   | |   | (___) || (__    | (____)|
(__   __)  | |   | ||  ___  |   | |   |  __)     (__   __)  | |( )| ||  __)   |  ___  |   | |   |  ___  ||  __)   |     __)
   | |     | |   ) || (   ) |   | |   | (           | |     | || || || (      | (   ) |   | |   | (   ) || (      | (\ (   
   | |     | (__/  )| )   ( |   | |   | (____/\     | |     | () () || (____/\| )   ( |   | |   | )   ( || (____/\| ) \ \__
   (_)     (______/ |/     \|   )_(   (_______/     (_)     (_______)(_______/|/     \|   )_(   |/     \|(_______/|/   \__/    

*/
// ###########################################################################################################################################
// # Includes:
// #
// # You will need to add ALL of the following libraries to your Arduino IDE to use the project:
// #
// # - AsyncTCP               // by me-no-dev:                    https://github.com/me-no-dev/AsyncTCP
// # - ESPAsyncWebServer      // by me-no-dev:                    https://github.com/me-no-dev/ESPAsyncWebServer
// # - ESPUI                  // by s00500:                       https://github.com/s00500/ESPUI
// # - ArduinoJson            // by bblanchon:                    https://github.com/bblanchon/ArduinoJson
// # - ArduinoJSON            // by Arduino:                      https://github.com/arduino-libraries/Arduino_JSON
// # - LITTLEFS               // by lorol:                        https://github.com/lorol/LITTLEFS
// #
// ###########################################################################################################################################
#include <WiFi.h>               // Built-in
#include "LedControl.h"         // Built-in
#include <HTTPClient.h>         // Built-in
#include <AsyncTCP.h>           // Used for the internal web server
#include <ESPAsyncWebServer.h>  // Used for the internal web server
#include <DNSServer.h>          // Used for the internal web server
#include <ESPUI.h>              // Used for the internal web server
#include "esp_log.h"            // Disable WiFi debug warnings
#include <Preferences.h>        // Used to save the configuration to the ESP32 flash
#include <Arduino_JSON.h>       // https://github.com/arduino-libraries/Arduino_JSON

// ###########################################################################################################################################
// # Version number of the code:
// ###########################################################################################################################################
const char* CLOCK_VERSION = "V2.1.0";

// ###########################################################################################################################################
// Hardware settings:
// ###########################################################################################################################################
LedControl lcA = LedControl(27, 14, 12, 1);  // DataIn pin, CLK pin, CS load pin, amount of MAX72XX displays = 1! --> Display A
LedControl lcB = LedControl(33, 25, 26, 1);  // DataIn pin, CLK pin, CS load pin, amount of MAX72XX displays = 1! --> Display B
LedControl lcC = LedControl(15, 13, 32, 1);  // DataIn pin, CLK pin, CS load pin, amount of MAX72XX displays = 1! --> Display C

// ###########################################################################################################################################
// # Internal web server settings:
// ###########################################################################################################################################
AsyncWebServer server(80);  // Web server for config
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;

// ###########################################################################################################################################
// # Declartions and variables used in the functions:
// ###########################################################################################################################################
Preferences preferences;
#define DEFAULT_AP_NAME "7-Segment"  // WiFi access point name of the ESP32
const char* hostname = "7Segment";   // Hostname to be set in your router
unsigned long previousMillis = 0;
const long interval = 1000;
int iHour, iMinute, iSecond, iDay, iMonth, iYear, iTempInt1, iTempInt2;
double iTemp, iHum;
bool changedvalues = false;
bool WiFIsetup = false;
int intensity, intensity_day, intensity_night, usenightmode, day_time_start, day_time_stop, statusNightMode;
int statusLabelID, statusNightModeID, DayNightSectionID, LEDsettingsSectionID, sliderBrightnessDayID, switchNightModeID, sliderBrightnessNightID, call_day_time_startID, call_day_time_stopID;
String iStartTime = " ";
int maxWiFiconnctiontries = 30;  // Maximum connection tries to logon to the set WiFi. After the amount of tries is reached the WiFi settings will be deleted!
String openWeatherMapApiKey = "";
String city = "";
String countryCode = "";
String weatherunits = "";
String DateFormat = "";
String Timezone = "";
String NTPserver = "";

// ###########################################################################################################################################
// Weather data values:
// ###########################################################################################################################################
unsigned long lastTime = 0;
unsigned long timerDelay = 5;  // Timer set to 5 minutes (60000 * 5) --> Do not set it to low or you might get banned from the service!
String jsonBuffer;

// ###########################################################################################################################################
// Startup actions:
// ###########################################################################################################################################
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("######################################################################");
  Serial.print("# Startup of version: ");
  Serial.println(CLOCK_VERSION);
  Serial.println("######################################################################");

  preferences.begin("7segment", false);  // Init ESP32 flash
  getFlashValues();                      // Read settings from flash

  // The MAX72XX is in power-saving mode on startup, we have to do a wakeup call:
  lcA.shutdown(0, false);
  lcB.shutdown(0, false);
  lcC.shutdown(0, false);

  // Clear the display:
  lcA.clearDisplay(0);
  lcB.clearDisplay(0);
  lcC.clearDisplay(0);

  // Connect to WiFi:
  intensity = 8;  // Default intensity before getting the time
  lcA.setIntensity(0, intensity);
  lcB.setIntensity(0, intensity);
  lcC.setIntensity(0, intensity);
  WIFI_SETUP();  // WiFi login and startup of web services and get weather
}

// ###########################################################################################################################################
// Loop actions during runtime:
// ###########################################################################################################################################
void loop() {
  if (WiFIsetup == true) {
    // Update the displays:
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Show time and date each second:
      printLocalTime();  // Locally get the time (NTP server requests done 1x per hour)

      // Display intensity:
      if (usenightmode == 1) {
        if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
          intensity = intensity_day;
          if ((iHour == 0) && (day_time_stop == 23)) intensity = intensity_night;  // Special function if day_time_stop set to 23 and time is 24, so 0...
        } else {
          intensity = intensity_night;
        }
      } else {
        intensity = intensity_day;
      }
      lcA.setIntensity(0, intensity);
      lcB.setIntensity(0, intensity);
      lcC.setIntensity(0, intensity);

      // Write on the display:
      writeArduinoOn7Segment();
    }
    // Get weather data:
    if ((millis() - lastTime) > timerDelay * 60000) getCurrentWeather();
  }
  if (changedvalues == true) setFlashValues();  // Write settings to flash
}

// ###########################################################################################################################################
// # Setup the internal web server configuration page:
// ###########################################################################################################################################
void setupWebInterface() {
  dnsServer.start(DNS_PORT, "*", apIP);

  // Section General:
  // ################
  ESPUI.separator("General:");

  // Status label:
  statusLabelID = ESPUI.label("Status:", ControlColor::Dark, "Operational");

  // Version:
  ESPUI.label("Version", ControlColor::None, CLOCK_VERSION);


  // Section LED night mode settings:
  // ################################
  DayNightSectionID = ESPUI.separator("Day/Night LED brightness mode settings:");

  // Use night mode function:
  switchNightModeID = ESPUI.switcher("Use night mode to reduce brightness", &switchNightMode, ControlColor::Dark, usenightmode);

  // Intensity DAY slider selector:
  sliderBrightnessDayID = ESPUI.slider("Brightness during the day", &sliderBrightnessDay, ControlColor::Dark, intensity_day, 0, 15);

  // Intensity NIGHT slider selector:
  sliderBrightnessNightID = ESPUI.slider("Brightness at night", &sliderBrightnessNight, ControlColor::Dark, intensity_night, 0, 15);

  // Night mode status:
  statusNightModeID = ESPUI.label("Night mode status", ControlColor::Dark, "Night mode not used");

  // Day mode start time:
  call_day_time_startID = ESPUI.number("Day time starts at", call_day_time_start, ControlColor::Dark, day_time_start, 0, 11);

  // Day mode stop time:
  call_day_time_stopID = ESPUI.number("Day time ends after", call_day_time_stop, ControlColor::Dark, day_time_stop, 12, 23);



  // Section WiFi:
  // #############
  ESPUI.separator("WiFi:");

  // WiFi SSID:
  ESPUI.label("SSID", ControlColor::Dark, WiFi.SSID());

  // WiFi signal strength:
  ESPUI.label("Signal", ControlColor::Dark, String(WiFi.RSSI()) + "dBm");

  // Hostname:
  ESPUI.label("Hostname", ControlColor::Dark, hostname);

  // WiFi ip-address:
  ESPUI.label("IP-address", ControlColor::Dark, IpAddress2String(WiFi.localIP()));

  // WiFi MAC-address:
  ESPUI.label("MAC address", ControlColor::Dark, WiFi.macAddress());



  // Section Time settings:
  // ######################
  ESPUI.separator("Time settings:");

  // NTP server:
  ESPUI.label("NTP server", ControlColor::Dark, NTPserver);

  // Time zone:
  ESPUI.label("Time zone", ControlColor::Dark, Timezone);

  // Date format:
  ESPUI.label("Date format", ControlColor::Dark, DateFormat);



  // Section Weather settings:
  // ######################
  ESPUI.separator("Weather settings:");

  // City:
  ESPUI.label("City", ControlColor::Dark, city);

  // Country code server:
  ESPUI.label("Country code", ControlColor::Dark, countryCode);

  // Weather units:
  ESPUI.label("Weather units", ControlColor::Dark, weatherunits);

  // Get new data every X minutes:
  ESPUI.label("Weather data updates every", ControlColor::Dark, String(timerDelay) + " minutes");

  // AppID server:
  ESPUI.label("Your OpenWeatherMap AppID", ControlColor::Dark, openWeatherMapApiKey);



  // Section Maintenance:
  // ####################
  ESPUI.separator("Maintenance:");

  // Restart Clock:
  ESPUI.button("Restart", &buttonRestart, ControlColor::Dark, "Restart", (void*)1);

  // Reset Clock settings:
  ESPUI.button("Reset ALL settings", &buttonClockReset, ControlColor::Dark, "Reset ALL clock settings", (void*)3);



  // Update night mode status text on startup:
  if (usenightmode == 1) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      ESPUI.print(statusNightModeID, "Day time");
      if ((iHour == 0) && (day_time_stop == 23)) ESPUI.print(statusNightModeID, "Night time");  // Special function if day_time_stop set to 23 and time is 24, so 0...
    } else {
      ESPUI.print(statusNightModeID, "Night time");
    }
  }

  // Deploy the page:
  ESPUI.begin("7-Segment Clock");
}

// ###########################################################################################################################################
// # Read settings from flash:
// ###########################################################################################################################################
void getFlashValues() {
  intensity_day = preferences.getUInt("intensity_day", 8);
  intensity_night = preferences.getUInt("intensity_night", 4);
  usenightmode = preferences.getUInt("usenightmode", 1);
  day_time_start = preferences.getUInt("day_time_start", 7);
  day_time_stop = preferences.getUInt("day_time_stop", 22);
  openWeatherMapApiKey = preferences.getString("AppID", "");
  weatherunits = preferences.getString("Units", "M");
  city = preferences.getString("City", "");
  countryCode = preferences.getString("Code", "");
  DateFormat = preferences.getString("Date", "");
  Timezone = preferences.getString("TimeZone", "");
  NTPserver = preferences.getString("TimeServer", "");
  openWeatherMapApiKey = preferences.getString("AppID", "");
  city = preferences.getString("City", "");
  countryCode = preferences.getString("Code", "");
  weatherunits = preferences.getString("Units", "");
  DateFormat = preferences.getString("Date", "");
}

// ###########################################################################################################################################
// # Write settings to flash:
// ###########################################################################################################################################
void setFlashValues() {
  changedvalues = false;
  preferences.putUInt("intensity_day", intensity_day);
  preferences.putUInt("intensity_night", intensity_night);
  preferences.putUInt("usenightmode", usenightmode);
  preferences.putUInt("day_time_start", day_time_start);
  preferences.putUInt("day_time_stop", day_time_stop);
  if (usenightmode == 1) {
    if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
      ESPUI.print(statusNightModeID, "Day time");
      if ((iHour == 0) && (day_time_stop == 23)) ESPUI.print(statusNightModeID, "Night time");  // Special function if day_time_stop set to 23 and time is 24, so 0...
    } else {
      ESPUI.print(statusNightModeID, "Night time");
    }
  } else {
    ESPUI.print(statusNightModeID, "Night mode not used");
  }
}

// ###########################################################################################################################################
// # GUI: Restart the Clock:
// ###########################################################################################################################################
void buttonRestart(Control* sender, int type, void* param) {
  if (changedvalues == true) setFlashValues();  // Write settings to flash
  preferences.end();
  delay(250);
  ESP.restart();
}

// ###########################################################################################################################################
// # GUI: Reset the Clock settings:
// ###########################################################################################################################################
void buttonClockReset(Control* sender, int type, void* param) {
  Serial.println("Status: CLOCK SETTINGS RESET REQUEST EXECUTED");
  preferences.clear();
  delay(100);
  WiFi.disconnect();  // DISCONNECT FROM WIFI
  delay(1000);
  preferences.end();
  Serial.println("####################################################################################################");
  Serial.println("# CLOCK SETTINGS WERE SET TO DEFAULT... CLOCK WILL NOW RESTART... PLEASE CONFIGURE AGAIN...        #");
  Serial.println("####################################################################################################");
  delay(250);
  ESP.restart();
}

// ###########################################################################################################################################
// # GUI: Night mode switch:
// ###########################################################################################################################################
void switchNightMode(Control* sender, int value) {
  switch (value) {
    case S_ACTIVE:
      usenightmode = 1;
      if ((iHour >= day_time_start) && (iHour <= day_time_stop)) {
        intensity = intensity_day;
        if ((iHour == 0) && (day_time_stop == 23)) intensity = intensity_night;  // Special function if day_time_stop set to 23 and time is 24, so 0...
      } else {
        intensity = intensity_night;
      }
      break;
    case S_INACTIVE:
      intensity = intensity_day;
      usenightmode = 0;
      break;
  }
  changedvalues = true;
}

// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: DAY
// ###########################################################################################################################################
void sliderBrightnessDay(Control* sender, int type) {
  intensity_day = sender->value.toInt();
  changedvalues = true;
}

// ###########################################################################################################################################
// # GUI: Slider change for LED intensity: NIGHT
// ###########################################################################################################################################
void sliderBrightnessNight(Control* sender, int type) {
  intensity_night = sender->value.toInt();
  changedvalues = true;
}

// ###########################################################################################################################################
// # GUI: Time Day Mode Start
// ###########################################################################################################################################
void call_day_time_start(Control* sender, int type) {
  day_time_start = sender->value.toInt();
  changedvalues = true;
}

// ###########################################################################################################################################
// # GUI: Time Day Mode Stop
// ###########################################################################################################################################
void call_day_time_stop(Control* sender, int type) {
  day_time_stop = sender->value.toInt();
  changedvalues = true;
}

// ###########################################################################################################################################
// # Wifi scan function to help you to setup your WiFi connection
// ###########################################################################################################################################
void ScanWiFi() {
  Serial.println("Scan WiFi networks - START");
  int n = WiFi.scanNetworks();
  Serial.println("WiFi scan done");
  Serial.println(" ");
  if (n == 0) {
    Serial.println("No WiFi networks found");
  } else {
    Serial.print(n);
    Serial.println(" WiFi networks found:");
    Serial.println(" ");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("Scan WiFi networks - END");
}

// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char index_html[] PROGMEM = R"=====(
  <!DOCTYPE html><html><head><title>7-Segment Clock</title></head>
          <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
  
  <body>
    <form action="/start" name="myForm">
      <center><b><h1>Welcome to the 7-Segment Clock setup</h1></b>
      <h2>Please add your local WiFi credentials and other settings on the next page</h2><br/>
      <input type="submit" value="Configure 7-Segment Clock" class="button">
     </center></form></body>
  </html>
 )=====";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char config_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE HTML><html><head><title>7-Segment Clock</title>
 <meta name="viewport" content="width=device-width, initial-scale=1">
  <script language="JavaScript">
  <!--
  function validateForm() {
  var x = document.forms["myForm"]["mySSID"].value;
  if (x == "") {
    alert("WiFi SSID must be set");
    return false;
  }
  var y = document.forms["myForm"]["myPW"].value;
  if (y == "") {
    alert("WiFi password must be set");
    return false;
  }
  var y = document.forms["myForm"]["myAppID"].value;
  if (y == "") {
    alert("AppID must be set");
    return false;
  }
  var y = document.forms["myForm"]["myCity"].value;
  if (y == "") {
    alert("City must be set");
    return false;
  }
  var y = document.forms["myForm"]["myCode"].value;
  if (y == "") {
    alert("Country code must be set");
    return false;
  }
  var y = document.forms["myForm"]["myUnits"].value;
  if (y == "") {
    alert("Unit format must be set");
    return false;
  }
  var y = document.forms["myForm"]["myDate"].value;
  if (y == "") {
    alert("Date format must be set");
    return false;
  }
  var y = document.forms["myForm"]["myServer"].value;
  if (y == "") {
    alert("NTP time server must be set");
    return false;
  }
  var y = document.forms["myForm"]["myZone"].value;
  if (y == "") {
    alert("Time zone must be set");
    return false;
  }
  } 
  //-->
  </script>
  </head>
  
   <style>
      body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
      }
      </style>
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
  
   <style>
    .button {
      display: inline-block;
      padding: 15px 25px;
      font-size: 24px;
      cursor: pointer;
      text-align: center;
      text-decoration: none;
      outline: none;
      color: #fff;
      background-color: #4CAF50;
      border: none;
      border-radius: 15px;
      box-shadow: 0 9px #999;
    }
    .button:hover {background-color: #3e8e41}
    .button:active {
      background-color: #3e8e41;
      box-shadow: 0 5px #666;
      transform: translateY(4px);
    }
    </style>
  
  <body>
  <form action="/get" name="myForm" onsubmit="return validateForm()" >
    <center><b><h1>Initial Clock setup:</h1></b>
    <label for="mySSID">Enter your WiFi SSID:</label><br/>
    <input type="text" id="mySSID" name="mySSID" value="" style="width: 200px;" /><br/><br/>
    <label for="myPW">Enter your WiFi password:</label><br/>
    <input type="text" id="myPW" name="myPW" value="" style="width: 200px;" /><br/><br/>
    <label for="myAppID">Enter your API key: (Get your own "Free" API key for "Current Weather Data" from <a href="https://openweathermap.org/api">OpenWeatherMap.org/api</a> >>> <a href="https://home.openweathermap.org/users/sign_up">Free API key signup</a>)</label><br/>
    <input type="text" id="myAppID" name="myAppID" value="" style="width: 200px;" /><br/><br/>
    <label for="myCity">Enter your City: (Find your own one on <a href="https://openweathermap.org">OpenWeatherMap.org</a>)</label><br/>
    <input type="text" id="myCity" name="myCity" value="" style="width: 200px;" /><br/><br/>
    <label for="myCode">Enter your country "Alpha2 code" (DE, GB or US): (<a href="https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes">WikiPedia</a>)</label><br/>
    <input type="text" id="myCode" name="myCode" value="DE" style="width: 200px;" /><br/><br/>
     <label for="myUnits">Select your unit format:</label><br/>
    <select id="myUnits" name="myUnits" style="width: 200px;">
    <option value="M" selected>Celsius</option>
    <option value="I">Fahrenheit</option>
    </select><br/><br/>
    <label for="myDate">Select your date format:</label><br/>
    <select id="myDate" name="myDate" style="width: 200px;">
    <option value="DDMMYYYY" selected>DD.MM.YYYY</option>
    <option value="MMDDYYYY">MM.DD.YYYY</option>
    </select><br/><br/>
    <label for="myServer">Enter your NTP time server:</label><br/>
    <input type="text" id="myServer" name="myServer" value="pool.ntp.org" style="width: 200px;" /><br/><br/>
    <label for="myZone">Enter your tome zone: (<a href="https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv">GitHub</a>)</label><br/>
    <input type="text" id="myZone" name="myZone" value="CET-1CEST,M3.5.0,M10.5.0/3" style="width: 200px;" /><br/><br/>
    <input type="submit" value="Save values and start 7-Segment Clock" class="button">
  </center></form></body></html>)rawliteral";


// ###########################################################################################################################################
// # Captive Portal web page to setup the device by AWSW:
// ###########################################################################################################################################
const char saved_html[] PROGMEM = R"rawliteral(
 <!DOCTYPE HTML><html><head>
  <title>Initial 7-Segment Clock setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1"></head>
    <style>
  body {
      padding: 25px;
      font-size: 25px;
      background-color: black;
      color: white;
    }
  </style>
  <body>
    <center><h2><b>Settings saved...<br><br>
    Clock will now try to connect to the named WiFi.<br>
    If it failes please try to connect to the temporary access point again.<br>
    Please close this page now and enjoy your 7-Segment Clock. =)</h2></b>
 </body></html>)rawliteral";


// ###########################################################################################################################################
// # Captive Portal by AWSW to avoid the usage of the WiFi Manager library to have more control
// ###########################################################################################################################################
const char* PARAM_INPUT_1 = "mySSID";
const char* PARAM_INPUT_2 = "myPW";
const char* PARAM_INPUT_3 = "myAppID";
const char* PARAM_INPUT_4 = "myCity";
const char* PARAM_INPUT_5 = "myCode";
const char* PARAM_INPUT_6 = "myUnits";
const char* PARAM_INPUT_7 = "myDate";
const char* PARAM_INPUT_8 = "myServer";
const char* PARAM_INPUT_9 = "myZone";
const String captiveportalURL = "http://192.168.4.1";
void CaptivePotalSetup() {
  // Display anination:
  for (int i = 0; i <= 7; i++) {
    lcA.setChar(0, i, 0, true);
    lcB.setChar(0, i, 0, true);
    lcC.setChar(0, i, 0, true);
    delay(100);
  }
  ScanWiFi();
  const char* temp_ssid = "7-Segment";
  const char* temp_password = "";
  WiFi.softAP(temp_ssid, temp_password);
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println("#################################################################################################################################################################################");
  Serial.print("# Temporary WiFi access point initialized. Please connect to the WiFi access point now and set your local WiFi credentials. Access point name: ");
  Serial.println(temp_ssid);
  Serial.print("# In case your browser does not open the 7-Segment Clock setup page automatically after connecting to the access point, please navigate to this URL manually to http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("#################################################################################################################################################################################");
  Serial.println(" ");
  Serial.println(" ");
  Serial.println(" ");
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      // Serial.println(inputMessage);
      preferences.putString("WIFIssid", inputMessage);  // Save entered WiFi SSID
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
      // Serial.println(inputMessage);
      preferences.putString("WIFIpass", inputMessage);  // Save entered WiFi password
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      // Serial.println(inputMessage);
      preferences.putString("AppID", inputMessage);  // Save entered AppID
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      // Serial.println(inputMessage);
      preferences.putString("City", inputMessage);  // Save entered City
      inputMessage = request->getParam(PARAM_INPUT_5)->value();
      inputParam = PARAM_INPUT_5;
      // Serial.println(inputMessage);
      preferences.putString("Code", inputMessage);  // Save entered Country Code
      inputMessage = request->getParam(PARAM_INPUT_6)->value();
      inputParam = PARAM_INPUT_6;
      // Serial.println(inputMessage);
      preferences.putString("Units", inputMessage);  // Save entered unit format
      inputMessage = request->getParam(PARAM_INPUT_7)->value();
      inputParam = PARAM_INPUT_7;
      Serial.println(inputMessage);
      preferences.putString("Date", inputMessage);  // Save entered date format
      inputMessage = request->getParam(PARAM_INPUT_8)->value();
      inputParam = PARAM_INPUT_8;
      // Serial.println(inputMessage);
      preferences.putString("TimeServer", inputMessage);  // Save entered time server
      inputMessage = request->getParam(PARAM_INPUT_9)->value();
      inputParam = PARAM_INPUT_9;
      // Serial.println(inputMessage);
      preferences.putString("TimeZone", inputMessage);  // Save entered time zone
      delay(250);
      preferences.end();
    } else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    request->send_P(200, "text/html", saved_html);
    delay(1000);
    ESP.restart();
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", config_html);
  });

  server.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("msftconnecttest.com", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/fwlink", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/wpad.dat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/generate_204", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/redirect", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/canonical.html", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/success.txt", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/ncsi.txt", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/service/update2/json", [](AsyncWebServerRequest* request) {
    request->send(200);
  });
  server.on("/chat", [](AsyncWebServerRequest* request) {
    request->send(404);
  });
  server.on("/startpage", [](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
  });
  server.on("/favicon.ico", [](AsyncWebServerRequest* request) {
    request->send(404);
  });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest* request) {
    AsyncWebServerResponse* response = request->beginResponse(200, "text/html", index_html);
    response->addHeader("Cache-Control", "public,max-age=31536000");
    request->send(response);
    Serial.println("Served Basic HTML Page");
  });

  server.onNotFound([](AsyncWebServerRequest* request) {
    request->redirect(captiveportalURL);
    Serial.print("onnotfound ");
    Serial.print(request->host());
    Serial.print(" ");
    Serial.print(request->url());
    Serial.print(" sent redirect to " + captiveportalURL + "\n");
  });

  server.begin();
  Serial.println("Clock Captive Portal web server started");
}

// ###########################################################################################################################################
// # Wifi setup and reconnect function that runs once at startup and during the loop function of the ESP:
// ###########################################################################################################################################
void WIFI_SETUP() {
  Serial.println(" ");
  esp_log_level_set("wifi", ESP_LOG_WARN);  // Disable WiFi debug warnings
  String WIFIssid = preferences.getString("WIFIssid");
  bool WiFiConfigEmpty = false;
  if (WIFIssid == "") {
    // Serial.println("WIFIssid empty");
    WiFiConfigEmpty = true;
  } else {
    // Serial.print("WIFIssid = ");
    // Serial.println(WIFIssid);
  }
  String WIFIpass = preferences.getString("WIFIpass");
  if (WIFIpass == "") {
    // Serial.println("WIFIpass empty");
    WiFiConfigEmpty = true;
  } else {
    // Serial.print("WIFIpass = ");
    // Serial.println(WIFIpass);
  }
  if (WiFiConfigEmpty == true) {
    CaptivePotalSetup();
  } else {
    Serial.println("Try to connect to found WiFi configuration: ");
    WiFi.disconnect();
    int tryCount = 0;
    WiFi.mode(WIFI_STA);
    WiFi.begin((const char*)WIFIssid.c_str(), (const char*)WIFIpass.c_str());
    Serial.println("Connecting to WiFi " + String(WIFIssid));
    while (WiFi.status() != WL_CONNECTED) {
      tryCount = tryCount + 1;
      Serial.print("WiFI connection try #: ");
      Serial.print(tryCount);
      Serial.println(" of " + String(maxWiFiconnctiontries));
      // Display anination:
      for (int i = 0; i <= 7; i++) {
        lcA.setChar(0, 7 - i, 7 - i, true);
        lcB.setChar(0, i, i, true);
        lcC.setChar(0, 7 - i, 7 - i, true);
        delay(100);
      }
      // Clear the display:
      lcA.clearDisplay(0);
      lcB.clearDisplay(0);
      lcC.clearDisplay(0);
      delay(100);
      if (tryCount == maxWiFiconnctiontries) {
        Serial.println("\n\nWIFI CONNECTION ERROR: If the connection still can not be established please check the WiFi settings or location of the device.\n\n");
        preferences.putString("WIFIssid", "");  // Reset entered WiFi ssid
        preferences.putString("WIFIpass", "");  // Reset entered WiFi password
        preferences.end();
        delay(250);
        Serial.println("WiFi settings deleted because in " + String(maxWiFiconnctiontries) + " tries the WiFi connection could not be established. Temporary Clock access point will be started to reconfigure WiFi again.");
        ESP.restart();
      }
      delay(500);
    }
    Serial.println(" ");
    WiFIsetup = true;
    Serial.print("Successfully connected now to WiFi SSID: ");
    Serial.println(WiFi.SSID());
    Serial.println("IP: " + WiFi.localIP().toString());
    Serial.println("DNS: " + WiFi.dnsIP().toString());
    delay(1000);
    ShowIPaddress();      // Display the current IP-address
    configNTPTime();      // NTP time setup
    setupWebInterface();  // Generate the configuration page
    getCurrentWeather();  // Get weather data
    Serial.println("######################################################################");
    Serial.println("# Web interface online at: http://" + IpAddress2String(WiFi.localIP()));
    Serial.println("######################################################################");
    Serial.println("# 7-Segment Clock startup finished...");
    Serial.println("######################################################################");
    Serial.println(" ");
  }
}

// ###########################################################################################################################################
// Write on the display:
// ###########################################################################################################################################
void writeArduinoOn7Segment() {
  if (intensity > 0) {  // Display intensity set > 0 --> Display on:
    // Leave the MAX72XX power-saving mode:
    lcA.shutdown(0, false);
    lcB.shutdown(0, false);
    lcC.shutdown(0, false);
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
    if (DateFormat == "DDMMYYYY") {
      lcB.setChar(0, 7, iDay / 10, false);
      lcB.setChar(0, 6, iDay % 10, true);
      lcB.setChar(0, 5, iMonth / 10, false);
      lcB.setChar(0, 4, iMonth % 10, true);
      lcB.setChar(0, 3, (iYear / 1000) % 10, false);
      lcB.setChar(0, 2, (iYear / 100) % 10, false);
      lcB.setChar(0, 1, (iYear / 10) % 10, false);
      lcB.setChar(0, 0, iYear % 10, false);
    }
    if (DateFormat == "MMDDYYYY") {
      lcB.setChar(0, 7, iMonth / 10, false);
      lcB.setChar(0, 6, iMonth % 10, true);
      lcB.setChar(0, 5, iDay / 10, false);
      lcB.setChar(0, 4, iDay % 10, true);
      lcB.setChar(0, 3, (iYear / 1000) % 10, false);
      lcB.setChar(0, 2, (iYear / 100) % 10, false);
      lcB.setChar(0, 1, (iYear / 10) % 10, false);
      lcB.setChar(0, 0, iYear % 10, false);
    }
    // Display C:
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
  } else {  // Display intensity set to 0 --> Display off:
    lcA.clearDisplay(0);
    lcB.clearDisplay(0);
    lcC.clearDisplay(0);
    // Set the MAX72XX in power-saving mode:
    lcA.shutdown(0, true);
    lcB.shutdown(0, true);
    lcC.shutdown(0, true);
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
  // Clear the display:
  lcA.clearDisplay(0);
  lcB.clearDisplay(0);
  lcC.clearDisplay(0);
  struct tm timeinfo;
  Serial.println("Setting up time");
  // Display anination:
  for (int i = 8; i >= 0; i = i - 1) {
    lcB.setChar(0, i, '-', false);
    delay(100);
  }
  delay(100);
  lcB.clearDisplay(0);
  configTime(0, 0, NTPserver.c_str());
  while (!getLocalTime(&timeinfo)) {
    // Display anination:
    for (int i = 8; i >= 0; i = i - 1) {
      lcB.setChar(0, i, '-', false);
      delay(100);
    }
    lcB.clearDisplay(0);
    Serial.println("! Failed to obtain time - Time server could not be reached ! --> Try: " + String(TimeResetCounter) + " of 5...");
    TimeResetCounter = TimeResetCounter + 1;
    if (TimeResetCounter == 6) {
      // Display anination:
      for (int i = 8; i >= 0; i = i - 1) {
        lcB.setChar(0, i, '8', false);
        delay(100);
      }
      lcB.clearDisplay(0);
      delay(100);
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
  } else {
    Serial.println("WiFi Disconnected");
  }
  lastTime = millis();
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

// ###########################################################################################################################################
// # GUI: Convert IP-address value to string:
// ###########################################################################################################################################
String IpAddress2String(const IPAddress& ipAddress) {
  return String(ipAddress[0]) + String(".") + String(ipAddress[1]) + String(".") + String(ipAddress[2]) + String(".") + String(ipAddress[3]);
}

// ###########################################################################################################################################
// # Show the IP-address on the display:
// ###########################################################################################################################################
void ShowIPaddress() {
  Serial.println("Show current IP-address on the display: " + IpAddress2String(WiFi.localIP()));

  // Display A:
  lcA.setChar(0, 7, ' ', false);
  lcA.setChar(0, 6, getDigit(WiFi.localIP()[0], 2), false);
  lcA.setChar(0, 5, getDigit(WiFi.localIP()[0], 1), false);
  lcA.setChar(0, 4, getDigit(WiFi.localIP()[0], 0), true);
  lcA.setChar(0, 3, getDigit(WiFi.localIP()[1], 2), false);
  lcA.setChar(0, 2, getDigit(WiFi.localIP()[1], 1), false);
  lcA.setChar(0, 1, getDigit(WiFi.localIP()[1], 0), true);
  lcA.setChar(0, 0, ' ', false);

  // Display B:
  lcB.setChar(0, 7, ' ', false);
  lcB.setChar(0, 6, getDigit(WiFi.localIP()[2], 2), false);
  lcB.setChar(0, 5, getDigit(WiFi.localIP()[2], 1), false);
  lcB.setChar(0, 4, getDigit(WiFi.localIP()[2], 0), true);
  lcB.setChar(0, 3, getDigit(WiFi.localIP()[3], 2), false);
  lcB.setChar(0, 2, getDigit(WiFi.localIP()[3], 1), false);
  lcB.setChar(0, 1, getDigit(WiFi.localIP()[3], 0), false);
  lcB.setChar(0, 0, ' ', false);

  delay(3000);
}

// ###########################################################################################################################################
// # Get a digit from a number at position pos: (Split IP-address octets in single digits)
// ###########################################################################################################################################
int getDigit(int number, int pos) {
  return (pos == 0) ? number % 10 : getDigit(number / 10, --pos);
}