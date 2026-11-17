#include <Arduino.h>
#include <IRremote.hpp>

#define IR_RECEIVE_PIN 5 
#define IR_SEND_PIN 1
#define BUTTON_PIN 0

#define LED_PIN 8 // C3 SuperMini built-in blue LED

decode_type_t storedProtocol = UNKNOWN;
uint16_t storedAddress = 0;
uint16_t storedCommand = 0;

void setup() {
  Serial.begin(115200);
  
  // 1. Set up the LED manually and force it OFF (HIGH is OFF on the SuperMini)
  pinMode(LED_PIN, OUTPUT);
  
  // FIX 1: Change to INPUT_PULLDOWN so the pin doesn't pick up random static
  pinMode(BUTTON_PIN, INPUT); 

  digitalWrite(LED_PIN, HIGH);
  
  // 2. Turn OFF the library's automatic (but inverted) LED feedback
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN);
  
  Serial.println("IR Receiver Ready. Point a remote and press a button...");
}

void loop() {
  // ==========================================
  // BLOCK A: Constantly check for incoming IR
  // ==========================================
  if (IrReceiver.decode()) {
    
    if (IrReceiver.decodedIRData.protocol != UNKNOWN 
        && !(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)
        && IrReceiver.decodedIRData.address != 0 
        && IrReceiver.decodedIRData.command != 0) {
        
        // --- Custom Blink ---
        digitalWrite(LED_PIN, LOW);  // Turn LED ON
        delay(100);                  // Wait 100 milliseconds
        digitalWrite(LED_PIN, HIGH); // Turn LED OFF
        
        // Save the valid data
        storedProtocol = IrReceiver.decodedIRData.protocol;
        storedAddress = IrReceiver.decodedIRData.address;
        storedCommand = IrReceiver.decodedIRData.command;

        // --- Print the Protocol as a String ---
        Serial.print("Protocol: ");
        Serial.print(getProtocolString(storedProtocol)); 
        Serial.print(" | Address: 0x");
        Serial.print(storedAddress, HEX);
        Serial.print(" | Command: 0x");
        Serial.println(storedCommand, HEX);
    }
    
    IrReceiver.resume();
  } // <-- FIX 2: Close the receiver block HERE!

  // ==========================================
  // BLOCK B: Constantly check the button
  // ==========================================
  if (digitalRead(BUTTON_PIN) == HIGH) {
    if (storedCommand != 0) {
      Serial.print("Firing Command: 0x");
      Serial.println(storedCommand, HEX);

      Serial.print(" ");
      Serial.print(storedAddress);
      
      if (storedProtocol == NEC) {
        IrSender.sendNEC(storedAddress, storedCommand, 0);
      } 
      else if (storedProtocol == SAMSUNG) {
        IrSender.sendSamsung(storedAddress, storedCommand, 0);
      } 
      else if (storedProtocol == SONY) {
        IrSender.sendSony(storedAddress, storedCommand, 2); // Sony usually needs 2 repeats
      }
      else {
        Serial.println("Protocol saved, but not programmed into the sender yet!");
      }
      
      // Brief delay so a single button tap doesn't fire 20 times instantly
      delay(200); 
    }
  }
}