#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS 15
#define RFID_RST 32

MFRC522 rfid(RFID_SS, RFID_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("RFID Debug Test");
  Serial.println("Pin Configuration:");
  Serial.println("SS (SDA) -> GPIO 15");
  Serial.println("RST -> GPIO 32");
  Serial.println("MOSI -> GPIO 23");
  Serial.println("MISO -> GPIO 19");
  Serial.println("SCK -> GPIO 18");
  Serial.println("3.3V -> 3.3V");
  Serial.println("GND -> GND");
  Serial.println();
  
  SPI.begin();
  rfid.PCD_Init();
  
  // Test communication
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.print("Firmware Version: 0x");
  Serial.print(version, HEX);
  
  if (version == 0x91 || version == 0x92) {
    Serial.println(" = MFRC522 v1.0/v2.0");
    Serial.println("RFID module detected successfully!");
  } else if (version == 0x00 || version == 0xFF) {
    Serial.println(" = Communication failure");
    Serial.println("Check wiring and power supply");
  } else {
    Serial.print(" = Unknown version");
  }
  
  Serial.println("\nPlace RFID card near reader...");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print("Card UID: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();
    rfid.PICC_HaltA();
    delay(1000);
  }
  delay(100);
}