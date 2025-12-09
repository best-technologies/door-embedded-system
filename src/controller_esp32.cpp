#include <WiFi.h>
#include <WebServer.h>

// Pin definitions
#define RELAY_PIN 13
#define LED_RED 4
#define LED_GREEN 2
#define BUZZER_PIN 12

// WiFi credentials
const char* ssid = "SMART_SOCKET";
const char* password = "123456789";

// Web server on port 80
WebServer server(80);

// Function declarations
void handleUnlock();
void handleBuzzer();
void handleNotFound();
void unlockDoor(int duration, const char* reason);
void soundVisitorBuzzer();

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("    WiFi CONTROLLER");
  Serial.println("========================================");
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi connection failed");
  }
  
  // Setup web server routes
  server.on("/api/unlock", HTTP_POST, handleUnlock);
  server.on("/api/buzzer", HTTP_POST, handleBuzzer);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("✓ HTTP Server started on port 80");
}

void loop() {
  server.handleClient();
  
  // Print status every 10 seconds
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 10000) {
    Serial.println("[Controller] Server listening...");
    lastStatus = millis();
  }
  delay(100);
}

void handleUnlock() {
  Serial.println("[API] Received unlock request");
  
  int duration = 5000;  // Default 5 seconds
  
  // Parse duration from request if provided
  if (server.hasArg("duration")) {
    duration = server.arg("duration").toInt();
  }
  
  String reason = "Remote";
  if (server.hasArg("reason")) {
    reason = server.arg("reason");
  }
  
  unlockDoor(duration, reason.c_str());
  
  // Send JSON response
  server.sendHeader("Content-Type", "application/json");
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Door unlocked\"}");
}

void handleBuzzer() {
  Serial.println("[API] Received buzzer request");
  
  soundVisitorBuzzer();
  
  // Send JSON response
  server.sendHeader("Content-Type", "application/json");
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Buzzer activated\"}");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

void unlockDoor(int duration, const char* reason) {
  Serial.printf("🔓 UNLOCKING for %s (%dms)\n", reason, duration);
  
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  
  delay(duration);
  
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  
  Serial.println("🔒 Locked");
}

void soundVisitorBuzzer() {
  Serial.println("🔔 VISITOR BUZZER");
  
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  }
}