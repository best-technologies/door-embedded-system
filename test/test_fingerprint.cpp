#include <Adafruit_Fingerprint.h>

HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

void setup() {
  Serial.begin(115200);
  fingerSerial.begin(57600, SERIAL_8N1, 16, 17);
  
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor not found");
    while (1) delay(1);
  }
  
  finger.getParameters();
  Serial.print("Status: 0x"); Serial.println(finger.status_reg, HEX);
  Serial.print("Sys ID: 0x"); Serial.println(finger.system_id, HEX);
  Serial.print("Capacity: "); Serial.println(finger.capacity);
  Serial.print("Security level: "); Serial.println(finger.security_level);
  Serial.print("Device address: "); Serial.println(finger.device_addr, HEX);
  Serial.print("Packet len: "); Serial.println(finger.packet_len);
  Serial.print("Baud rate: "); Serial.println(finger.baud_rate);
  
  Serial.println("Place finger to test detection...");
}

void loop() {
  uint8_t p = finger.getImage();
  
  if (p == FINGERPRINT_OK) {
    Serial.println("Image taken");
    
    p = finger.image2Tz();
    if (p == FINGERPRINT_OK) {
      Serial.println("Image converted");
      
      p = finger.fingerFastSearch();
      if (p == FINGERPRINT_OK) {
        Serial.print("Found ID #"); Serial.print(finger.fingerID);
        Serial.print(" with confidence of "); Serial.println(finger.confidence);
      } else if (p == FINGERPRINT_NOTFOUND) {
        Serial.println("Did not find a match");
      } else {
        Serial.println("Unknown error");
      }
    }
  } else if (p != FINGERPRINT_NOFINGER) {
    Serial.print("Error: "); Serial.println(p);
  }
  
  delay(50);
}