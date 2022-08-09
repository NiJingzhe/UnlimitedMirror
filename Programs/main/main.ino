#include <ArduinoOTA.h>
#include <Arduino.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include <WString.h>
#define LEDPin D5
#define TCHPin D6
#define MAX_BRIGHTNESS 200
#define LEDNumber 23
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

struct ConfigInfo {
  String ssid;
  String passwd;
  String cityName = "huzhou";
};

struct WeatherInfo {
  String nowWeather = "";
  String lastWeather = "";
  bool changing = false;
};

ConfigInfo configInfo;
WeatherInfo weatherInfo;

Ticker requestCtrl;
Ticker displayCtrl;
Ticker touchSwitchCtrl;
bool touched = false;
bool requestReady = false;
bool displayReady = false;
bool firstRequestDone = false;
int requestInterval = 1;
int requestCounter = 0;

ESP8266WebServer server;

WiFiClient client;
HTTPClient httpClient;
String API_head = "http://api.seniverse.com/v3/weather/now.json?key=SSXQT7-NvcQE3xxXn&location=";
String API_tail = "&language=zh-Hans&unit=c";

CRGB * displayInfo;

void startupConfig();
void LEDStripInit();
void LEDDisplay();
String readString();
void writeString();
void requestWeatherInfo();
void setPage();
void resultDisplay();

void setup() {
  Serial.begin(115200);
  pinMode(TCHPin, INPUT);
  EEPROM.begin(4096);
  startupConfig();
  LEDStripInit();
  WiFi.softAP("Mirror");
  server.on("/", setPage);
  server.on("/set_finished/", resultDisplay);
  server.begin();
  displayCtrl.attach_ms(20, []() {
    displayReady = firstRequestDone ? true : false;
  });
  requestCtrl.attach(requestInterval, []() {
    requestCounter++;
  });
  touchSwitchCtrl.attach_ms(10, []() {
    touched = digitalRead(TCHPin) == 1 ? true : false;
  });
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.setHostname("ESP8266");
    ArduinoOTA.setPassword("ilovenana");
    ArduinoOTA.begin();
  }
  displayInfo[0] = CRGB::Black;
  FastLED.show();
  FastLED.setBrightness(0);
  FastLED.show();
}


void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  if (requestCounter > requestInterval) {
    requestCounter = 0;
    requestWeatherInfo();
    requestReady = false;
  }

  while (touched) {
    displayReady = false;
    displayInfo[0] = CRGB::Black;
    FastLED.show();
    FastLED.setBrightness(0);
    FastLED.show();
    delay(10000);
  }

  if (displayReady && !touched) {
    requestInterval = 120;
    FastLED.setBrightness(200);
    LEDDisplay(weatherInfo.nowWeather);
    displayReady = false;
  }

}


//ckeck eeprom on startup
void startupConfig() {
  String ssid = readString(1);
  String passwd = readString(256);
  String cityName = readString(512);
  Serial.println(ssid);
  Serial.println(passwd);
  Serial.println(cityName);
  if (ssid != "" && passwd != "" && cityName != "") {
    configInfo.ssid = ssid;
    configInfo.passwd = passwd;
    configInfo.cityName = cityName;
    Serial.println(configInfo.ssid);
    Serial.println(configInfo.passwd);
    Serial.println(configInfo.cityName);
    WiFi.begin(configInfo.ssid, configInfo.passwd);
    Serial.print("Connecting");
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
      WiFi.begin(configInfo.ssid, configInfo.passwd);
    }
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("Connected!  ");
      Serial.println(WiFi.localIP());
    }

  }
  else {
    configInfo.ssid = "";
    configInfo.passwd = "";
    configInfo.cityName = "huzhou";
  }
}



//ROM operate function
String readString(int addr) {

  String result = "";

  if ((addr > 0) && (addr < 4096)) {
    int length = EEPROM.read(addr);
    for (int i = 1; i <= length; i++) {
      result += (char)EEPROM.read(addr + i);
    }
    return result;
  }
  else {
    return "";
  }
}

