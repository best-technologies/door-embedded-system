#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_Fingerprint.h>
#include <MFRC522.h>
#include <Keypad.h>

// Pin definitions
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define RFID_SS 15
#define BUZZER_PIN 22
#define RFID_RST 34

// Device objects
TFT_eSPI tft = TFT_eSPI();
HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
MFRC522 rfid(RFID_SS, RFID_RST);

// Network config
const char* ssid = "max-router2.4G";
const char* password = "maximus123";
const char* server = "http:// 192.168.1.179:1000";
const char* deviceId = "DOOR-001";
const char* adminPassword = "45236900";

// Controller ESP32 IP address (will be discovered via mDNS or hardcoded)
const char* controllerIP = "192.168.1.171";  // Update this to controller's actual IP
const uint16_t controllerPort = 80;

// System state
enum AuthMode { FINGERPRINT, RFID, PASSWORD };
enum ScreenMode { BOOT, HOME, FINGERPRINT_SCREEN, RFID_SCREEN, PASSWORD_SCREEN, ADMIN_SCREEN, USER_ID_INPUT, FINGERPRINT_ENROLL, RFID_USER_ID_INPUT, RFID_ENROLL };
ScreenMode currentScreen = BOOT;
AuthMode currentMode = FINGERPRINT;
String inputPassword = "";
String userIdInput = "";
bool systemLocked = true;
bool scanningActive = false;
int failedAttempts = 0;
bool wifiConnected = false;
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 5000; // Check every 5 seconds

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] ={32, 33, 25, 26};// {26, 25, 33, 32};
byte colPins[COLS] ={27, 14, 12, 13}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void connectWiFi();
bool checkWiFiConnection();
void attemptWiFiReconnect();

void showBootScreen();
void updateBootScreen(int dots);

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
void showAdminScreen();
void handleAdminScreen(char key);

void drawFingerprintIcon(int x, int y, uint16_t color);
void drawRFIDIcon(int x, int y, uint16_t color);
void drawPasswordScreen();

void authenticatePassword(String password);
void checkAdminPassword(String password);
void triggerVisitorBuzzer();
void showUserIdInput();
void handleUserIdInput(char key);
void enrollFingerprint(String userId);
uint8_t getFingerprintEnroll(int id);
void sendFingerprintToAPI(String userId, int fingerprintId);
void showRfidUserIdInput();
void handleRfidUserIdInput(char key);
void enrollRfid(String userId);
void sendRfidToAPI(String userId, String rfidTag);

void handleKeypad();
void showPasswordError();
void updateStatusArea(String message);
void playBuzzer(int times, int duration);

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  fingerSerial.begin(57600, SERIAL_8N1, 16, 17);
  
  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  showBootScreen();
  
  // Connect to WiFi
  connectWiFi();
  
  // Draw main interface
  drawMainInterface();
  
  Serial.println("Door system initialized");
}

void showBootScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(200, 140);
  tft.println("Booting");
}

void updateBootScreen(int dots) {
  // Clear dots area
  tft.fillRect(320, 140, 100, 30, TFT_BLACK);
  
  // Show dots
  tft.setCursor(320, 140);
  for (int i = 0; i <= dots; i++) {
    tft.print(".");
  }
}

void loop() {
  // Check WiFi periodically
  if (millis() - lastWifiCheck > wifiCheckInterval) {
    checkWiFiConnection();
    lastWifiCheck = millis();
  }
  
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
  Serial.println("[WiFi] Connecting...");
  int attempts = 0;
  int maxAttempts = 20; // 10 seconds timeout
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    if (currentScreen == BOOT) {
      updateBootScreen(attempts % 4);
    }
    delay(500);
    attempts++;
  }
  
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (wifiConnected) {
    Serial.println("[WiFi] Connected");
    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WiFi] Connection failed");
  }
}

