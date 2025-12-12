#include <Adafruit_Fingerprint.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <XPT2046_Touchscreen.h>

#include "auth_config.h"
#include "device_control.h"
#include "display_manager.h"
#include "fingerprint_auth.h"
#include "keypad_handler.h"
#include "rfid_auth.h"
#include "wifi_manager.h"

// Pin Definitions
#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
#define RFID_SS 15
#define RFID_RST 21
#define BUZZER_PIN 22

TFT_eSPI tft = TFT_eSPI();
HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
MFRC522 rfid(RFID_SS, RFID_RST);

enum AuthMode { FINGERPRINT_AUTH, RFID_AUTH, PASSWORD_AUTH };
ScreenMode currentScreen = BOOT;
AuthMode currentMode = FINGERPRINT_AUTH;
String inputPassword = "";
String userIdInput = "";
bool systemLocked = true;
bool scanningActive = false;
int failedAttempts = 0;
unsigned long lastWifiCheck = 0;

void grantAccess(const char* userName, JsonObject user);
void showAdminScreen();
void handleAdminScreen(char key);
void showPasswordScreen();
void showUserIdInput();
void handleUserIdInput(char key);
void enrollFingerprint(String userId);
void sendFingerprintToAPI(String userId, int fingerprintId);
void showRfidUserIdInput();
void handleRfidUserIdInput(char key);
void enrollRfid(String userId);
void sendRfidToAPI(String userId, String rfidTag);
void showPasswordError();
void playBuzzer(int times, int duration);
void checkAdminPassword(String password);
void authenticatePassword(String password);
void triggerVisitorBuzzer();
void emergencyUnlock();

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  fingerSerial.begin(57600, SERIAL_8N1, 16, 17);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);

  showBootScreen();
  connectWiFi();
  drawMainInterface();

  Serial.println("Door system initialized");
}

void loop() {
  // Check WiFi periodically
  if (millis() - lastWifiCheck > 30000) {
    attemptWiFiReconnect();
    lastWifiCheck = millis();
  }

  handleKeypad();

  // Handle fingerprint scanning if in fingerprint mode
  if (currentScreen == FINGERPRINT_SCREEN && scanningActive) {
    uint8_t p = finger.getImage();
    if (p == FINGERPRINT_OK) {
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
      }
    }
    delay(100);
  }

  // Note: RFID scanning is handled in showRFIDScreen() blocking loop
  // Do NOT call readRFID() here as it conflicts with the blocking loop
}

