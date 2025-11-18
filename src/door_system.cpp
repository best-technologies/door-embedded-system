#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_Fingerprint.h>
#include <MFRC522.h>
#include <Keypad.h>

// Pin definitions
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define TOUCH_CS 21
#define TOUCH_IRQ 22
#define RFID_SS 25
#define RFID_RST 26
#define BUZZER_PIN 26

// Device objects
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
MFRC522 rfid(RFID_SS, RFID_RST);

// Network config
const char* ssid = "SMART_SOCKET";
const char* password = "123456789";
const char* server = "https://embedded-door-lock.onrender.com";
const char* deviceId = "DOOR-001";

// Controller ESP32 MAC address
uint8_t controllerMAC[] = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56};

// System state
enum AuthMode { FINGERPRINT, RFID, PASSWORD };
enum ScreenMode { HOME, FINGERPRINT_SCREEN, RFID_SCREEN, PASSWORD_SCREEN };
ScreenMode currentScreen = HOME;
AuthMode currentMode = FINGERPRINT;
String inputPassword = "";
bool systemLocked = true;
bool scanningActive = false;
int failedAttempts = 0;

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ESP-NOW message structure
typedef struct {
  bool unlock;
  int duration;
  char reason[32];
} UnlockMessage;

void initESPNow();
void connectWiFi();

void drawMainInterface();
void drawButton(int x, int y, int w, int h, uint16_t color, const char* text);

void showFingerprintStatus(String message);
void handleFingerprint();
void handleRFID();
uint8_t getFingerprintID();

void authenticateUser(int fingerID, const char* method);
void authenticateCard(String cardID);
void grantAccess(const char* userName, JsonObject user);
void emergencyUnlock();

void showFingerprintScreen();
void showRFIDScreen();
void showPasswordScreen();

void drawFingerprintIcon(int x, int y, uint16_t color);
void drawRFIDIcon(int x, int y, uint16_t color);
void drawPasswordScreen();

void authenticatePassword(String password);

void handleKeypad();
void showPasswordError();
void updateStatusArea(String message);
void playBuzzer(int times, int duration);

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  // Initialize touch
  ts.begin();
  ts.setRotation(1);
  
  // Initialize fingerprint sensor
  fingerSerial.begin(57600, SERIAL_8N1, 16, 17);
  finger.begin(57600);
  
  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Connect to WiFi
  connectWiFi();
  
  // Initialize ESP-NOW
  initESPNow();
  
  // Draw main interface
  drawMainInterface();
  
  Serial.println("Door system initialized");
}

void loop() {
  handleKeypad();
  
  if (currentScreen == FINGERPRINT_SCREEN) {
    handleFingerprint();
  } else if (currentScreen == RFID_SCREEN && scanningActive) {
    handleRFID();
  }
  
  delay(100);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_WHITE);
  tft.print("Connecting WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(".");
  }
  
  tft.println(" Connected!");
  delay(1000);
}

void initESPNow() {
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  
  // Add controller peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, controllerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
  }
}

