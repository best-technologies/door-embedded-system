#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Pin definitions for ESP32 (adjust if needed)
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TOUCH_CS 21
#define TOUCH_IRQ 22

TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// Function declarations
void displayTest();
void touchTest();

void setup() {
  Serial.begin(115200);

  // Initialize display
  tft.init();
  tft.setRotation(1);  // Landscape mode for 480x320
  tft.fillScreen(TFT_BLACK);

  // Initialize touch
  ts.begin();
  ts.setRotation(1);

  // Display test
  displayTest();

  Serial.println("Display and touch test ready!");
}

void loop() {
  touchTest();
  delay(50);
}

void displayTest() {
  // Test basic colors
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  tft.fillScreen(TFT_BLACK);

  // Test text
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("4.0\" TFT Display Test");
  tft.println("Resolution: 480x320");

  // Test shapes
  tft.drawRect(10, 60, 100, 50, TFT_YELLOW);
  tft.fillCircle(200, 85, 25, TFT_CYAN);
  tft.drawLine(10, 130, 470, 130, TFT_MAGENTA);

  // Touch instruction
  tft.setCursor(10, 150);
  tft.setTextColor(TFT_GREEN);
  tft.println("Touch screen to test touch!");

  // Draw touch areas
  tft.drawRect(50, 200, 100, 60, TFT_WHITE);
  tft.setCursor(60, 220);
  tft.setTextColor(TFT_WHITE);
  tft.println("TOUCH");
  tft.println("  HERE");
}

void touchTest() {
  if (ts.touched()) {
    TS_Point p = ts.getPoint();

    // Map touch coordinates to screen coordinates
    int x = map(p.x, 200, 3700, 0, 480);
    int y = map(p.y, 240, 3800, 0, 320);

    // Constrain to screen bounds
    x = constrain(x, 0, 479);
    y = constrain(y, 0, 319);

    // Draw touch point
    tft.fillCircle(x, y, 3, TFT_RED);

    // Display coordinates
    tft.fillRect(300, 200, 170, 80, TFT_BLACK);
    tft.setCursor(310, 210);
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(1);
    tft.println("Touch detected:");
    tft.print("X: ");
    tft.println(x);
    tft.print("Y: ");
    tft.println(y);
    tft.print("Raw X: ");
    tft.println(p.x);
    tft.print("Raw Y: ");
    tft.println(p.y);

    Serial.printf("Touch: X=%d, Y=%d (Raw: %d,%d)\n", x, y, p.x, p.y);
  }
}