void sendUnlockRequest(const char* reason) {
  if (!wifiConnected) {
    Serial.println("[HTTP] WiFi not connected, cannot send unlock request");
    return;
  }
  
  HTTPClient http;
  String url = String("http://") + controllerIP + "/api/v1/unlock?duration=5000&reason=" + String(reason);
  
  Serial.println("[HTTP] Sending unlock request to: " + url);
  http.begin(url);
  int httpCode = http.POST("");
  
  if (httpCode == 200) {
    Serial.println("[HTTP] ✓ Unlock request successful");
  } else {
    Serial.println("[HTTP] ✗ Unlock request failed: " + String(httpCode));
  }
  
  http.end();
}

void sendBuzzerRequest() {
  if (!wifiConnected) {
    Serial.println("[HTTP] WiFi not connected, cannot send buzzer request");
    return;
  }
  
  HTTPClient http;
  String url = String("http://") + controllerIP + "/api/v1/buzzer";
  
  Serial.println("[HTTP] Sending buzzer request to: " + url);
  http.begin(url);
  int httpCode = http.POST("");
  
  if (httpCode == 200) {
    Serial.println("[HTTP] ✓ Buzzer request successful");
  } else {
    Serial.println("[HTTP] ✗ Buzzer request failed: " + String(httpCode));
  }
  
  http.end();
}

