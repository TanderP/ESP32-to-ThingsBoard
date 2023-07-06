#include <Arduino.h>
#include "Wire.h"
#include <WiFi.h>
#include <Wire.h>
#include <BH1750.h>
#include <Ticker.h>
#include <DHTesp.h>
#include "ThingsBoard.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels
#define OLED_RESET -1

#define WIFI_SSID "TEKDUS"
#define WIFI_PASSWORD "SSIDPASSWORD"

#define THINGSBOARD_SERVER "demo.thingsboard.io"
#define THINGSBOARD_ACCESS_TOKEN "ai82jzy1b8klcli8tbqs"

#define DHT_PIN 18
#define DHT_TYPE DHTesp::DHT11

BH1750 lightMeter ;
DHTesp dht;
WiFiClient espClient;
ThingsBoard tb(espClient);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ESP32-DEvkit V4
//Upesy wroom

float temp, hum, lux;
String WifiStatus;
String TBStatus;
//xyz = 241
//x = temp
//y = hum
//z = lux
void SensorRead(void *pvParameters){ //Y
  while(1){
  hum = dht.getHumidity(); // read humidity
  temp = dht.getTemperature();
  lux = lightMeter.readLightLevel();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
void sensorDisplay(void *pvParameters){
  while (1) {

   Serial.printf("Serial Monitor : Lux = %.2f lx || Temperature  : %.2f | Humidity : %.2f\n", lux, temp, hum);
     if (tb.connected())
    {
      tb.sendTelemetryFloat("humidity", hum);
      tb.sendTelemetryFloat("temperature", temp);
      tb.sendTelemetryFloat("light", lux);
    }
    else
    {
      Serial.println("ThingsBoard connection failed!");
    }
  vTaskDelay(1000 / portTICK_PERIOD_MS);}
   //
}
void WifiConnect(void *pvParameters){ // connect to wifi
   
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
   
    Serial.println("Connection Failed! Rebooting...");
    WifiStatus = "Reconnecting";
    delay(5000);
    ESP.restart(); // if not connect, restart
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
  WifiStatus = "Connected";
  vTaskDelete(NULL); // deleting the task
}
void TBConnect(void *pvParameters){
 while(1){
   if (!tb.connected())
  {
    if (tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN)){
      Serial.println("Connected to thingsboard");
      TBStatus = "Connected";

      }
    else
    {
      Serial.println("Error connected to thingsboard");
      TBStatus = "Not Connected";
      delay(3000);
    }
  }
  delay(1000);
  tb.loop();
 }
}
  void setup() {
Serial.begin(115200);
 
  // Initialize the I2C bus (BH1750 library doesn't do this automatically)
  Wire.begin();
  lightMeter.begin();
  dht.setup(DHT_PIN, DHT_TYPE);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

xTaskCreatePinnedToCore(SensorRead, "sensorRead", 2044, NULL, 1, NULL, 0);
xTaskCreatePinnedToCore(sensorDisplay, "sensorDisplay", 2044, NULL, 1, NULL, 0);
xTaskCreatePinnedToCore(WifiConnect, "WifiConnect", 4048, NULL, 1, NULL, 0);
xTaskCreatePinnedToCore(TBConnect, "TBConnect", 4048, NULL, 1, NULL, 1);
  // put your setup code here, to run once:
}
void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); 
  display.println("Hello World");
  display.printf("Data: %.2f \n", lux);
  display.printf("WIFI : %s \n", WifiStatus);
  display.printf("TB : %s \n", TBStatus);
  display.display();
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  // put your main code here, to run repeatedly:
}
