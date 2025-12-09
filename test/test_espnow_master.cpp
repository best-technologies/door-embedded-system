#include <esp_now.h>
#include <WiFi.h>

// Replace with your slave ESP32 MAC Address
uint8_t slaveMAC[] = {0x34, 0x5F, 0x45, 0xA8, 0x08, 0x28};

// Message structure
typedef struct {
  int messageId;
  char text[32];
  bool status;
} TestMessage;

int messageCounter = 0;

// Callback when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println("\n=== SEND STATUS ===");
  Serial.print("Message ID: ");
  Serial.println(messageCounter);
  Serial.print("Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS ✓" : "FAILED ✗");
  Serial.println("==================\n");
}

// Callback when data is received
void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  TestMessage msg;
  memcpy(&msg, incomingData, sizeof(msg));
  
  Serial.println("\n=== RECEIVED FROM SLAVE ===");
  Serial.print("Message ID: ");
  Serial.println(msg.messageId);
  Serial.print("Text: ");
  Serial.println(msg.text);
  Serial.print("Status: ");
  Serial.println(msg.status ? "TRUE" : "FALSE");
  Serial.println("===========================\n");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("    ESP-NOW MASTER TEST");
  Serial.println("========================================");
  
  // Set device as WiFi Station
  WiFi.mode(WIFI_STA);
  
  Serial.print("Master MAC Address: ");
  Serial.println(WiFi.macAddress());
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed!");
    return;
  }
  
  Serial.println("✓ ESP-NOW initialized");
  
  // Register callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceive);
  
  // Add slave as peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, slaveMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("ERROR: Failed to add peer!");
    return;
  }
  
  Serial.println("✓ Slave added as peer");
  Serial.print("Slave MAC: ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", 
                slaveMAC[0], slaveMAC[1], slaveMAC[2], 
                slaveMAC[3], slaveMAC[4], slaveMAC[5]);
  Serial.println("\n✓ Ready to send messages");
  Serial.println("Sending message every 3 seconds...\n");
}

void loop() {
  messageCounter++;
  
  // Prepare message
  TestMessage msg;
  msg.messageId = messageCounter;
  sprintf(msg.text, "Hello from Master #%d", messageCounter);
  msg.status = (messageCounter % 2 == 0);
  
  // Send message
  Serial.println(">>> SENDING MESSAGE <<<");
  Serial.print("Message ID: ");
  Serial.println(msg.messageId);
  Serial.print("Text: ");
  Serial.println(msg.text);
  Serial.print("Status: ");
  Serial.println(msg.status ? "TRUE" : "FALSE");
  
  esp_err_t result = esp_now_send(slaveMAC, (uint8_t*)&msg, sizeof(msg));
  
  if (result != ESP_OK) {
    Serial.println("ERROR: Send failed!");
  }
  
  delay(3000); // Send every 3 seconds
}