void drawMainInterface() {
  tft.fillScreen(TFT_BLACK);
  currentScreen = HOME;
  
  // WiFi status at top
  if (!wifiConnected) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED);
    tft.setCursor(180, 5);
    tft.println("WiFi Not Connected");
  }
  
  // Title
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(140, 10);
  tft.println("DOOR ACCESS");
  
  // Button colors based on WiFi status
  uint16_t buttonColor = wifiConnected ? TFT_BLUE : TFT_DARKGREY;
  uint16_t textColor = wifiConnected ? TFT_WHITE : TFT_LIGHTGREY;
  
  // Four buttons with smaller height
  tft.fillRect(50, 50, 380, 45, buttonColor);
  tft.drawRect(50, 50, 380, 45, TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(textColor);
  tft.setCursor(60, 65);
  tft.println("Fingerprint => Press A");
  
  buttonColor = wifiConnected ? TFT_GREEN : TFT_DARKGREY;
  tft.fillRect(50, 110, 380, 45, buttonColor);
  tft.drawRect(50, 110, 380, 45, TFT_WHITE);
  tft.setCursor(60, 125);
  tft.println("RFID => Press B");
  
  buttonColor = wifiConnected ? TFT_YELLOW : TFT_DARKGREY;
  textColor = wifiConnected ? TFT_BLACK : TFT_LIGHTGREY;
  tft.fillRect(50, 170, 380, 45, buttonColor);
  tft.drawRect(50, 170, 380, 45, TFT_WHITE);
  tft.setTextColor(textColor);
  tft.setCursor(60, 185);
  tft.println("Password => Press C");
  
  buttonColor = wifiConnected ? TFT_MAGENTA : TFT_DARKGREY;
  textColor = wifiConnected ? TFT_WHITE : TFT_LIGHTGREY;
  tft.fillRect(50, 230, 380, 45, buttonColor);
  tft.drawRect(50, 230, 380, 45, TFT_WHITE);
  tft.setTextColor(textColor);
  tft.setCursor(60, 245);
  tft.println("Visitors => Press D");
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
  // Keep waiting until finger is detected
  while (true) {
    // Check for keypad input to allow exit
    char key = keypad.getKey();
    if (key == '*') {
      drawMainInterface();
      return;
    }
    
    uint8_t p = finger.getImage();
    
    if (p == FINGERPRINT_OK) {
      // Finger detected, break out of waiting loop
      break;
    }
    
    delay(100); // Small delay to prevent excessive polling
  }
  
  // Finger detected, process it
  showFingerprintStatus("Finger detected - scanning...");
  drawFingerprintIcon(240, 120, TFT_YELLOW);
  
  uint8_t result = getFingerprintID();
  
  if (result == FINGERPRINT_OK) {
    showFingerprintStatus("Match found!");
    failedAttempts = 0;
    authenticateUser(finger.fingerID, "fingerprint");
  } else if (result == FINGERPRINT_NOTFOUND) {
    failedAttempts++;
    showFingerprintStatus("No match - Attempt " + String(failedAttempts) + "/5");
    drawFingerprintIcon(240, 120, TFT_RED);
    playBuzzer(2, 200);
    delay(2000);
    
    if (failedAttempts >= 5) {
      failedAttempts = 0;
      drawMainInterface();
  } else {
      showFingerprintStatus("Place finger on sensor...");
      drawFingerprintIcon(240, 120, TFT_WHITE);
    }
  } else {
    failedAttempts++;
    showFingerprintStatus("Scan error - Attempt " + String(failedAttempts) + "/5");
    drawFingerprintIcon(240, 120, TFT_RED);
    playBuzzer(1, 300);
    delay(1500);
    
    if (failedAttempts >= 5) {
      failedAttempts = 0;
      drawMainInterface();
    } else {
      showFingerprintStatus("Place finger on sensor...");
      drawFingerprintIcon(240, 120, TFT_WHITE);
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
  
  // Check WiFi before API call
  if (!checkWiFiConnection()) {
    drawFingerprintIcon(240, 120, TFT_RED);
    showFingerprintStatus("WiFi connection failed");
    playBuzzer(2, 200);
    delay(2000);
    drawMainInterface();
    return;
  }
  
  HTTPClient http;
  http.setTimeout(10000);  // 10 second timeout
  http.begin(String(server) + "/api/v1/access/verify-fingerprint");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["fingerprintId"] = fingerID;
  doc["deviceId"] = deviceId;
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("=== FINGERPRINT AUTH DEBUG ===");
  Serial.println("Detected Fingerprint ID: " + String(fingerID));
  Serial.println("Request payload: " + payload);
  Serial.println("Server URL: " + String(server) + "/api/v1/access/verify-fingerprint");
  
  int httpCode = http.POST(payload);
  
  Serial.println("HTTP Code: " + String(httpCode));
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("API Response: " + response);
    
    DynamicJsonDocument responseDoc(2048);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (error) {
      Serial.println("JSON parsing failed: " + String(error.c_str()));
      return;
    }
    
    bool success = responseDoc["success"];
    bool authorized = responseDoc["data"]["authorized"];
    Serial.println("Success: " + String(success));
    Serial.println("Authorized: " + String(authorized));
    Serial.println("Data exists: " + String(responseDoc["data"].isNull() ? "false" : "true"));
    Serial.println("User exists: " + String(responseDoc["data"]["user"].isNull() ? "false" : "true"));
    Serial.println("Message: " + String(responseDoc["message"].as<const char*>()));
    
    if (!success) {
      showFingerprintStatus("ID " + String(fingerID) + " not registered");
      delay(2000);
    }
    
    if (success && authorized) {
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
      String reason = responseDoc["data"]["reason"] | "Access denied";
      Serial.println("Denial reason: " + reason);
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
    drawFingerprintIcon(240, 120, TFT_RED);
    String errorMsg = "API Error " + String(httpCode);
    if (httpCode == -1) errorMsg = "DNS/Connection Failed";
    if (httpCode == -11) errorMsg = "Request Timeout";
    showFingerprintStatus(errorMsg + " - Attempt " + String(failedAttempts) + "/5");
    Serial.println("[HTTP] Error: " + errorMsg);
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
  
  http.end();
}

void authenticateCard(String cardID) {
  drawRFIDIcon(240, 120, TFT_YELLOW); // Show scanning
  
  // Check WiFi before API call
  if (!checkWiFiConnection()) {
    drawRFIDIcon(240, 120, TFT_RED);
    playBuzzer(2, 200);
    delay(2000);
    drawRFIDIcon(240, 120, TFT_WHITE);
    return;
  }
  
  HTTPClient http;
  http.begin(String(server) + "/api/v1/access/verify-rfid");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["rfidTag"] = cardID; // Remove 0x prefix
  
  Serial.println("=== RFID AUTH DEBUG ===");
  Serial.println("Detected RFID: " + cardID);
  Serial.println("Expected RFID: 73A079F6");
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
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(50, 100);
  tft.println("Name: " + String(userName));
  tft.setCursor(50, 130);
  tft.println("Dept: " + String(user["department"].as<const char*>()));
  tft.setCursor(50, 160);
  tft.println("Email: " + String(user["email"].as<const char*>()));
  tft.setCursor(50, 190);
  tft.println("Status: " + String(user["status"].as<const char*>()));
  
  playBuzzer(1, 500);
  
  // Send unlock command to controller via HTTP
  Serial.printf("Sending unlock: %s (5000ms)\n", userName);
  sendUnlockRequest(userName);
  
  delay(3000);
  drawMainInterface();
}

void emergencyUnlock() {
  // Send unlock command via HTTP
  Serial.println("Sending emergency unlock");
  sendUnlockRequest("EMERGENCY");
  
  updateStatusArea("EMERGENCY UNLOCK");
  playBuzzer(3, 300);
}

void showFingerprintScreen() {
  currentScreen = FINGERPRINT_SCREEN;
  scanningActive = true;
  failedAttempts = 0;
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 30);
  tft.println("FINGERPRINT SCAN");
  
  drawFingerprintIcon(240, 120, TFT_WHITE);
  
  showFingerprintStatus("Place finger on sensor...");
  
  tft.setCursor(180, 250);
  tft.setTextColor(TFT_YELLOW);
  tft.println("Press * to go back");
}

void showFingerprintStatus(String message) {
  // Clear status area
  tft.fillRect(50, 200, 380, 40, TFT_BLACK);
  
  // Display status message
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(120, 220);
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
      if (!wifiConnected) {
        // Check WiFi status and update if reconnected
        if (WiFi.status() == WL_CONNECTED) {
          wifiConnected = true;
          drawMainInterface();
        }
        return; // Block keypad input when WiFi not connected
      }
    
    if (key == 'A') {
        showFingerprintScreen();
      } else if (key == 'B') {
        showRFIDScreen();
      } else if (key == 'C') {
        showPasswordScreen();
      } else if (key == 'D') {
        triggerVisitorBuzzer();
      }
    }
    else if (currentScreen == PASSWORD_SCREEN) {
      if (key >= '0' && key <= '9') {
        inputPassword += key;
        drawPasswordScreen();
      }
      else if (key == 'A' || key == 'B' || key == 'C') {
        checkAdminPassword(inputPassword);
      }
      else if (key == 'D' || key == '#') {
        inputPassword = "";
        drawPasswordScreen();
      }
      else if (key == '*') {
        drawMainInterface();
      }
    }
    else if (currentScreen == ADMIN_SCREEN) {
      handleAdminScreen(key);
    }
    else if (currentScreen == USER_ID_INPUT) {
      handleUserIdInput(key);
    }
    else if (currentScreen == RFID_USER_ID_INPUT) {
      handleRfidUserIdInput(key);
    }
    else if (key == '*') {
      scanningActive = false;
      drawMainInterface();
    }
  }
}

