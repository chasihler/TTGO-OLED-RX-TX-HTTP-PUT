#include <HTTPClient.h>
#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>

#include <ESP32DMASPIMaster.h>
#include <ESP32DMASPISlave.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h" 
#include "images.h"

const char* ssid = "ChasAndMelissa";
const char* password =  "Password";

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     23   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    915E6hj';


SSD1306 display(0x3c, 21, 22);
String rssi = "RSSI --";
String packSize = "--";
String packet ;

int loopcount;
    String put_string;


void loraData(){
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0 , 15 , "Received "+ packSize + " bytes");
  display.drawStringMaxWidth(0 , 26 , 128, packet);
  display.drawString(0, 0, rssi); 
  display.display();
  Serial.println(rssi);
}

void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  loraData();
}

void setup() {
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high、
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Receiver Callback");
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);  
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
  LoRa.receive();
  Serial.println("init ok");
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);
   
  delay(1500);
  WiFi.begin(ssid, password); 
  
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(1000);
    Serial.print("Connecting to WiFi..");
    Serial.print(WiFi.status());
    Serial.print(" ");
    Serial.println(WiFi.localIP().toString());
  }
  Serial.println("Connected to the WiFi network");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { cbk(packetSize);  }
  delay(10);
 
  loopcount = loopcount + 1;

  if (loopcount > 1000) {
    
     if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    
     HTTPClient http;   
    
     http.begin("https://cloudypass.org/posttome.php");  //Specify destination for HTTP request
     http.addHeader("Content-Type", "text/plain");             //Specify content-type header

    put_string  = "{";
    put_string += "\"on\":true";
    put_string += "}";
        
   int httpResponseCode = http.PUT(put_string);   
 
   if(httpResponseCode>0){
 
    String response = http.getString();   
 
    Serial.println(httpResponseCode);
    Serial.println(response);          
 
   }else{
 
    Serial.print("Error on sending PUT Request: ");
    Serial.println(httpResponseCode);
 
   }
 
   http.end();
  
    }else{
  
      Serial.println("Error in WiFi connection");    
    }
    loopcount = 0;
  }

  
}