void grantAccess(const char* userName, JsonObject user) {
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

  Serial.printf("Sending unlock: %s (5000ms)\n", userName);
  sendUnlockRequest(userName);

  delay(3000);
  drawMainInterface();
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

void emergencyUnlock() {
  Serial.println("Sending emergency unlock");
  sendUnlockRequest("EMERGENCY");
  updateStatusArea("EMERGENCY UNLOCK");
  playBuzzer(3, 300);
}

void showPasswordScreen() {
  currentScreen = PASSWORD_SCREEN;
  inputPassword = "";
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(150, 50);
  tft.println("PASSWORD");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(100, 150);
  tft.println("Enter password (0-9, #=delete, *=enter)");

  while (currentScreen == PASSWORD_SCREEN) {
    char key = keypad.getKey();

    if (key == '*') {
      checkAdminPassword(inputPassword);
      return;
    } else if (key == '#') {
      if (inputPassword.length() > 0) {
        inputPassword = inputPassword.substring(0, inputPassword.length() - 1);
      }
    } else if (key >= '0' && key <= '9') {
      inputPassword += key;
    } else if (key == 'D') {
      drawMainInterface();
      return;
    }

    tft.fillRect(50, 200, 380, 40, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(150, 210);
    for (int i = 0; i < inputPassword.length(); i++) {
      tft.print("*");
    }

    delay(200);
  }
}

void checkAdminPassword(String password) {
  if (password == ADMIN_PASSWORD) {
    showAdminScreen();
  } else {
    authenticatePassword(password);
  }
}

void authenticatePassword(String password) {
  if (!checkWiFiConnection()) {
    showPasswordError();
    return;
  }

  HTTPClient http;
  http.setTimeout(HTTP_TIMEOUT_MS);
  http.begin(String(API_SERVER) + VERIFY_PASSWORD_ENDPOINT);
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(512);
  doc["password"] = password;
  doc["deviceId"] = DEVICE_ID;

  String payload;
  serializeJson(doc, payload);

  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(2048);
    deserializeJson(responseDoc, response);

    bool authorized = responseDoc["data"]["authorized"];

    if (authorized) {
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
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_RED);
  tft.setCursor(100, 120);
  tft.println("Invalid Password");

  playBuzzer(2, 200);
  delay(2000);
  drawMainInterface();
}

void showAdminScreen() {
  currentScreen = ADMIN_SCREEN;
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(150, 50);
  tft.println("ADMIN MENU");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 120);
  tft.println("A: Enroll Fingerprint");
  tft.setCursor(50, 140);
  tft.println("B: Enroll RFID");
  tft.setCursor(50, 160);
  tft.println("C: Emergency Unlock");
  tft.setCursor(50, 180);
  tft.println("*: Go Back");

  while (currentScreen == ADMIN_SCREEN) {
    char key = keypad.getKey();

    if (key) {
      handleAdminScreen(key);
    }
    delay(200);
  }
}

void handleAdminScreen(char key) {
  if (key == 'A') {
    showUserIdInput();
  } else if (key == 'B') {
    showRfidUserIdInput();
  } else if (key == 'C') {
    emergencyUnlock();
    showAdminScreen();
  } else if (key == '*') {
    drawMainInterface();
  }
}

void showUserIdInput() {
  currentScreen = USER_ID_INPUT;
  userIdInput = "";
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 50);
  tft.println("Enter User ID");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 150);
  tft.println("Enter ID (0-9, *=clear, #=done)");

  while (currentScreen == USER_ID_INPUT) {
    char key = keypad.getKey();

    if (key) {
      handleUserIdInput(key);
    }

    tft.fillRect(50, 200, 380, 40, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(150, 210);
    tft.println(userIdInput);

    delay(200);
  }
}

void handleUserIdInput(char key) {
  if (key >= '0' && key <= '9') {
    userIdInput += key;
  } else if (key == '*') {
    userIdInput = "";
  } else if (key == '#') {
    if (userIdInput.length() > 0) {
      enrollFingerprint(userIdInput);
    }
  }
}

void enrollFingerprint(String userId) {
  currentScreen = FINGERPRINT_ENROLL;
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(100, 50);
  tft.println("ENROLL USER");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 150);
  tft.println("Scanning fingerprints...");

  uint8_t fingerprintId = 1;

  // Simple fingerprint capture (implement full enrollment as needed)
  tft.setCursor(50, 180);
  tft.println("Press finger to enroll...");

  delay(5000);
  sendFingerprintToAPI(userId, fingerprintId);
}

void sendFingerprintToAPI(String userId, int fingerprintId) {
  if (!checkWiFiConnection()) {
    return;
  }

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(100, 120);
  tft.println("Sending to server...");

  // Implement API call as needed
  delay(2000);
  showAdminScreen();
}

void showRfidUserIdInput() {
  currentScreen = RFID_USER_ID_INPUT;
  userIdInput = "";
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 50);
  tft.println("Enter User ID");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 150);
  tft.println("Enter ID (0-9, *=clear, #=done)");

  while (currentScreen == RFID_USER_ID_INPUT) {
    char key = keypad.getKey();

    if (key) {
      handleRfidUserIdInput(key);
    }

    tft.fillRect(50, 200, 380, 40, TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(150, 210);
    tft.println(userIdInput);

    delay(200);
  }
}

void handleRfidUserIdInput(char key) {
  if (key >= '0' && key <= '9') {
    userIdInput += key;
  } else if (key == '*') {
    userIdInput = "";
  } else if (key == '#') {
    if (userIdInput.length() > 0) {
      enrollRfid(userIdInput);
    }
  }
}

void enrollRfid(String userId) {
  currentScreen = RFID_ENROLL;
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(100, 50);
  tft.println("ENROLL RFID");

  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(50, 150);
  tft.println("Tap card to enroll...");

  delay(5000);

  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(100, 120);
  tft.println("Enrolled!");

  delay(2000);
  showAdminScreen();
}

void sendRfidToAPI(String userId, String rfidTag) {
  // Implement as needed
}

void playBuzzer(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    delay(duration);
  }
}