void authenticatePassword(String password) {
  // Check WiFi before API call
  if (!checkWiFiConnection()) {
    showPasswordError();
    return;
  }
  
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

bool checkWiFiConnection() {
  bool wasConnected = wifiConnected;
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (!wifiConnected && wasConnected) {
    // WiFi just disconnected
    Serial.println("WiFi disconnected, attempting reconnection...");
    attemptWiFiReconnect();
  } else if (wifiConnected && !wasConnected) {
    // WiFi just reconnected
    Serial.println("WiFi reconnected!");
    if (currentScreen == HOME) {
      drawMainInterface(); // Refresh home screen
    }
  }
  
  return wifiConnected;
}

void attemptWiFiReconnect() {
  WiFi.disconnect();
  delay(100);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(500);
    attempts++;
  }
  
  wifiConnected = (WiFi.status() == WL_CONNECTED);
  
  if (currentScreen == HOME) {
    drawMainInterface(); // Update home screen status
  }
}

void triggerVisitorBuzzer() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(100, 120);
  tft.println("CALLING...");
  
  Serial.println("Sending visitor buzzer");
  sendBuzzerRequest();
  
  delay(2000);
  drawMainInterface();
}

void checkAdminPassword(String password) {
  if (password == adminPassword) {
    showAdminScreen();
  } else {
    authenticatePassword(password);
  }
}