void drawMainInterface() {
  tft.fillScreen(TFT_BLACK);
  currentScreen = HOME;
  
  // Title
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 20);
  tft.println("DOOR ACCESS");
  
  // Long buttons with keypad instructions
  tft.fillRect(50, 80, 380, 50, TFT_BLUE);
  tft.drawRect(50, 80, 380, 50, TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(60, 100);
  tft.println("Fingerprint => Press A");
  
  tft.fillRect(50, 150, 380, 50, TFT_GREEN);
  tft.drawRect(50, 150, 380, 50, TFT_WHITE);
  tft.setCursor(60, 170);
  tft.println("RFID => Press B");
  
  tft.fillRect(50, 220, 380, 50, TFT_YELLOW);
  tft.drawRect(50, 220, 380, 50, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(60, 240);
  tft.println("Password => Press C");
}

void drawButton(int x, int y, int w, int h, uint16_t color, const char* text) {
  tft.fillRect(x, y, w, h, color);
  tft.drawRect(x, y, w, h, TFT_WHITE);
  
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(1);
  int textX = x + (w - strlen(text) * 6) / 2;
  int textY = y + (h - 8) / 2;
  tft.setCursor(textX, textY);
  tft.println(text);
}

void handleFingerprint() {
  // Check if finger is present first
  uint8_t p = finger.getImage();
  
  if (p == FINGERPRINT_NOFINGER) {
    if (scanningActive) {
      scanningActive = false;
      showFingerprintStatus("Waiting for finger...");
      drawFingerprintIcon(240, 120, TFT_WHITE);
    }
  } else{ 
    // Finger detected, start scanning process
      scanningActive = true;
      showFingerprintStatus("Finger detected - scanning...");
      drawFingerprintIcon(240, 120, TFT_YELLOW);
      
      // Now do the full fingerprint scan
      uint8_t result = getFingerprintID();
      
      if (result == FINGERPRINT_OK) {
        showFingerprintStatus("Match found!");
        failedAttempts = 0; // Reset counter on success
        authenticateUser(finger.fingerID, "fingerprint");
      } else if (result == FINGERPRINT_NOTFOUND) {
        failedAttempts++;
        showFingerprintStatus("No match - Attempt " + String(failedAttempts) + "/5");
        drawFingerprintIcon(240, 120, TFT_RED);
        playBuzzer(2, 200);
        delay(2000);
        scanningActive = false;
        
        if (failedAttempts >= 5) {
          failedAttempts = 0;
          drawMainInterface();
        } else {
          showFingerprintStatus("Waiting for finger...");
          drawFingerprintIcon(240, 120, TFT_WHITE);
        }
      } else {
        failedAttempts++;
        showFingerprintStatus("Scan error - Attempt " + String(failedAttempts) + "/5");
        drawFingerprintIcon(240, 120, TFT_RED);
        playBuzzer(1, 300);
        delay(1500);
        scanningActive = false;
        
        if (failedAttempts >= 5) {
          failedAttempts = 0;
          drawMainInterface();
        } else {
          showFingerprintStatus("Waiting for finger...");
          drawFingerprintIcon(240, 120, TFT_WHITE);
        }
      }
    }
}

void handleRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;
  
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    cardID += String(rfid.uid.uidByte[i], HEX);
  }
  
  authenticateCard(cardID);
  rfid.PICC_HaltA();
}

void authenticateUser(int fingerID, const char* method) {
  drawFingerprintIcon(240, 120, TFT_YELLOW); // Show scanning
  
  HTTPClient http;
  http.begin(String(server) + "/api/v1/access/verify-fingerprint");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["fingerprintId"] = fingerID;
  doc["deviceId"] = deviceId;
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(2048);
    deserializeJson(responseDoc, response);
    
    if (responseDoc["success"] && responseDoc["data"]["authorized"]) {
      drawFingerprintIcon(240, 120, TFT_GREEN); // Success
      delay(1000);
      JsonObject user = responseDoc["data"]["user"];
      String firstName = user["firstName"];
      String lastName = user["lastName"];
      String fullName = firstName + " " + lastName;
      scanningActive = false;
      grantAccess(fullName.c_str(), user);
      return; // Exit to prevent going back to fingerprint screen
    } else {
      failedAttempts++;
      drawFingerprintIcon(240, 120, TFT_RED); // Failed
      showFingerprintStatus("Access denied - Attempt " + String(failedAttempts) + "/5");
      playBuzzer(2, 200);
      delay(2000);
      scanningActive = false;
      
      if (failedAttempts >= 5) {
        failedAttempts = 0;
        drawMainInterface();
      } else {
        showFingerprintStatus("Waiting for finger...");
        drawFingerprintIcon(240, 120, TFT_WHITE);
      }
      return;
    }
  } else {
    failedAttempts++;
    drawFingerprintIcon(240, 120, TFT_RED); // Error
    showFingerprintStatus("Connection error - Attempt " + String(failedAttempts) + "/5");
    playBuzzer(2, 200);
    delay(2000);
    scanningActive = false;
    
    if (failedAttempts >= 5) {
      failedAttempts = 0;
      drawMainInterface();
    } else {
      showFingerprintStatus("Waiting for finger...");
      drawFingerprintIcon(240, 120, TFT_WHITE);
    }
  }
  
  http.end();
}

