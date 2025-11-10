#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <esp_now.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_Fingerprint.h>
#include <MFRC522.h>

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
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* pi_server = "https://embedded-door-lock.onrender.com"; // Raspberry Pi IP

// Controller ESP32 MAC address
uint8_t controllerMAC[] = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56};

// System state
enum AuthMode { FINGERPRINT, RFID, PASSWORD };
AuthMode currentMode = FINGERPRINT;
String tempPassword = "";
bool systemLocked = true;

// ESP-NOW message structure
typedef struct {
  bool unlock;
  int duration;
  char reason[32];
} UnlockMessage;

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
  handleTouch();
  
  switch(currentMode) {
    case FINGERPRINT:
      handleFingerprint();
      break;
    case RFID:
      handleRFID();
      break;
    case PASSWORD:
      // Password handled via touch interface
      break;
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
  
  // Title
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(120, 20);
  tft.println("DOOR ACCESS");
  
  // Mode buttons
  drawButton(50, 80, 120, 60, TFT_BLUE, "FINGERPRINT");
  drawButton(190, 80, 120, 60, TFT_GREEN, "RFID");
  drawButton(330, 80, 120, 60, TFT_YELLOW, "PASSWORD");
  
  // Status area
  tft.drawRect(50, 160, 380, 100, TFT_WHITE);
  tft.setCursor(60, 180);
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN);
  tft.println("Place finger on sensor");
  
  // Emergency unlock button
  drawButton(180, 280, 120, 30, TFT_RED, "EMERGENCY");
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

void handleTouch() {
  if (!ts.touched()) return;
  
  TS_Point p = ts.getPoint();
  int x = map(p.x, 200, 3700, 0, 480);
  int y = map(p.y, 240, 3800, 0, 320);
  
  // Mode selection
  if (y >= 80 && y <= 140) {
    if (x >= 50 && x <= 170) {
      currentMode = FINGERPRINT;
      updateStatusArea("Place finger on sensor");
    } else if (x >= 190 && x <= 310) {
      currentMode = RFID;
      updateStatusArea("Present RFID card");
    } else if (x >= 330 && x <= 450) {
      currentMode = PASSWORD;
      showPasswordInput();
    }
  }
  
  // Emergency unlock
  if (x >= 180 && x <= 300 && y >= 280 && y <= 310) {
    emergencyUnlock();
  }
}

void handleFingerprint() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return;
  
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return;
  
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    authenticateUser(finger.fingerID, "fingerprint");
  } else {
    updateStatusArea("Access denied");
    playBuzzer(2, 200);
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
  // Send to Raspberry Pi for verification
  HTTPClient http;
  http.begin(String(pi_server) + "/api/v1/");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(1024);
  doc["method"] = method;
  doc["fingerprint_id"] = fingerID;
  doc["timestamp"] = WiFi.getTime();
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(1024);
    deserializeJson(responseDoc, response);
    
    if (responseDoc["access_granted"]) {
      grantAccess(responseDoc["user_name"]);
    } else {
      updateStatusArea("Access denied");
      playBuzzer(2, 200);
    }
  }
  
  http.end();
}

void authenticateCard(String cardID) {
  HTTPClient http;
  http.begin(String(pi_server) + "/api/authenticate");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(1024);
  doc["method"] = "rfid";
  doc["card_id"] = cardID;
  doc["timestamp"] = WiFi.getTime();
  
  String payload;
  serializeJson(doc, payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode == 200) {
    String response = http.getString();
    DynamicJsonDocument responseDoc(1024);
    deserializeJson(responseDoc, response);
    
    if (responseDoc["access_granted"]) {
      grantAccess(responseDoc["user_name"]);
    } else {
      updateStatusArea("Invalid card");
      playBuzzer(2, 200);
    }
  }
  
  http.end();
}

void grantAccess(const char* userName) {
  updateStatusArea(String("Welcome ") + userName);
  playBuzzer(1, 500);
  
  // Send unlock command to controller ESP32
  UnlockMessage msg;
  msg.unlock = true;
  msg.duration = 5000; // 5 seconds
  strcpy(msg.reason, userName);
  
  esp_now_send(controllerMAC, (uint8_t*)&msg, sizeof(msg));
  
  // Log attendance
  logAttendance(userName);
  
  delay(2000);
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

void showPasswordInput() {
  // Implement touch keypad for password input
  updateStatusArea("Password mode - TODO");
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

void logAttendance(const char* userName) {
  HTTPClient http;
  http.begin(String(pi_server) + "/api/attendance");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["user_name"] = userName;
  doc["timestamp"] = WiFi.getTime();
  doc["action"] = "entry";
  
  String payload;
  serializeJson(doc, payload);
  
  http.POST(payload);
  http.end();
}