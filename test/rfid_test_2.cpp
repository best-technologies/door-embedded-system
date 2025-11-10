#include "pins_arduino.h"
#include <iterator>
#include "esp32-hal-spi.h"
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 15
#define RST_PIN 25
#define MOSI_PIN 23
#define MISO_PIN 19
#define CLK_PIN 18

MFRC522 mfrc522(SS_PIN, RST_PIN);
// MFRC522::MIFARE_Key key;
// byte nuidPICC[4];
// String tidString = "NIL";


void setup() {
	Serial.begin(115200);		// Initialize serial communications with the PC
	// pinMode(SS_PIN, OUTPUT);
	// digitalWrite(SS_PIN, HIGH);
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);				// Optional delay. Some board do need more time after init to be ready, see Readme
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() {
	// Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

	// Dump debug info about the card; PICC_HaltA() is automatically called
	mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}