void authenticateCard(String cardID) {
  drawRFIDIcon(240, 120, TFT_YELLOW); // Show scanning
  
  HTTPClient http;
  http.begin(String(server) + "/api/v1/access/verify-rfid");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["rfidTag"] = "0x" + cardID;
  doc["deviceId"] = deviceId;
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(2048);
    deserializeJson(responseDoc, response);
    
    if (responseDoc["success"] && responseDoc["data"]["authorized"]) {
      drawRFIDIcon(240, 120, TFT_GREEN); // Success
      delay(1000);
      JsonObject user = responseDoc["data"]["user"];
      String firstName = user["firstName"];
      String lastName = user["lastName"];
      String fullName = firstName + " " + lastName;
      scanningActive = false;
      grantAccess(fullName.c_str(), user);
    } else {
      drawRFIDIcon(240, 120, TFT_RED); // Failed
      playBuzzer(2, 200);
      delay(2000);
      drawRFIDIcon(240, 120, TFT_WHITE);
    }
  } else {
    drawRFIDIcon(240, 120, TFT_RED); // Error
    playBuzzer(2, 200);
    delay(2000);
    drawRFIDIcon(240, 120, TFT_WHITE);
  }
  
  http.end();
}

void grantAccess(const char* userName, JsonObject user) {
  // Display user info
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(50, 50);
  tft.println("ACCESS GRANTED");
  
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(50, 100);
  tft.println("Name: " + String(userName));
  tft.setCursor(50, 120);
  tft.println("Dept: " + String(user["department"].as<const char*>()));
  tft.setCursor(50, 140);
  tft.println("Email: " + String(user["email"].as<const char*>()));
  tft.setCursor(50, 160);
  tft.println("Status: " + String(user["status"].as<const char*>()));
  
  playBuzzer(1, 500);
  
  // Send unlock command to controller ESP32
  UnlockMessage msg;
  msg.unlock = true;
  msg.duration = 5000; // 5 seconds
  strcpy(msg.reason, userName);
  
  esp_now_send(controllerMAC, (uint8_t*)&msg, sizeof(msg));
  
  delay(3000);
  drawMainInterface();
}

void emergencyUnlock() {
  UnlockMessage msg;
  msg.unlock = true;
  msg.duration = 10000; // 10 seconds
  strcpy(msg.reason, "EMERGENCY");
  
  esp_now_send(controllerMAC, (uint8_t*)&msg, sizeof(msg));
  
  updateStatusArea("EMERGENCY UNLOCK");
  playBuzzer(3, 300);
}

void showFingerprintScreen() {
  currentScreen = FINGERPRINT_SCREEN;
  scanningActive = false;
  failedAttempts = 0; // Reset counter when entering screen
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 30);
  tft.println("FINGERPRINT SCAN");
  
  drawFingerprintIcon(240, 120, TFT_WHITE);
  
  showFingerprintStatus("Waiting for finger...");
  
  tft.setCursor(180, 250);
  tft.setTextColor(TFT_YELLOW);
  tft.println("Press * to go back");
}

void showFingerprintStatus(String message) {
  // Clear status area
  tft.fillRect(50, 70, 380, 40, TFT_BLACK);
  
  // Display status message
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(60, 85);
  tft.println(message);
  
  Serial.println("Fingerprint: " + message);
}

void showRFIDScreen() {
  currentScreen = RFID_SCREEN;
  scanningActive = true;
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(160, 30);
  tft.println("RFID SCAN");
  
  tft.setTextSize(1);
  tft.setCursor(170, 70);
  tft.println("Present RFID card");
  
  drawRFIDIcon(240, 120, TFT_WHITE);
  
  tft.setCursor(180, 250);
  tft.setTextColor(TFT_YELLOW);
  tft.println("Press * to go back");
}

void showPasswordScreen() {
  currentScreen = PASSWORD_SCREEN;
  inputPassword = "";
  drawPasswordScreen();
}

void drawFingerprintIcon(int x, int y, uint16_t color) {
  // Draw simple fingerprint pattern
  for (int i = 0; i < 5; i++) {
    tft.drawCircle(x, y, 20 + i*8, color);
  }
  // Add some lines for fingerprint ridges
  for (int i = 0; i < 8; i++) {
    tft.drawLine(x-30, y-20+i*5, x+30, y-20+i*5, color);
  }
}

