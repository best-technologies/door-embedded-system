#include <esp_now.h>
#include <WiFi.h>

// Replace with your master ESP32 MAC Address
uint8_t masterMAC[] = {0x14, 0x2B, 0x2F, 0xC5, 0x85, 0x58}; // Will be updated when receiving

// Message structure (must match master)
typedef struct {
  int messageId;
  char text[32];
  bool status;
} TestMessage;

int receivedCount = 0;

// Callback when data is received
void onDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  receivedCount++;
  
  TestMessage msg;
  memcpy(&msg, incomingData, sizeof(msg));
  
  Serial.println("\n=== MESSAGE RECEIVED ===");
  Serial.print("From MAC: ");
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", 
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("Message ID: ");
  Serial.println(msg.messageId);
  Serial.print("Text: ");
  Serial.println(msg.text);
  Serial.print("Status: ");
  Serial.println(msg.status ? "TRUE" : "FALSE");
  Serial.print("Total Received: ");
  Serial.println(receivedCount);
  Serial.println("========================\n");
  
  // Save master MAC on first message
  if (receivedCount == 1) {
    memcpy(masterMAC, mac, 6);
    
    // Add master as peer for bidirectional communication
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, masterMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;
    
    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
      Serial.println("✓ Master added as peer for replies");
    }
  }
  
  // Send acknowledgment back to master
  TestMessage ackMsg;
  ackMsg.messageId = msg.messageId;
  sprintf(ackMsg.text, "ACK from Slave #%d", msg.messageId);
  ackMsg.status = true;
  
  Serial.println(">>> SENDING ACK TO MASTER <<<");
  esp_err_t result = esp_now_send(masterMAC, (uint8_t*)&ackMsg, sizeof(ackMsg));
  
  if (result == ESP_OK) {
    Serial.println("✓ ACK sent successfully");
  } else {
    Serial.println("✗ ACK send failed");
  }
}

// Callback when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("ACK Delivery Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "SUCCESS ✓" : "FAILED ✗");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("    ESP-NOW SLAVE TEST");
  Serial.println("========================================");
  
  // Set device as WiFi Station
  WiFi.mode(WIFI_STA);
  
  Serial.print("Slave MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("\n IMPORTANT: Update master code with this MAC address!");
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed!");
    return;
  }
  
  Serial.println("✓ ESP-NOW initialized");
  
  // Register callbacks
  esp_now_register_recv_cb(onDataReceive);
  esp_now_register_send_cb(onDataSent);
  
  Serial.println("✓ Callbacks registered");
  Serial.println("\n✓ Ready to receive messages");
  Serial.println("Waiting for master...\n");
}

void loop() {
  // Slave just waits for messages
  // All work is done in the callback
  delay(100);
}
