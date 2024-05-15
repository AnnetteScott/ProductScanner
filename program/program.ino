#include <Adafruit_GFX.h>       // Core graphics
#include <MCUFRIEND_kbv.h>      // Hardware-specific
#include <WiFi.h>               // Wifi
#include <HTTPClient.h>         // Http request
#include "Adafruit_TinyUSB.h"
#include <string> 

MCUFRIEND_kbv tft;

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF
#define GREY    0x8410

const char* ssid = "SSID";
const char* password = "PASSWORD";
String buffer = "";

Adafruit_USBH_Host USBHost;


void setup() {
  Serial.begin(9600);
  tft.begin(tft.readID());
  tft.setRotation(2);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  USBHost.begin(0);

  testScreen();
}

void loop() {
  USBHost.task();
  Serial1.flush();
}

String getHTTP(String barcode){
  HTTPClient http;
  String URL = "http://SERVER_IP/getProduct?barcode=" + barcode; // Works with HTTP
  http.begin(URL); // Works with HTTP

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    tft.println(payload); // Print response
    return payload;
  }
  return "Didn't work";
}

void testScreen(){
  tft.fillScreen(WHITE);
  tft.setCursor(5, 5);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
}

void parseReport(uint8_t const *report, uint16_t len){
  String out = String(report[2]);
  if (out == "0") {
    return;
  }

  String character = getChar(out);
  if(character == "end"){
    getHTTP(buffer);
    tft.println(buffer);
    buffer = "";
    return;
  }

  buffer += character;
}

String getChar(String input){
  if(input == "38"){
    return "9";
  }else if(input == "37"){
    return "8";
  }else if(input == "36"){
    return "7";
  }else if(input == "35"){
    return "6";
  }else if(input == "34"){
    return "5";
  }else if(input == "33"){
    return "4";
  }else if(input == "32"){
    return "3";
  }else if(input == "31"){
    return "2";
  }else if(input == "30"){
    return "1";
  }else if(input == "39"){
    return "0";
  }else if(input == "40"){
    return "end";
  }

  return "";
}

// Invoked when device is mounted (configured)
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  tuh_hid_receive_report(dev_addr, instance);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
  parseReport(report, len);
  tuh_hid_receive_report(dev_addr, instance);
}


