//灯效
void thunderShower() {
  Ticker fade;
  int twinkleInterval = random(100, 300);
  int fadeSpeed = 20;
  fade.attach_ms(20, [fadeSpeed] {fadeToBlackBy( displayInfo, LEDNumber, fadeSpeed); FastLED.show();});
  int pos = random8(LEDNumber);
  int pos2 = random8(LEDNumber);
  int pos3 = random8(LEDNumber);
  int pos4 = random8(LEDNumber);
  int pos5 = random8(LEDNumber);
  pos = pos < 1 ? 2 : pos;
  pos = pos >= 22 ? 21 : pos;

  if ( random8() < 230) {
    displayInfo[pos] = CHSV(150, 200, 100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos2] = CHSV(150, 200, 100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos3] = CHSV(150, 200, 100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos4] = CHSV(150, 200, 100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos5] = CHSV(150, 200, 100);
    FastLED.show();
    fadeSpeed = 50;
    fade.detach();
    fade.attach_ms(20, [fadeSpeed] {fadeToBlackBy( displayInfo, LEDNumber, fadeSpeed); FastLED.show();});
  }
  else {
    delay(80);
    displayInfo[pos] = CHSV(200, 100, 255);
    displayInfo[pos - 1] = CHSV(220, 50, 150);
    displayInfo[pos - 2] = CHSV(200, 20, 50);
    displayInfo[pos + 1] = CHSV(200, 50, 150);
    displayInfo[pos + 2] = CHSV(200, 20, 50);
    FastLED.show();
    fadeSpeed = 12;
    fade.detach();
    fade.attach_ms(20, [fadeSpeed] {fadeToBlackBy( displayInfo, LEDNumber, fadeSpeed); FastLED.show();});
    delay(80);
  }
  delay(twinkleInterval);
}



void rain(int level) {
  int twinkleInterval, fadeSpeed;
  int twinkleChance = 255;
  Ticker fade;
  switch (level) {
    case 0:
      twinkleInterval = random(4, 12) * 100;
      fadeSpeed = 40;
      break;
    case 1:
      twinkleInterval = random(100, 400);
      fadeSpeed = 20;
      break;
    case 2:
      twinkleInterval = 50;
      fadeSpeed = 20;
      break;
    case 3:
      twinkleInterval = 20;
      fadeSpeed = 10;
      break;
  }
  fade.attach_ms(20, [fadeSpeed] {fadeToBlackBy( displayInfo, LEDNumber, fadeSpeed); FastLED.show();});
  int pos = random8(LEDNumber);
  pos = pos < 1 ? 2 : pos;
  pos = pos >= 22 ? 21 : pos;

  if ( random8() < twinkleChance) {
    displayInfo[pos] = CHSV(150, 200, 100);
    FastLED.show();
  }
  delay(twinkleInterval);
}

void snow(int level) {
  int twinkleInterval, fadeSpeed;
  int twinkleChance = 255;
  Ticker fade;
  switch (level) {
    case 0:
      twinkleInterval = random(4, 12) * 100;
      fadeSpeed = 40;
      break;
    case 1:
      twinkleInterval = random(100, 400);
      fadeSpeed = 20;
      break;
    case 2:
      twinkleInterval = 50;
      fadeSpeed = 20;
      break;
    case 3:
      twinkleInterval = 20;
      fadeSpeed = 10;
      break;
  }
  fade.attach_ms(20, [fadeSpeed] {fadeToBlackBy( displayInfo, LEDNumber, fadeSpeed); FastLED.show();});
  int pos = random8(LEDNumber);
  pos = pos < 1 ? 2 : pos;
  pos = pos >= 22 ? 21 : pos;

  if ( random8() < 255) {
    displayInfo[pos] = CHSV(150, 80, 150);
    FastLED.show();
  }
  delay(twinkleInterval);
}

void warnningDust() {
  fill_solid(displayInfo, LEDNumber, CHSV(20, 250, 0));
  FastLED.show();
  for (int i = 1; i < 150; i++) {
    fill_solid(displayInfo, LEDNumber, CHSV(0, 250, i));
    delay(i / 10);
    FastLED.show();
  }
  for (int i = 1; i < 150; i++) {
    fill_solid(displayInfo, LEDNumber, CHSV(0, 250, 150 - i));
    delay((150 - i) / 10);
    FastLED.show();
  }
  fill_solid(displayInfo, LEDNumber, CHSV(20, 250, 0));
  FastLED.show();
}

