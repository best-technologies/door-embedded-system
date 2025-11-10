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
bool lastTouchState = false;

// Button structure
struct Button {
  int x, y, w, h;
  String label;
  uint16_t color;
  bool isSpecial;
};

// Keypad layout (4x3 grid)
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
  if (btn.isSpecial) {
    tft.setTextSize(1);
  } else {
    tft.setTextSize(3);
  }
  
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

void drawTextArea() {
  tft.fillRect(40, 70, 380, 40, TFT_WHITE);
  tft.drawRect(40, 70, 380, 40, TFT_BLACK);
  
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(50, 85);
  tft.println(inputText);
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
  drawTextArea();
  
  Serial.println("Touch Keypad Test Ready");
}

void loop() {
  bool currentTouchState = ts.touched();
  
  if (currentTouchState && !lastTouchState) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, 200, 3700, 0, 480);
    int y = map(p.y, 240, 3800, 0, 320);
    
    x = constrain(x, 0, 479);
    y = constrain(y, 0, 319);
    
    handleTouch(x, y);
  }
  
  lastTouchState = currentTouchState;
  delay(50);
}