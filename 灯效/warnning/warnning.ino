#include "FastLED.h"            // 此示例程序需要使用FastLED库
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <Ticker.h>


#define NUM_LEDS 24             // LED灯珠数量
#define DATA_PIN D5             // Arduino输出控制信号引脚
#define LED_TYPE WS2812B         // LED灯带型号
#define COLOR_ORDER GRB         // RGB灯珠中红色、绿色、蓝色LED的排列顺序

#define twinkleChance 255        //  闪烁数量，数值越大闪烁越多（0-255） 
 
uint8_t max_bright = 200;       // LED亮度控制变量，可使用数值为 0 ～ 255， 数值越大则光带亮度越高
 
CRGB displayInfo[NUM_LEDS];            // 建立光带leds
 
void setup() { 
  Serial.begin(115200);           // 启动串行通讯
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
  randomSeed(analogRead(A0));  //使用一个未被使用到的引脚

}
 
void loop() {      
  ArduinoOTA.handle();
  warnning();
//  fill_solid(leds, 24, CRGB(0,0,0)); 
//  FastLED.show(); 
}

void warnning(){
  fill_solid(displayInfo, NUM_LEDS, CHSV(20,250,0));
  FastLED.show(); 
  for(int i = 1; i < 150; i++){
    fill_solid(displayInfo, NUM_LEDS, CHSV(0,250,i));
    delay(i / 10);      
    FastLED.show();
  }
  for(int i = 1; i < 150; i++){
    fill_solid(displayInfo, NUM_LEDS, CHSV(0,250,150 - i));
    delay((150 - i)/10);  
    FastLED.show();    
  }
  fill_solid(displayInfo, NUM_LEDS, CHSV(20,250,0));
  FastLED.show();  
}