void showAdminScreen() {
  currentScreen = ADMIN_SCREEN;
  tft.fillScreen(TFT_BLACK);
  
  // Title
  tft.setTextSize(3);
  tft.setTextColor(TFT_RED);
  tft.setCursor(150, 20);
  tft.println("ADMIN PANEL");
  
  // Add Fingerprint button
  tft.fillRect(50, 80, 380, 50, TFT_BLUE);
  tft.drawRect(50, 80, 380, 50, TFT_WHITE);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(60, 100);
  tft.println("Add Fingerprint => Press A");
  
  // Add RFID button
  tft.fillRect(50, 150, 380, 50, TFT_GREEN);
  tft.drawRect(50, 150, 380, 50, TFT_WHITE);
  tft.setCursor(60, 170);
  tft.println("Add RFID => Press B");
  
  // Settings button
  tft.fillRect(50, 220, 380, 50, TFT_YELLOW);
  tft.drawRect(50, 220, 380, 50, TFT_WHITE);
  tft.setTextColor(TFT_BLACK);
  tft.setCursor(60, 240);
  tft.println("Settings => Press C");
  
  // Back button
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(200, 290);
  tft.println("Press * to go back");
}

void handleAdminScreen(char key) {
  if (key == 'A') {
    showUserIdInput();  // Fingerprint functionality
  } else if (key == 'B') {
    showRfidUserIdInput();  // Add RFID functionality
  } else if (key == 'C') {
    // Settings functionality
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(100, 100);
    tft.println("Settings");
    tft.setCursor(100, 130);
    tft.println("Feature Coming Soon");
    delay(2000);
    showAdminScreen();
  } else if (key == '*') {
    drawMainInterface();
  }
}

void showUserIdInput() {
  currentScreen = USER_ID_INPUT;
  userIdInput = "";
  tft.fillScreen(TFT_BLACK);
  
  // Title
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 30);
  tft.println("ENTER USER ID");
  
  // Input display area
  tft.drawRect(50, 80, 380, 50, TFT_WHITE);
  tft.setCursor(60, 100);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.println("BTL-");
  
  // Instructions
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(50, 160);
  tft.println("Enter 6 digits (e.g., 251101 for BTL-25-11-01)");
  tft.setCursor(50, 180);
  tft.println("0-9: Enter digits");
  tft.setCursor(50, 200);
  tft.println("A/B/C: Confirm and start enrollment");
  tft.setCursor(50, 220);
  tft.println("D/#: Clear all");
  tft.setCursor(50, 240);
  tft.println("*: Back to admin");
}

void handleUserIdInput(char key) {
  if (key >= '0' && key <= '9' && userIdInput.length() < 6) {
    userIdInput += key;
    
    // Update display
    tft.fillRect(60, 100, 350, 30, TFT_BLACK);
    tft.setCursor(60, 100);
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN);
    
    String formatted = "BTL-";
    for (int i = 0; i < userIdInput.length(); i++) {
      if (i == 2 || i == 4) formatted += "-";
      formatted += userIdInput[i];
    }
    tft.println(formatted);
  }
  else if (key == 'A' || key == 'B' || key == 'C') {
    if (userIdInput.length() == 6) {
      String userId = "BTL-" + userIdInput.substring(0,2) + "-" + userIdInput.substring(2,4) + "-" + userIdInput.substring(4,6);
      enrollFingerprint(userId);
    }
  }
  else if (key == 'D' || key == '#') {
    userIdInput = "";
    showUserIdInput();
  }
  else if (key == '*') {
    showAdminScreen();
  }
}

