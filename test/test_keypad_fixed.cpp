#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TOUCH_CS 21
#define TOUCH_IRQ 22

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

String inputText = "";
unsigned long lastTouchTime = 0;

struct Button {
  int x, y, w, h;
  String label;
  uint16_t color;
  bool isSpecial;
};

Button buttons[12] = {
  {40, 120, 80, 60, "1", TFT_DARKGREY, false},
  {140, 120, 80, 60, "2", TFT_DARKGREY, false},
  {240, 120, 80, 60, "3", TFT_DARKGREY, false},
  {340, 120, 80, 60, "CLEAR", TFT_RED, true},
  
  {40, 190, 80, 60, "4", TFT_DARKGREY, false},
  {140, 190, 80, 60, "5", TFT_DARKGREY, false},
  {240, 190, 80, 60, "6", TFT_DARKGREY, false},
  {340, 190, 80, 60, "ENTER", TFT_GREEN, true},
  
  {40, 260, 80, 60, "7", TFT_DARKGREY, false},
  {140, 260, 80, 60, "8", TFT_DARKGREY, false},
  {240, 260, 80, 60, "9", TFT_DARKGREY, false},
  {340, 260, 80, 60, "0", TFT_DARKGREY, false}
};

void drawButton(Button btn) {
  tft.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 8, btn.color);
  tft.drawRoundRect(btn.x, btn.y, btn.w, btn.h, 8, TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(btn.isSpecial ? 1 : 3);
  
  int textX = btn.x + (btn.w - btn.label.length() * (btn.isSpecial ? 6 : 18)) / 2;
  int textY = btn.y + (btn.h - (btn.isSpecial ? 8 : 24)) / 2;
  
  tft.setCursor(textX, textY);
  tft.println(btn.label);
}

void flashButton(Button btn) {
  tft.fillRoundRect(btn.x, btn.y, btn.w, btn.h, 8, TFT_WHITE);
  delay(100);
  drawButton(btn);
}

void drawKeypad() {
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 20);
  tft.println("TOUCH KEYPAD");
  
  for (int i = 0; i < 12; i++) {
    drawButton(buttons[i]);
  }
}

void updateTextDisplay() {
  tft.fillRect(41, 71, 378, 38, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(50, 85);
  tft.println(inputText);
}

void handleTouch(int x, int y) {
  for (int i = 0; i < 12; i++) {
    if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].w &&
        y >= buttons[i].y && y <= buttons[i].y + buttons[i].h) {
      
      flashButton(buttons[i]);
      
      if (buttons[i].label == "CLEAR") {
        inputText = "";
        Serial.println("CLEAR pressed");
      } else if (buttons[i].label == "ENTER") {
        Serial.print("ENTER pressed - Input: ");
        Serial.println(inputText);
        inputText = "";
      } else {
        if (inputText.length() < 20) {
          inputText += buttons[i].label;
          Serial.print("Button pressed: ");
          Serial.print(buttons[i].label);
          Serial.print(" - Current input: ");
          Serial.println(inputText);
        }
      }
      
      updateTextDisplay();
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  ts.begin();
  ts.setRotation(1);
  
  drawKeypad();
  
  tft.fillRect(40, 70, 380, 40, TFT_WHITE);
  tft.drawRect(40, 70, 380, 40, TFT_BLACK);
  updateTextDisplay();
  
  Serial.println("Touch Keypad Test Ready");
  Serial.println("NOTE: Touch controller showing invalid readings (8191,8191)");
  Serial.println("Check T_CS and T_IRQ connections");
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    
    // Filter out invalid readings
    if (p.x > 4000 || p.y > 4000 || p.x < 100 || p.y < 100) {
      return; // Skip invalid touch data
    }
    
    // Debounce
    if (millis() - lastTouchTime < 200) {
      return;
    }
    lastTouchTime = millis();
    
    // Map coordinates with different ranges
    int x = map(p.x, 300, 3800, 0, 480);
    int y = map(p.y, 300, 3800, 0, 320);
    
    x = constrain(x, 0, 479);
    y = constrain(y, 0, 319);
    
    Serial.printf("Valid touch - Raw: %d,%d Mapped: %d,%d\n", p.x, p.y, x, y);
    
    handleTouch(x, y);
  }
  
  delay(50);
}