#include <Arduino.h>

#define RELAY_PIN 2
#define LED_PIN 4
#define BUZZER_PIN 26

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  
  Serial.println("Relay/Output Test");
  Serial.println("Commands:");
  Serial.println("'relay on' - Activate relay");
  Serial.println("'relay off' - Deactivate relay");
  Serial.println("'led on' - Turn on LED");
  Serial.println("'led off' - Turn off LED");
  Serial.println("'buzzer' - Test buzzer");
  Serial.println("'test' - Test all outputs");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();
    
    if (command == "relay on") {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay ON");
    }
    else if (command == "relay off") {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay OFF");
    }
    else if (command == "led on") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED ON");
    }
    else if (command == "led off") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED OFF");
    }
    else if (command == "buzzer") {
      for (int i = 0; i < 3; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
      Serial.println("Buzzer test complete");
    }
    else if (command == "test") {
      Serial.println("Testing all outputs...");
      
      digitalWrite(LED_PIN, HIGH);
      delay(500);
      digitalWrite(RELAY_PIN, HIGH);
      delay(500);
      digitalWrite(BUZZER_PIN, HIGH);
      delay(300);
      digitalWrite(BUZZER_PIN, LOW);
      delay(1000);
      
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      Serial.println("Test complete");
    }
  }
  delay(100);
}