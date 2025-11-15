/*
 * W25Q16JV Flash Memory Demo
 * Simple example demonstrating basic read/write operations
 */

#include <Arduino.h>
#include <SPI.h>
#include <W25Q16JV.h>

// SPI Pin definitions for ESP32C3
#define SCK_PIN  2
#define MISO_PIN 7
#define MOSI_PIN 6
#define CS_PIN   10

W25Q16JV flash(CS_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  delay(500);
  
  Serial.println("\n=== W25Q16JV Flash Demo ===\n");
  
  // Initialize SPI with explicit pins (critical for ESP32C3!)
  Serial.println("Initializing SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("✓ SPI initialized\n");
  
  // Initialize flash
  if (!flash.begin()) {
    Serial.println("✗ Flash initialization failed!");
    while (1) delay(1000);
  }
  Serial.println("✓ Flash initialized\n");
  
  // Read device information
  uint16_t deviceID = flash.readDeviceID();
  uint32_t jedecID = flash.readJEDECID();
  
  Serial.printf("Device ID: 0x%04X\n", deviceID);
  Serial.printf("JEDEC ID: 0x%06X\n\n", jedecID);
  
  // Simple read/write test
  uint32_t testAddr = 0x1000;
  uint8_t writeData[4] = {0xAA, 0xBB, 0xCC, 0xDD};
  uint8_t readData[4] = {0};
  
  Serial.println("Erasing sector...");
  flash.eraseSector(testAddr);
  delay(100);
  
  Serial.println("Writing 4 bytes...");
  flash.writeData(testAddr, writeData, 4);
  delay(5);
  
  Serial.println("Reading 4 bytes...");
  flash.readData(testAddr, readData, 4);
  
  Serial.print("Data: ");
  for (int i = 0; i < 4; i++) {
    Serial.printf("0x%02X ", readData[i]);
  }
  Serial.println("\n\n✓ Test complete!");
}

void loop() {
  delay(1000);
}
