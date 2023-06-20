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
// # WiFi settings:
// ###########################################################################################################################################
const char* ssid = "YourNetworkName";
const char* password = "YourNetworkPassword";

// ###########################################################################################################################################
// # Weather settings: Source:
// ###########################################################################################################################################
// Use your own API key by signing up for a free developer account at https://openweathermap.org/ --> https://openweathermap.org/appid/
// It's free to get an API key, but don't take more than 60 readings/minute!
String openWeatherMapApiKey = "Your_API_Key";  // See: https://openweathermap.org/ --> https://openweathermap.org/appid/
String weatherunits = "M";                     // Set to "M" for Metric "°C" Celsius values or to "I" for Imperial "°F" Fahrenheit values
String city = "SetYourCityHere";               // Your home city See: http://bulk.openweathermap.org/sample/
String countryCode = "DE";                     // Your _ISO-3166-1_two-letter_country_code country code, on OWM find your nearest city and the country code is displayed
                                               // https://en.wikipedia.org/wiki/List_of_ISO_3166_country_codes

// ###########################################################################################################################################
// # NTP time server settings:
// ###########################################################################################################################################
const char* Timezone = "CET-1CEST,M3.5.0,M10.5.0/3";  // You can check a list of timezone string variables here:  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
const char* NTPserver = "pool.ntp.org";               // Time server address. Choose the closest one to you here: https://gist.github.com/mutin-sa/eea1c396b1e610a2da1e5550d94b0453