void warnningWater() {
  fill_solid(displayInfo, LEDNumber, CHSV(20, 250, 0));
  FastLED.show();
  for (int i = 1; i < 150; i++) {
    fill_solid(displayInfo, LEDNumber, CHSV(180, 250, i));
    delay(i / 10);
    FastLED.show();
  }
  for (int i = 1; i < 150; i++) {
    fill_solid(displayInfo, LEDNumber, CHSV(180, 250, 150 - i));
    delay((150 - i) / 10);
    FastLED.show();
  }
  fill_solid(displayInfo, LEDNumber, CHSV(20, 250, 0));
  FastLED.show();
}

void sunny() {
  int pos = random8(LEDNumber);
  pos = pos < 1 ? 2 : pos;
  pos = pos >= 22 ? 21 : pos;
  fill_solid(displayInfo, LEDNumber, CHSV(20, 220, 60));
  FastLED.show();
  for (int i = 0; i < 100; i++) {
    displayInfo[pos] = CHSV(20, 220, 60 + i);
    displayInfo[pos + 1] = CHSV(20, 220, 60 + i * 0.6);
    displayInfo[pos - 1] = CHSV(20, 220, 60 + i * 0.6);
    displayInfo[pos - 2] = CHSV(20, 220, 60 + i * 0.4);
    displayInfo[pos + 2] = CHSV(20, 220, 60 + i * 0.4);
    FastLED.show();
    delay(20);
  }
  for (int i = 0; i < 100; i++) {
    displayInfo[pos] = CHSV(20, 220, 160 - i);
    displayInfo[pos + 1] = CHSV(20, 220, 120 - i * 0.6);
    displayInfo[pos - 1] = CHSV(20, 220, 120 - i * 0.6);
    displayInfo[pos - 2] = CHSV(20, 220, 100 - i * 0.4);
    displayInfo[pos + 2] = CHSV(20, 220, 100 - i * 0.4);
    FastLED.show();
    delay(20);
  }
}


void cloudy() {
  int pos = random8(LEDNumber);
  pos = pos < 1 ? 2 : pos;
  pos = pos >= 22 ? 21 : pos;
  fill_solid(displayInfo, LEDNumber, CHSV(230, 120, 60));
  FastLED.show();
  for (int i = 0; i < 100; i++) {
    displayInfo[pos] = CHSV(230, 120, 60 + i);
    displayInfo[pos + 1] = CHSV(230, 120, 60 + i * 0.6);
    displayInfo[pos - 1] = CHSV(230, 120, 60 + i * 0.6);
    displayInfo[pos - 2] = CHSV(230, 120, 60 + i * 0.4);
    displayInfo[pos + 2] = CHSV(230, 120, 60 + i * 0.4);
    FastLED.show();
    delay(20);
  }
  for (int i = 0; i < 100; i++) {
    displayInfo[pos] = CHSV(230, 120, 160 - i);
    displayInfo[pos + 1] = CHSV(230, 120, 120 - i * 0.6);
    displayInfo[pos - 1] = CHSV(230, 120, 120 - i * 0.6);
    displayInfo[pos - 2] = CHSV(230, 120, 100 - i * 0.4);
    displayInfo[pos + 2] = CHSV(230, 120, 100 - i * 0.4);
    FastLED.show();
    delay(20);
  }
}



void writeString(int addr, String data) {
  EEPROM.write(addr, data.length());
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(addr + 1 + i, (int)data[i]);
  }
  EEPROM.commit();
  return;
}


//LED strip initialize
void LEDStripInit() {
  displayInfo = new CRGB[LEDNumber];
  LEDS.addLeds<LED_TYPE, LEDPin, COLOR_ORDER>(displayInfo, LEDNumber);  // 初始化光带
  FastLED.setBrightness(MAX_BRIGHTNESS);   // 设置光带亮度
  FastLED.clear();
  fill_solid(displayInfo, LEDNumber, CRGB::Black);
  FastLED.show();
}



//LED Strip control
void LEDDisplay(String weatherState) {
  if (weatherState == "" && weatherInfo.lastWeather == "") {
    fill_solid(displayInfo, LEDNumber, CRGB::Black);
    FastLED.show();
  }
  if (weatherState == "晴") {
    sunny();
  }
  if (weatherState == "阴" || weatherState == "多云") {
    cloudy();
  }
  if (weatherState == "小雨" || weatherState == "阵雨")
    rain(0);
  if (weatherState == "中雨")
    rain(1);
  if (weatherState == "大雨")
    rain(2);
  if (weatherState == "暴雨" || weatherState == "大暴雨")
    rain(3);

  if (weatherState == "雷阵雨") {
    thunderShower();
  }

  if (weatherState == "小雪" || weatherState == "阵雪")
    snow(0);
  if (weatherState == "中雪")
    snow(1);
  if (weatherState == "大雪")
    snow(2);

  if (weatherState == "沙尘暴" || weatherState == "强沙尘暴" )
    warnningDust();
  if (weatherState == "冻雨" || weatherState == "雷阵雨伴有冰雹" || weatherState == "特大暴雨" || weatherState == "暴雪")
    warnningWater();


}