void enrollFingerprint(String userId) {
  currentScreen = FINGERPRINT_ENROLL;
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(100, 30);
  tft.println("FINGERPRINT ENROLL");
  
  tft.setTextSize(1);
  tft.setCursor(50, 70);
  tft.println("User ID: " + userId);
  
  // Find next available fingerprint ID
  int nextId = 1;
  while (nextId <= 127) {
    if (finger.loadModel(nextId) != FINGERPRINT_OK) {
      break; // Found empty slot
    }
    nextId++;
  }
  
  if (nextId > 127) {
    tft.setTextColor(TFT_RED);
    tft.setCursor(50, 100);
    tft.println("Error: Fingerprint storage full");
    delay(3000);
    showAdminScreen();
    return;
  }
  
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 100);
  tft.println("Enrolling as ID: " + String(nextId));
  
  uint8_t result = getFingerprintEnroll(nextId);
  
  if (result == FINGERPRINT_OK) {
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(50, 200);
    tft.println("Enrollment successful!");
    tft.setCursor(50, 220);
    tft.println("Sending to server...");
    
    sendFingerprintToAPI(userId, nextId);
  } else {
    tft.setTextColor(TFT_RED);
    tft.setCursor(50, 200);
    tft.println("Enrollment failed!");
    delay(3000);
    showAdminScreen();
  }
}

uint8_t getFingerprintEnroll(int id) {
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(50, 120);
  tft.println("Place finger on sensor...");
  
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      delay(100);
      continue;
    } else if (p != FINGERPRINT_OK) {
      return p;
    }
  }
  
  tft.fillRect(50, 120, 400, 20, TFT_BLACK);
  tft.setCursor(50, 120);
  tft.println("Image taken, processing...");
  
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) return p;
  
  tft.fillRect(50, 120, 400, 20, TFT_BLACK);
  tft.setCursor(50, 120);
  tft.println("Remove finger");
  delay(2000);
  
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  
  tft.fillRect(50, 120, 400, 20, TFT_BLACK);
  tft.setCursor(50, 120);
  tft.println("Place same finger again...");
  
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      delay(100);
      continue;
    } else if (p != FINGERPRINT_OK) {
      return p;
    }
  }
  
  tft.fillRect(50, 120, 400, 20, TFT_BLACK);
  tft.setCursor(50, 120);
  tft.println("Image taken, creating model...");
  
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) return p;
  
  p = finger.createModel();
  if (p != FINGERPRINT_OK) return p;
  
  p = finger.storeModel(id);
  return p;
}

void sendFingerprintToAPI(String userId, int fingerprintId) {
  if (!checkWiFiConnection()) {
    tft.setTextColor(TFT_RED);
    tft.setCursor(50, 240);
    tft.println("WiFi connection failed");
    delay(3000);
    showAdminScreen();
    return;
  }
  
  HTTPClient http;
  http.begin(String(server) + "/api/v1/users/" + userId + "/fingerprints");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(128);
  doc["fingerprintId"] = fingerprintId;
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200 || httpCode == 201) {
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(50, 240);
    tft.println("Successfully registered!");
  } else {
    tft.setTextColor(TFT_RED);
    tft.setCursor(50, 240);
    tft.println("Server error: " + String(httpCode));
  }
  
  http.end();
  delay(3000);
  showAdminScreen();
}

