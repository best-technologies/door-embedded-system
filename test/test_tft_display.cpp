#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TOUCH_CS 21
#define TOUCH_IRQ 22

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  ts.begin();
  ts.setRotation(1);
  
  // Test display
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("TFT Display Test");
  tft.println("Touch to draw");
  
  Serial.println("TFT Display test ready");
}

void loop() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    int x = map(p.x, 200, 3700, 0, 480);
    int y = map(p.y, 240, 3800, 0, 320);
    
    x = constrain(x, 0, 479);
    y = constrain(y, 0, 319);
    
    tft.fillCircle(x, y, 5, TFT_YELLOW);
    Serial.printf("Touch: X=%d, Y=%d\n", x, y);
  }
  delay(50);
}