//weather infomation request function
void requestWeatherInfo() {
  String requestURL = API_head + configInfo.cityName + API_tail;

  //新建有一个HTTPClient的对象httpClient
  String response = "";
  //设定请求的url；
  httpClient.begin(client, requestURL);
  //设定get请求，并且返回请求码；
  int httpCode = httpClient.GET();
  Serial.println("Requesting");
  //设定浏览器ua
  httpClient.setUserAgent("Mozilla/5.0 (Windows NT 11.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/99.0.4844.82 Safari/537.36");
  //如果get请求返回HTTP_CODE_OK，则代表和服务器请求成功；
  if (httpCode == HTTP_CODE_OK) {
    Serial.println("requesting success!");
    //获取请求url的浏览器html代码；
    response = httpClient.getString();
    //向串口输出html代码
    //Serial.println(response);
  }
  //结束请求
  httpClient.end();

  JSONVar weatherJSON = JSON.parse(response);
  if (weatherJSON.hasOwnProperty("results")) {
    JSONVar results0 = weatherJSON["results"][0];
    JSONVar results0_now = results0["now"];
    weatherInfo.nowWeather = (const char*)results0_now["text"];
  }
  else {
    weatherInfo.nowWeather = "";
  }
  Serial.println(weatherInfo.nowWeather);
  if (weatherInfo.nowWeather != weatherInfo.lastWeather && weatherInfo.nowWeather != "") {
    weatherInfo.changing = true;
    weatherInfo.lastWeather = weatherInfo.nowWeather;
    Serial.println("change happened!");
  }
  else {
    weatherInfo.changing = false;
  }

  firstRequestDone = true;
}



//server call back
void setPage() {

  String webpage = "";
  webpage =  "<html>\
          <head>\
            <title>SetPage</title>\
            <style>\
              body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
            </style>\
          </head>\
          <body>\
            <h1>Plaese input the name and password of your WiFi</h1><br>\
            <form method=\"post\" action=\"/set_finished/\">\
              <p>name</p>\
              <input type=\"text\" name=\"ssid\"}\'><br>\
              <p>password</p>\
              <input type=\"text\" name=\"passwd\"}\'><br>\
              <p>Your city name</p>\
              <input type=\"text\" name=\"cityName\"}\'><br>\
              <input type=\"submit\" value=\"Submit\">\
          </body>\
        </html>";

  server.send(200, "text/html", webpage);
}

void resultDisplay() {
  //this page only allows POST
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
  } else {
    //get the arguments,0 is ssid and 1 is password
    configInfo.ssid = server.arg(0);
    configInfo.passwd = server.arg(1);
    configInfo.cityName = server.arg(2);

    String webpage = "";
    webpage = "<html>\
            <head>\
              <title>SetFinished</title>\
              <style>\
                body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
              </style>\
            </head>\
            <body>\
              <h4>the name of your WiFi: " + configInfo.ssid + " and the password is: " + configInfo.passwd + " and your city name is: " + configInfo.cityName + "</h4>" + '\n' +
              "<form method=\"POST\" action=\"/\">\
                <input type=\"submit\" value=\"Back\">\
            </body>\
          </html>";

    server.send(200, "text/html", webpage);

    Serial.println("Web Server Set successfully!");

    //store the wifi info into eeprom
    if (configInfo.ssid != "") {
      writeString(1, configInfo.ssid);
      writeString(256, configInfo.passwd);
      writeString(512, configInfo.cityName);
      //start wifi
      WiFi.begin(configInfo.ssid, configInfo.passwd);
      while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
        WiFi.begin(configInfo.ssid, configInfo.passwd);
      }
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        Serial.println("");
        Serial.print("Connected!  ");
        Serial.println(WiFi.localIP());
      }
    }
  }
}