void showRfidUserIdInput() {
  currentScreen = RFID_USER_ID_INPUT;
  userIdInput = "";
  tft.fillScreen(TFT_BLACK);
  
  // Title
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(100, 30);
  tft.println("ENTER USER ID FOR RFID");
  
  // Input display area
  tft.drawRect(50, 80, 380, 50, TFT_WHITE);
  tft.setCursor(60, 100);
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN);
  tft.println("BTL-");
  
  // Instructions
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(50, 160);
  tft.println("Enter 6 digits (e.g., 251101 for BTL-25-11-01)");
  tft.setCursor(50, 180);
  tft.println("0-9: Enter digits");
  tft.setCursor(50, 200);
  tft.println("A/B/C: Confirm and scan RFID card");
  tft.setCursor(50, 220);
  tft.println("D/#: Clear all");
  tft.setCursor(50, 240);
  tft.println("*: Back to admin");
}

void handleRfidUserIdInput(char key) {
  if (key >= '0' && key <= '9' && userIdInput.length() < 6) {
    userIdInput += key;
    
    // Update display
    tft.fillRect(60, 100, 350, 30, TFT_BLACK);
    tft.setCursor(60, 100);
    tft.setTextSize(3);
    tft.setTextColor(TFT_CYAN);
    
    String formatted = "BTL-";
    for (int i = 0; i < userIdInput.length(); i++) {
      if (i == 2 || i == 4) formatted += "-";
      formatted += userIdInput[i];
    }
    tft.println(formatted);
  }
  else if (key == 'A' || key == 'B' || key == 'C') {
    if (userIdInput.length() == 6) {
      String userId = "BTL-" + userIdInput.substring(0,2) + "-" + userIdInput.substring(2,4) + "-" + userIdInput.substring(4,6);
      enrollRfid(userId);
    }
  }
  else if (key == 'D' || key == '#') {
    userIdInput = "";
    showRfidUserIdInput();
  }
  else if (key == '*') {
    showAdminScreen();
  }
}

void enrollRfid(String userId) {
  currentScreen = RFID_ENROLL;
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 30);
  tft.println("RFID ENROLLMENT");
  
  tft.setTextSize(1);
  tft.setCursor(50, 70);
  tft.println("User ID: " + userId);
  
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(50, 100);
  tft.println("Present RFID card to reader...");
  
  drawRFIDIcon(240, 150, TFT_WHITE);
  
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 250);
  tft.println("Press * to cancel");
  
  // Wait for RFID card
  while (true) {
    char key = keypad.getKey();
    if (key == '*') {
      showAdminScreen();
      return;
    }
    
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      String cardID = "";
      for (byte i = 0; i < rfid.uid.size; i++) {
        cardID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
        cardID += String(rfid.uid.uidByte[i], HEX);
      }
      
      tft.fillRect(50, 100, 400, 20, TFT_BLACK);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(50, 100);
      tft.println("Card detected: " + cardID);
      
      drawRFIDIcon(240, 150, TFT_GREEN);
      
      tft.setCursor(50, 120);
      tft.println("Sending to server...");
      
      sendRfidToAPI(userId, cardID);
      rfid.PICC_HaltA();
      return;
    }
    
    delay(100);
  }
}

void sendRfidToAPI(String userId, String rfidTag) {
  if (!checkWiFiConnection()) {
    tft.setTextColor(TFT_RED);
    tft.setCursor(50, 200);
    tft.println("WiFi connection failed");
    delay(3000);
    showAdminScreen();
    return;
  }
  
  HTTPClient http;
  http.begin(String(server) + "/api/v1/users/" + userId + "/rfid-tags");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(128);
  doc["rfidTag"] = "0x" + rfidTag;
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200 || httpCode == 201) {
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(50, 200);
    tft.println("RFID successfully registered!");
  } else {
    tft.setTextColor(TFT_RED);
    tft.setCursor(50, 200);
    tft.println("Server error: " + String(httpCode));
  }
  
  http.end();
  delay(3000);
  showAdminScreen();
}

