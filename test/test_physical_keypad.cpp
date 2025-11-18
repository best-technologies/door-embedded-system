#include <Keypad.h>

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},  // A = Enter
  {'4', '5', '6', 'B'},  // B = Enter  
  {'7', '8', '9', 'C'},  // C = Clear
  {'*', '0', '#', 'D'}   // * = Back, # = Clear, D = unused
};

byte rowPins[ROWS] = {13, 12, 14, 27}; // R1, R2, R3, R4
byte colPins[COLS] = {26, 25, 33, 32};// C4, C3, C2, C1

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputText = "";

void setup() {
  Serial.begin(115200);
  Serial.println("Physical Keypad Test");
  Serial.println("Key mapping:");
  Serial.println("1-9, 0: Numbers");
  Serial.println("A/B: Enter");
  Serial.println("C/#: Clear");
  Serial.println("*: Back");
}

void loop() {
  char key = keypad.getKey();
  
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    
    if (key >= '0' && key <= '9') {
      inputText += key;
      Serial.print("Input: ");
      Serial.println(inputText);
    }
    else if (key == 'A' || key == 'B' || key == 'C') {
      Serial.print("ENTER - Final input: ");
      Serial.println(inputText);
      inputText = "";
    }
    else if (key == 'D' || key == '#') {
      inputText = "";
      Serial.println("CLEAR - Input cleared");
    }
    else if (key == '*') {
      if (inputText.length() > 0) {
        inputText.remove(inputText.length() - 1);
        Serial.print("BACK - Input: ");
        Serial.println(inputText);
      }
    }
  }
}