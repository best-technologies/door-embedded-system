#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Best Technologies";
const char* password = "BestTech25";
const char* pi_server = "https://embedded-door-lock.onrender.com";

void testAuthentication();
void getUsers();
void verifyByid();

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
  
  Serial.println("WiFi/API Test Commands:");
  Serial.println("'auth' - Test authentication API");
  Serial.println("'users' - Get users list");
  Serial.println("'log' - Test attendance logging");
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command == "auth") {
      testAuthentication();
    }
    else if (command == "users") {
      getUsers();
    }
    // else if (command == "log") {
    //   logAttendance();
    // }
  }
  delay(100);
}

void testAuthentication() {
  HTTPClient http;
  http.begin(String(pi_server) + "/api/authenticate");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["method"] = "fingerprint";
  doc["fingerprint_id"] = 1;
  doc["timestamp"] = millis();
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("Testing authentication API...");
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Code: %d\n", httpCode);
    Serial.println("Response: " + response);
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void getUsers() {
  HTTPClient http;
  http.begin(String(pi_server) + "/api/v1/users");
  
  Serial.println("Getting users list...");
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Code: %d\n", httpCode);
    Serial.println("Response: " + response);
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}

void verifyByid() {
  HTTPClient http;
  http.begin(String(pi_server) + "/api/v1/access/verify-fingerprint");
  http.addHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(512);
  doc["user_name"] = "User_id";
  doc["action"] = "entry";
  doc["method"] = "test";
  
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("Testing attendance logging...");
  int httpCode = http.POST(payload);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Code: %d\n", httpCode);
    Serial.println("Response: " + response);
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
}