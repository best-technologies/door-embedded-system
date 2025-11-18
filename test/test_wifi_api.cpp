#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* server = "https://embedded-door-lock.onrender.com";
const char* deviceId = "DOOR-001";

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  Serial.println("API Test Commands:");
  Serial.println("'finger' - Test fingerprint verification");
  Serial.println("'rfid' - Test RFID verification");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command == "finger") {
      testFingerprintAuth();
    }
    else if (command == "rfid") {
      testRFIDAuth();
    }
  }
  delay(100);
}

void testFingerprintAuth() {
  HTTPClient http;
  http.begin(String(server) + "/api/v1/access/verify-fingerprint");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["fingerprintId"] = 1;
  doc["deviceId"] = deviceId;
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("Testing fingerprint verification...");
  Serial.println("Request: " + payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Code: %d\n", httpCode);
    Serial.println("Response: " + response);
    
    if (httpCode == 200) {
      parseUserResponse(response);
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void testRFIDAuth() {
  HTTPClient http;
  http.begin(String(server) + "/api/v1/access/verify-rfid");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["rfidTag"] = "0xA1B2C3D4";
  doc["deviceId"] = deviceId;
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("Testing RFID verification...");
  Serial.println("Request: " + payload);
  
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Code: %d\n", httpCode);
    Serial.println("Response: " + response);
    
    if (httpCode == 200) {
      parseUserResponse(response);
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void parseUserResponse(String response) {
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, response);
  
  bool success = doc["success"];
  String message = doc["message"];
  
  Serial.println("\n=== PARSED RESPONSE ===");
  Serial.println("Success: " + String(success ? "true" : "false"));
  Serial.println("Message: " + message);
  
  if (success && doc["data"]["authorized"]) {
    JsonObject user = doc["data"]["user"];
    
    String firstName = user["firstName"];
    String lastName = user["lastName"];
    String email = user["email"];
    String department = user["department"];
    String status = user["status"];
    
    Serial.println("\n=== USER INFO ===");
    Serial.println("Name: " + firstName + " " + lastName);
    Serial.println("Email: " + email);
    Serial.println("Department: " + department);
    Serial.println("Status: " + status);
    Serial.println("Access: GRANTED");
  } else {
    Serial.println("Access: DENIED");
    if (doc["data"]["reason"]) {
      Serial.println("Reason: " + String(doc["data"]["reason"].as<const char*>()));
    }
  }
  Serial.println("=====================\n");
}