void drawRFIDIcon(int x, int y, uint16_t color) {
  // Draw RFID card shape
  tft.drawRect(x-40, y-25, 80, 50, color);
  tft.fillRect(x-35, y-20, 70, 40, TFT_BLACK);
  
  // Draw RFID waves
  for (int i = 1; i <= 3; i++) {
    tft.drawCircle(x-20, y, i*8, color);
  }
  
  // Draw chip
  tft.fillRect(x+10, y-10, 20, 20, color);
}

void drawPasswordScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // Title
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(150, 30);
  tft.println("ENTER PASSWORD");
  
  // Password display area
  tft.drawRect(50, 80, 380, 50, TFT_WHITE);
  tft.setCursor(60, 100);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  
  // Show asterisks
  String asterisks = "";
  for (int i = 0; i < inputPassword.length(); i++) {
    asterisks += "*";
  }
  tft.println(asterisks);
  
  // Instructions
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(50, 160);
  tft.println("0-9: Enter digits");
  tft.setCursor(50, 180);
  tft.println("A/B/C: Submit password");
  tft.setCursor(50, 200);
  tft.println("D/#: Clear all");
  tft.setCursor(50, 220);
  tft.println("*: Back to home");
}

void handleKeypad() {
  char key = keypad.getKey();
  
  if (key) {
    if (currentScreen == HOME) {
      if (key == 'A') {
        showFingerprintScreen();
      } else if (key == 'B') {
        showRFIDScreen();
      } else if (key == 'C') {
        showPasswordScreen();
      }
    }
    else if (currentScreen == PASSWORD_SCREEN) {
      if (key >= '0' && key <= '9') {
        inputPassword += key;
        drawPasswordScreen();
      }
      else if (key == 'A' || key == 'B' || key == 'C') {
        authenticatePassword(inputPassword);
      }
      else if (key == 'D' || key == '#') {
        inputPassword = "";
        drawPasswordScreen();
      }
      else if (key == '*') {
        drawMainInterface();
      }
    }
    else if (key == '*') {
      scanningActive = false;
      drawMainInterface();
    }
  }
}

void authenticatePassword(String password) {
  HTTPClient http;
  http.begin(String(server) + "/api/v1/access/verify-keypad");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["password"] = password;
  doc["deviceId"] = deviceId;
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(2048);
    deserializeJson(responseDoc, response);
    
    if (responseDoc["success"] && responseDoc["data"]["authorized"]) {
      JsonObject user = responseDoc["data"]["user"];
      String firstName = user["firstName"];
      String lastName = user["lastName"];
      String fullName = firstName + " " + lastName;
      grantAccess(fullName.c_str(), user);
    } else {
      showPasswordError();
    }
  } else {
    showPasswordError();
  }
  
  http.end();
}

void showPasswordError() {
  tft.fillRect(50, 260, 380, 30, TFT_RED);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(180, 270);
  tft.println("INVALID PASSWORD");
  playBuzzer(2, 200);
  inputPassword = "";
  delay(2000);
  drawPasswordScreen();
}

void updateStatusArea(String message) {
  tft.fillRect(51, 161, 378, 98, TFT_BLACK);
  tft.setCursor(60, 180);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN);
  tft.println(message);
}

void playBuzzer(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}

uint8_t getFingerprintID() {
  showFingerprintStatus("Processing image...");
  
  uint8_t p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      break;
    case FINGERPRINT_IMAGEMESS:
      showFingerprintStatus("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      showFingerprintStatus("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      showFingerprintStatus("No features found");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      showFingerprintStatus("Invalid image");
      return p;
    default:
      showFingerprintStatus("Processing error");
      return p;
  }

  showFingerprintStatus("Searching database...");
  p = finger.fingerSearch();
  
  if (p == FINGERPRINT_OK) {
    showFingerprintStatus("Match found! ID: " + String(finger.fingerID));
    return FINGERPRINT_OK;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    showFingerprintStatus("Search error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    showFingerprintStatus("No match in database");
    return p;
  } else {
    showFingerprintStatus("Search failed");
    return p;
  }
}

