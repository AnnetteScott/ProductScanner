#include <KeyboardBT.h>
#include <string>

String receivedMessage;

void setup() {
  KeyboardBT.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);

  // Initialize Serial2 for receiving data
  Serial2.setRX(9);
  Serial2.setTX(8);
  Serial2.begin(9600);

  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (Serial2.available() > 0) {
    char receivedChar = Serial2.read();
    KeyboardBT.write(receivedChar);
    Serial.print(receivedChar);
    if (receivedChar == '\n') {
      Serial.println(receivedMessage);  // Print the received message in the Serial monitor
      receivedMessage = "";  // Reset the received message
    } else {
      receivedMessage += receivedChar;  // Append characters to the received message
    }
  }
}
