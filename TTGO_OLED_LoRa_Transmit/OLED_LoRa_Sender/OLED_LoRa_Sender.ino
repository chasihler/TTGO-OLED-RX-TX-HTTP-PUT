// Charles Ihler
// iradan.com
// Plenty of items sourced from the google machine. 
// I just found a combination of crap that worked. 

#include <ESP32DMASPIMaster.h>
#include <ESP32DMASPISlave.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h"
#include "images.h"


#define SCK     5    // GPIO5  -- SCK
#define MISO    19   // GPIO19 -- MISO
#define MOSI    27   // GPIO27 -- MOSI
#define SS      18   // GPIO18 -- CS
#define RST     23   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    915E6 //US version

#define uS_TO_S_FACTOR 1000000  //  ms to S
#define TIME_TO_SLEEP  15        //  in seconds

RTC_DATA_ATTR int bootCount = 0;  //fun! a first for me.. 


unsigned int counter = 0;

SSD1306 display(0x3c, 21, 22);
String rssi = "RSSI --";
String packSize = "--";
String packet ;
const int ADCpin = 34;
const int VBATpin = 35;

int ADC_VALUE = 0;
float ADCv_value = 0; 
float VBAT = 0.0000; 

void setup() {
  pinMode(16,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(25,OUTPUT);
  pinMode(ADCpin, INPUT);
  pinMode(VBATpin, INPUT);
  
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50); 
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println();
  Serial.println("LoRa Sender Test");
  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  //LoRa.onReceive(cbk);
//  LoRa.receive();
  display.init();
  display.flipScreenVertically();  
  display.setFont(ArialMT_Plain_10);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
  
   ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  Serial.println("initialization complete");

 
  delay(1500);

  
}

void loop() {
  //ADC test, not ready for LoRa yet
  ADC_VALUE = analogRead(ADCpin);
  Serial.print("ADC VALUE: ");
  Serial.println(ADC_VALUE);
  ADCv_value = (ADC_VALUE * 3.3 ) / (4095);
  Serial.print("ADC IO34 Voltage: ");
  Serial.print(ADCv_value);
  Serial.println("V");
  
  VBAT = (float)(analogRead(VBATpin)) / 4095*2*3.3*1.1;
  Serial.println("VBAT: "); 
  Serial.print(VBAT); 
  Serial.println("V");
  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  
  display.drawString(0, 0, "Sending packet: ");
  display.drawString(90, 0, String(counter));
  display.drawString(0, 12, String(ADC_VALUE));
  display.drawString(0, 24, "VBAT: ");
  display.drawString(35, 24, String(VBAT));
  Serial.println(String(counter));
  
  display.display();

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;
  digitalWrite(25, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(25, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  display.clear();
  display.display();
  //sleepy time
  esp_deep_sleep_start();  
}
