#include <esp_now.h>
#include <WiFi.h>

// Test receiver MAC (replace with actual controller MAC)
uint8_t receiverMAC[] = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56};

typedef struct {
  bool unlock;
  int duration;
  char reason[32];
} TestMessage;

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }
  
  esp_now_register_send_cb(onDataSent);
  
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  
  Serial.println("ESP-NOW test ready");
  Serial.println("Type 'send' to test message transmission");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readString();
    input.trim();
    
    if (input == "send") {
      TestMessage msg;
      msg.unlock = true;
      msg.duration = 3000;
      strcpy(msg.reason, "TEST");
      
      esp_err_t result = esp_now_send(receiverMAC, (uint8_t*)&msg, sizeof(msg));
      
      if (result == ESP_OK) {
        Serial.println("Message sent successfully");
      } else {
        Serial.println("Error sending message");
      }
    }
  }
  delay(100);
}