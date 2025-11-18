#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 MAC Address Test");
  Serial.println("======================");
  
  // Get MAC address
  String macAddress = WiFi.macAddress();
  
  Serial.print("MAC Address: ");
  Serial.println(macAddress);
  
  // Convert to byte array format for ESP-NOW
  Serial.print("For ESP-NOW (controllerMAC[]): {");
  
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  for (int i = 0; i < 6; i++) {
    Serial.print("0x");
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(", ");
  }
  
  Serial.println("};");
  Serial.println("\nCopy the byte array above to update controllerMAC[] in door_system.cpp");
}

void loop() {
  // Nothing to do here
}