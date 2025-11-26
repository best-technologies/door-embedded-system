#include <esp_now.h>
#include <WiFi.h>

// Relay control pin
#define RELAY_PIN 2
#define LED_RED 4
#define LED_GREEN 16
#define BUZZER_PIN 15

// Message structure (must match door ESP32)
typedef struct {
  bool unlock;
  int duration;
  char reason[32];
} UnlockMessage;

// Function declarations
void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len);
void unlockDoor(int duration, const char* reason);

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Lock engaged
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH); // Indicate ready
  
  // Set device as WiFi Station
  WiFi.mode(WIFI_STA);
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  
  // Register callback for receiving data
  esp_now_register_recv_cb(onDataReceive);
  
  Serial.println("Controller ESP32 ready");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Just keep the system alive with minimal delay
  delay(10);
}

void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  UnlockMessage msg;
  memcpy(&msg, incomingData, sizeof(msg));
  
  Serial.printf("Received unlock command: %s for %d ms\n", msg.reason, msg.duration);
  
  if (msg.unlock) {
    unlockDoor(msg.duration, msg.reason);
  }
}

void unlockDoor(int duration, const char* reason) {
  Serial.printf("Unlocking door for %s (%d ms)\n", reason, duration);
  
  // Activate relay (unlock)
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);
  
  // Keep unlocked for specified duration
  delay(duration);
  
  // Lock again
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);
  
  Serial.println("Door locked");
}