void testESPNowConnection() {
  Serial.println("\n[ESP-NOW] Testing connection to controller...");
  
  UnlockMessage testMsg;
  testMsg.unlock = false;
  testMsg.duration = 0;
  testMsg.soundBuzzer = false;
  strcpy(testMsg.reason, "TEST");
  
  esp_err_t result = esp_now_send(controllerMAC, (uint8_t*)&testMsg, sizeof(testMsg));
  
  if (result == ESP_OK) {
    Serial.println("[ESP-NOW] Test message sent successfully");
  } else {
    Serial.print("[ESP-NOW] Test message failed, error: ");
    Serial.println(result);
  }
}