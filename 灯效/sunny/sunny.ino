#include "FastLED.h"            // 此示例程序需要使用FastLED库
#include <ArduinoOTA.h>
#include <Arduino.h>
#include <Ticker.h>


#define NUM_LEDS 24             // LED灯珠数量
#define DATA_PIN D5             // Arduino输出控制信号引脚
#define LED_TYPE WS2812B         // LED灯带型号
#define COLOR_ORDER GRB         // RGB灯珠中红色、绿色、蓝色LED的排列顺序

#define twinkleChance 255        //  闪烁数量，数值越大闪烁越多（0-255） 
 
uint8_t max_bright = 255;       // LED亮度控制变量，可使用数值为 0 ～ 255， 数值越大则光带亮度越高
 
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
 

int pos[14] = {'\0'};
 
void loop() {      
  ArduinoOTA.handle();
  sunny();
}

void sunny(){
  int pos = random8(NUM_LEDS);
  fill_solid(displayInfo, NUM_LEDS, CHSV(20, 220, 60));
  FastLED.show();
  for (int i = 0; i < 150; i++){
    displayInfo[pos] = CHSV(20,220,60+i);
    displayInfo[pos+1] = CHSV(20,220,60+i*0.6);
    displayInfo[pos-1] = CHSV(20,220,60+i*0.6);
    displayInfo[pos-2] = CHSV(20,220,60+i*0.4);
    displayInfo[pos+2] = CHSV(20,220,60+i*0.4);
    FastLED.show();
    delay(5); 
  }
  for (int i = 0; i < 150; i++){
    displayInfo[pos] = CHSV(20,220,210-i);
    displayInfo[pos+1] = CHSV(20,220,150-i*0.6);
    displayInfo[pos-1] = CHSV(20,220,150-i*0.6);
    displayInfo[pos-2] = CHSV(20,220,120-i*0.4);
    displayInfo[pos+2] = CHSV(20,220,120-i*0.4);
    FastLED.show();
    delay(5); 
  }
}
