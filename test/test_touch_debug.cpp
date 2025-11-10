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
  delay(1000);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Test touch initialization
  Serial.println("Touch Debug Test");
  
  if (ts.begin()) {
    Serial.println("Touch controller initialized successfully");
  } else {
    Serial.println("ERROR: Touch controller failed to initialize");
    Serial.println("Check wiring:");
    Serial.println("T_CS -> GPIO 21");
    Serial.println("T_IRQ -> GPIO 22");
    Serial.println("T_DIN -> GPIO 23 (shared with display)");
    Serial.println("T_DO -> GPIO 19 (shared with display)");
    Serial.println("T_CLK -> GPIO 18 (shared with display)");
  }
  
  ts.setRotation(1);
  
  // Display instructions
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("TOUCH DEBUG");
  tft.setCursor(10, 40);
  tft.println("Touch anywhere");
  tft.setCursor(10, 70);
  tft.println("Check serial monitor");
  
  Serial.println("Touch anywhere on screen...");
}

void loop() {
  // Check if touch is detected
  bool touched = ts.touched();
  
  if (touched) {
    TS_Point p = ts.getPoint();
    
    // Display raw coordinates
    Serial.print("RAW Touch - X: ");
    Serial.print(p.x);
    Serial.print(", Y: ");
    Serial.print(p.y);
    Serial.print(", Pressure: ");
    Serial.println(p.z);
    
    // Map to screen coordinates
    int screenX = map(p.x, 200, 3700, 0, 480);
    int screenY = map(p.y, 240, 3800, 0, 320);
    
    screenX = constrain(screenX, 0, 479);
    screenY = constrain(screenY, 0, 319);
    
    Serial.print("MAPPED Touch - X: ");
    Serial.print(screenX);
    Serial.print(", Y: ");
    Serial.println(screenY);
    
    // Draw touch point on screen
    tft.fillCircle(screenX, screenY, 5, TFT_RED);
    
    // Update display with coordinates
    tft.fillRect(10, 100, 460, 60, TFT_BLACK);
    tft.setCursor(10, 100);
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(1);
    tft.print("Raw: ");
    tft.print(p.x);
    tft.print(",");
    tft.println(p.y);
    tft.print("Screen: ");
    tft.print(screenX);
    tft.print(",");
    tft.println(screenY);
    
    delay(100); // Debounce
  } else {
    // Periodically check touch controller status
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 2000) {
      Serial.println("Waiting for touch...");
      lastCheck = millis();
    }
  }
  
  delay(50);
}