#include <SPI.h>
#include <MFRC522.h>

#define RFID_SS 15
#define RFID_RST 32
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18

MFRC522 mfrc522(RFID_SS);  // Create MFRC522 instance

/**
 * Check firmware only once at startup
 */
void setup() {
  Serial.begin(115200);   // Initialize serial communications with the PC
  while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();          // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522 module
  
  Serial.println(F("*****************************"));
  Serial.println(F("MFRC522 Digital self test"));
  Serial.println(F("*****************************"));
  mfrc522.PCD_DumpVersionToSerial();  // Show version of PCD - MFRC522 Card Reader
  Serial.println(F("-----------------------------"));
  Serial.println(F("Only known versions supported"));
  Serial.println(F("-----------------------------"));
  Serial.println(F("Performing test..."));
  bool result = mfrc522.PCD_PerformSelfTest(); // perform the test
  Serial.println(F("-----------------------------"));
  Serial.print(F("Result: "));
  if (result)
    Serial.println(F("OK"));
  else
    Serial.println(F("DEFECT or UNKNOWN"));
    Serial.println();
}

void loop() {} // nothing to do