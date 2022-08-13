#include "FastLED.h"            // 此示例程序需要使用FastLED库
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <Ticker.h>

#define NUM_LEDS 24             // LED灯珠数量
#define DATA_PIN D5             // Arduino输出控制信号引脚
#define LED_TYPE WS2812B         // LED灯带型号
#define COLOR_ORDER GRB         // RGB灯珠中红色、绿色、蓝色LED的排列顺序
 

#define twinkleChance 100        //  闪烁数量，数值越大闪烁越多（0-255） 
 
uint8_t max_bright = 255;       // LED亮度控制变量，可使用数值为 0 ～ 255， 数值越大则光带亮度越高
 
CRGB displayInfo[NUM_LEDS];            // 建立光带leds
 
void setup() { 
  Serial.begin(12500);           // 启动串行通讯
  delay(1000);                  // 稳定性等待
 
  LEDS.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(displayInfo, NUM_LEDS);  // 初始化光带
  
  FastLED.setBrightness(max_bright); // 设置光带亮度


  WiFi.begin("HONOR-10F10N","fl284502");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    WiFi.begin("HONOR-10F10N","fl284502");
  }
  if (WiFi.waitForConnectResult() == WL_CONNECTED){
    Serial.println("");
    Serial.print("Connected!  ");
    Serial.println(WiFi.localIP());      
  }
  
  if (WiFi.waitForConnectResult() == WL_CONNECTED){
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

}
 
void loop() {      
  ArduinoOTA.handle();
  fill_solid(displayInfo, 24, CRGB(0,0,0)); 
  thunderShower();
  FastLED.show(); 
}

void thunderShower(){
  Ticker fade;
  int twinkleInterval = random(100,300);
  int fadeSpeed = 20;
  fade.attach_ms(20,[fadeSpeed]{fadeToBlackBy( displayInfo, NUM_LEDS, fadeSpeed);FastLED.show();});
  int pos = random8(NUM_LEDS);
  int pos2 = random8(NUM_LEDS);
  int pos3 = random8(NUM_LEDS);
  int pos4 = random8(NUM_LEDS);
  int pos5 = random8(NUM_LEDS);
  pos = pos < 1 ? 2 : pos;
  pos = pos >= 23 ? 22 : pos;
  
  if( random8() < 230) {
    displayInfo[pos] = CHSV(150,200,100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos2] = CHSV(150,200,100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos3] = CHSV(150,200,100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos4] = CHSV(150,200,100);
    FastLED.show();
    delay(random8() / 4);
    displayInfo[pos5] = CHSV(150,200,100);
    FastLED.show();
    fadeSpeed = 50;
    fade.detach();
    fade.attach_ms(20,[fadeSpeed]{fadeToBlackBy( displayInfo, NUM_LEDS, fadeSpeed);FastLED.show();});
  }   
  else{
    delay(80);
    displayInfo[pos] = CHSV(200, 100, 255);
    displayInfo[pos-1] = CHSV(220, 50, 150);
    displayInfo[pos-2] = CHSV(200, 20, 50);
    displayInfo[pos+1] = CHSV(200, 50, 150);
    displayInfo[pos+2] = CHSV(200, 20, 50);
    FastLED.show();
    fadeSpeed = 12;
    fade.detach();
    fade.attach_ms(20,[fadeSpeed]{fadeToBlackBy( displayInfo, NUM_LEDS, fadeSpeed);FastLED.show();});
    delay(80);
  }
  delay(twinkleInterval);
}
