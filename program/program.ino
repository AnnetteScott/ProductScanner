#include <Adafruit_GFX.h>       // Core graphics
#include <MCUFRIEND_kbv.h>      // Hardware-specific
#include <WiFi.h>               // Wifi
#include <HTTPClient.h>         // Http request
#include "Adafruit_TinyUSB.h"
#include <string> 

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define MaxLength 18
#define LeftPadding 10

//Buttons and swtiches
const byte upButton = 4;
const byte downButton = 15;
const byte okButton = 5;
const byte cancelButton = 3;
const byte menuButton = 2;
const byte inOutSwitch = 20;

bool upButtonPrev;
bool downButtonPrev;
bool okButtonPrev;
bool cancelButtonPrev;
bool menuButtonPrev;
bool addingProduct;

bool needConfirmation;
bool gotConfirmation;
String toConfirmBarcode = "";

const char* ssid = "SSID";
const char* password = "PASSWORD";
String buffer = "";
MCUFRIEND_kbv tft;
Adafruit_USBH_Host USBHost;


void setup() {
  tft.begin(tft.readID());
  tft.setRotation(2);
  tft.fillScreen(WHITE);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  USBHost.begin(0);
  setUpButtons();
  displayInOut(20, 5);
}

void loop() {
  USBHost.task();
  Serial1.flush();
  checkButton();
}

void displayInOut(int x, int y){
  tft.fillScreen(WHITE);
  if(addingProduct){
    tft.setTextColor(GREEN);
    print(x, y, "Adding Product");
  }else {
    tft.setTextColor(RED);
    print(x, y, "Removing Product");
  }
  tft.setTextColor(BLACK);
}

void print(int x, int y, String msg){
  tft.setCursor(x, y);
  String words[MaxLength] = {};
  int i = 0, j = 0;
  while (j < msg.length()) {
    if (msg.charAt(j) == ' ') {
      i++;
    }
    else {
      words[i] += msg.charAt(j);
    }
    j++;
  }

  int currentLength = 0;
  for(int i = 0; i < MaxLength; i++){
    currentLength += words[i].length() + 1;
    if(currentLength > MaxLength){
      y += 20;
      tft.setCursor(x, y);
      currentLength = 0;
    }
    tft.print(words[i] + " ");
  }
}

void setUpButtons(){
  pinMode (upButton, INPUT_PULLUP);
  pinMode (downButton, INPUT_PULLUP);
  pinMode (okButton, INPUT_PULLUP);
  pinMode (cancelButton, INPUT_PULLUP);
  pinMode (menuButton, INPUT_PULLUP);
  pinMode (inOutSwitch, INPUT_PULLUP);

  upButtonPrev = digitalRead(upButton) == LOW;
  downButtonPrev = digitalRead(downButton) == LOW;
  okButtonPrev = digitalRead(okButton) == LOW;
  cancelButtonPrev = digitalRead(cancelButton) == LOW;
  menuButtonPrev = digitalRead(menuButton) == LOW;
  addingProduct = digitalRead(inOutSwitch) == LOW;
}

void checkButton() {
  unsigned long currentTime = millis();
  bool up = digitalRead(upButton) == LOW;
  bool down = digitalRead(downButton) == LOW;
  bool ok = digitalRead(okButton) == LOW;
  bool cancel = digitalRead(cancelButton) == LOW;
  bool menu = digitalRead(menuButton) == LOW;
  bool inOut = digitalRead(inOutSwitch) == LOW;

  if (up && !upButtonPrev)
  {
    //Up Button Pressed
  }

  if (down && !downButtonPrev)
  {
    //Down Button Pressed
  }

  if (ok && !okButtonPrev)
  {
    if(needConfirmation){
      gotConfirmation = true;
      getHTTP(toConfirmBarcode);
    }
  }

  if (cancel && !cancelButtonPrev)
  {
    needConfirmation = false;
    gotConfirmation = false;
    toConfirmBarcode = "";
    displayInOut(25, 5);
  }

  if (menu && !menuButtonPrev)
  {
    //Menu Button Pressed
  }

  if (inOut != addingProduct)
  {
    addingProduct = inOut;
    displayInOut(20, 5);
    
  }

  upButtonPrev = up;
  downButtonPrev = down;
  okButtonPrev = ok;
  cancelButtonPrev = cancel;
  menuButtonPrev = menu;

  delay(100);
}

void getHTTP(String barcode){
  HTTPClient http;
  String api = addingProduct ? "getProduct" : "removeProduct";
  String URL = "http://192.168.0.101:48673/" + api + "?barcode=" + barcode;

  if(needConfirmation && gotConfirmation){
    URL += "&confirm=CONFIRMED";
  }

  http.begin(URL);

  int httpCode = http.GET();
  displayInOut(25, 5);

  if(httpCode == 400){
    print(LeftPadding, 40, "Invalid Barcode");
  }
  else if(httpCode == 404){
    print(LeftPadding, 40, "Product Not Found");
  }
  else if(httpCode == 418){ //Need confirmation as new product
    String payload = http.getString();
    toConfirmBarcode = barcode;
    askForConfirmation(payload);
  }
  else if(httpCode == 200){
    String payload = http.getString();
    print(LeftPadding, 40, payload);
    needConfirmation = false;
    gotConfirmation = false;
    toConfirmBarcode = "";
  }
}

void parseReport(uint8_t const *report, uint16_t len){
  String out = String(report[2]);
  if (out == "0") {
    return;
  }

  String character = getChar(out);
  if(character == "end"){
    getHTTP(buffer);
    buffer = "";
    return;
  }
  buffer += character;
}

//Converts barcode scanner input into char
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

// Invoked when device is mounted
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  tuh_hid_receive_report(dev_addr, instance);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
  parseReport(report, len);
  tuh_hid_receive_report(dev_addr, instance);
}

void askForConfirmation(String name){
  needConfirmation = true;
  gotConfirmation = false;
  tft.fillScreen(WHITE);
  tft.setTextColor(RED);
  print(LeftPadding, 5, "Need Confirmation");
  tft.setTextColor(BLACK);
  print(LeftPadding, 40, name);
}















