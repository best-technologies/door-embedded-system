#include <SPI.h>
#include <MFRC522.h>

// RFID Reader pins
#define SS_PIN 15     // Slave Select
#define RST_PIN 21    // Reset

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   RFID MODULE DIAGNOSTIC TEST          ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  // Initialize SPI
  SPI.begin();
  Serial.println("✓ SPI initialized");

  // Initialize RFID module
  rfid.PCD_Init();
  Serial.println("✓ RFID PCD_Init() called");

  delay(500);

  // Check firmware version
  byte ver = rfid.ReadReg(0x37);
  Serial.print("Firmware Version: 0x");
  Serial.println(ver, HEX);

  // Enable antenna
  rfid.PCD_AntennaOn();
  Serial.println("✓ RFID antenna enabled");

  // Get antenna gain
  byte gain = rfid.GetAntennaGain();
  Serial.print("Antenna Gain: ");
  Serial.println(gain);

  // Check if module is responding
  if (ver == 0 || ver == 0xFF) {
    Serial.println("\n⚠ WARNING: RFID module not responding!");
    Serial.println("Check connections:");
    Serial.println("  - VCC (3.3V)");
    Serial.println("  - GND");
    Serial.println("  - MOSI (GPIO 23)");
    Serial.println("  - MISO (GPIO 19)");
    Serial.println("  - SCK (GPIO 18)");
    Serial.println("  - SS (GPIO 15)");
    Serial.println("  - RST (GPIO 21)");
  } else {
    Serial.println("✓ RFID module detected!");
  }

  Serial.println();
  Serial.println("═══════════════════════════════════════");
  Serial.println("Waiting for RFID cards...");
  Serial.println("═══════════════════════════════════════");
  Serial.println();
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) {
    delay(100);
    return;
  }

  // Try to read the UID
  if (!rfid.PICC_ReadCardSerial()) {
    Serial.println("Failed to read card serial");
    delay(100);
    return;
  }

  // Card detected!
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║     CARD DETECTED!                     ║");
  Serial.println("╚════════════════════════════════════════╝");

  // Print card UID
  Serial.print("Card UID: ");
  String cardID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");

    // Build card ID string
    cardID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    cardID += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
  Serial.println("Card ID (no spaces): " + cardID);
  Serial.println("Card Type: " + String(rfid.uid.sak, HEX));
  Serial.println("UID Size: " + String(rfid.uid.size) + " bytes");

  // Halt the card
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  Serial.println("Card halted. Ready for next card.");
  Serial.println();

  delay(500);
}
