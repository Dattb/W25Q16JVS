/*
 * W25Q16JV Full Feature Demo
 * 
 * Chương trình demo đầy đủ tất cả tính năng của thư viện
 * Bao gồm menu tương tác qua Serial Monitor
 */

#include <W25Q16JV.h>

#define CS_PIN 10

W25Q16JV flash(CS_PIN);

void printMenu() {
  Serial.println("\n========================================");
  Serial.println("     W25Q16JV Flash Control Menu");
  Serial.println("========================================");
  Serial.println("1. Read Chip Information");
  Serial.println("2. Read Status Registers");
  Serial.println("3. Read Byte/Data");
  Serial.println("4. Write Byte (Read-Modify-Write)");
  Serial.println("5. Write Page");
  Serial.println("6. Erase Sector");
  Serial.println("7. Erase Block (64KB)");
  Serial.println("8. Hex Dump Memory");
  Serial.println("9. Fill Pattern");
  Serial.println("0. Power Down/Wake Up");
  Serial.println("h. Show this menu");
  Serial.println("========================================");
  Serial.print("> ");
}

void readChipInfo() {
  Serial.println("\n=== Chip Information ===");
  
  uint16_t deviceID = flash.readDeviceID();
  Serial.print("Device ID: 0x");
  Serial.println(deviceID, HEX);
  
  uint32_t jedecID = flash.readJEDECID();
  Serial.print("JEDEC ID: 0x");
  Serial.println(jedecID, HEX);
  
  uint8_t mfg = (jedecID >> 16) & 0xFF;
  uint8_t type = (jedecID >> 8) & 0xFF;
  uint8_t cap = jedecID & 0xFF;
  
  Serial.print("  Manufacturer: 0x");
  Serial.print(mfg, HEX);
  Serial.println(" (Winbond)");
  
  Serial.print("  Memory Type: 0x");
  Serial.println(type, HEX);
  
  Serial.print("  Capacity: 0x");
  Serial.print(cap, HEX);
  Serial.println(" (16Mbit / 2MB)");
  
  uint64_t uniqueID = flash.readUniqueID();
  Serial.print("Unique ID: 0x");
  Serial.print((uint32_t)(uniqueID >> 32), HEX);
  Serial.println((uint32_t)(uniqueID & 0xFFFFFFFF), HEX);
  
  Serial.print("Total Capacity: ");
  Serial.print(flash.getCapacity());
  Serial.println(" bytes");
}

void readStatusRegisters() {
  Serial.println("\n=== Status Registers ===");
  
  uint8_t sr1 = flash.readStatusReg1();
  uint8_t sr2 = flash.readStatusReg2();
  uint8_t sr3 = flash.readStatusReg3();
  
  Serial.print("SR1: 0x");
  if (sr1 < 0x10) Serial.print("0");
  Serial.print(sr1, HEX);
  Serial.print(" - BUSY: ");
  Serial.print((sr1 & 0x01) ? "1" : "0");
  Serial.print(", WEL: ");
  Serial.println((sr1 & 0x02) ? "1" : "0");
  
  Serial.print("SR2: 0x");
  if (sr2 < 0x10) Serial.print("0");
  Serial.println(sr2, HEX);
  
  Serial.print("SR3: 0x");
  if (sr3 < 0x10) Serial.print("0");
  Serial.println(sr3, HEX);
}

void readMemory() {
  Serial.println("\n=== Read Memory ===");
  Serial.print("Enter address (hex, e.g., 1000): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("Enter length (bytes): ");
  while (!Serial.available());
  uint16_t len = Serial.parseInt();
  
  if (len > 256) len = 256;
  
  uint8_t buffer[256];
  flash.readData(addr, buffer, len);
  
  Serial.println("\nData:");
  for (uint16_t i = 0; i < len; i++) {
    if (i % 16 == 0) {
      Serial.print("\n0x");
      if (addr + i < 0x100000) Serial.print("0");
      if (addr + i < 0x10000) Serial.print("0");
      if (addr + i < 0x1000) Serial.print("0");
      if (addr + i < 0x100) Serial.print("0");
      if (addr + i < 0x10) Serial.print("0");
      Serial.print(addr + i, HEX);
      Serial.print(": ");
    }
    if (buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void writeByte() {
  Serial.println("\n=== Write Byte (Read-Modify-Write) ===");
  Serial.print("Enter address (hex): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("Enter byte value (hex): 0x");
  while (!Serial.available());
  String valStr = Serial.readStringUntil('\n');
  uint8_t value = strtol(valStr.c_str(), NULL, 16);
  
  Serial.print("\nWriting 0x");
  Serial.print(value, HEX);
  Serial.print(" to address 0x");
  Serial.println(addr, HEX);
  
  uint32_t start = millis();
  int result = flash.writeByte(addr, value);
  uint32_t elapsed = millis() - start;
  
  if (result == W25Q_OK) {
    Serial.print("✓ Write successful in ");
    Serial.print(elapsed);
    Serial.println(" ms");
    
    uint8_t verify = flash.readByte(addr);
    Serial.print("Verify: 0x");
    Serial.println(verify, HEX);
  } else {
    Serial.print("✗ Write failed with error: ");
    Serial.println(result);
  }
}

void writePage() {
  Serial.println("\n=== Write Page ===");
  Serial.print("Enter address (hex): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("Enter fill pattern (hex): 0x");
  while (!Serial.available());
  String valStr = Serial.readStringUntil('\n');
  uint8_t pattern = strtol(valStr.c_str(), NULL, 16);
  
  uint8_t buffer[256];
  for (int i = 0; i < 256; i++) {
    buffer[i] = pattern;
  }
  
  Serial.println("\nWriting page...");
  flash.writePage(addr, buffer, 256);
  Serial.println("✓ Page written");
}

void eraseSector() {
  Serial.println("\n=== Erase Sector (4KB) ===");
  Serial.print("Enter sector address (hex): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("⚠ This will erase 4KB at 0x");
  Serial.println(addr & 0xFFFFF000, HEX);
  Serial.print("Continue? (y/n): ");
  
  while (!Serial.available());
  char confirm = Serial.read();
  
  if (confirm == 'y' || confirm == 'Y') {
    Serial.println("\nErasing...");
    uint32_t start = millis();
    flash.eraseSector(addr);
    uint32_t elapsed = millis() - start;
    
    Serial.print("✓ Sector erased in ");
    Serial.print(elapsed);
    Serial.println(" ms");
  } else {
    Serial.println("Cancelled");
  }
}

void eraseBlock() {
  Serial.println("\n=== Erase Block (64KB) ===");
  Serial.print("Enter block address (hex): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("⚠ This will erase 64KB at 0x");
  Serial.println(addr & 0xFFFF0000, HEX);
  Serial.print("Continue? (y/n): ");
  
  while (!Serial.available());
  char confirm = Serial.read();
  
  if (confirm == 'y' || confirm == 'Y') {
    Serial.println("\nErasing...");
    uint32_t start = millis();
    flash.eraseBlock64K(addr);
    uint32_t elapsed = millis() - start;
    
    Serial.print("✓ Block erased in ");
    Serial.print(elapsed);
    Serial.println(" ms");
  } else {
    Serial.println("Cancelled");
  }
}

void hexDump() {
  Serial.println("\n=== Hex Dump ===");
  Serial.print("Enter start address (hex): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("Enter number of lines (16 bytes each): ");
  while (!Serial.available());
  uint16_t lines = Serial.parseInt();
  
  if (lines > 32) lines = 32;
  
  uint8_t buffer[16];
  Serial.println();
  
  for (uint16_t line = 0; line < lines; line++) {
    uint32_t line_addr = addr + (line * 16);
    flash.readData(line_addr, buffer, 16);
    
    Serial.print("0x");
    if (line_addr < 0x100000) Serial.print("0");
    if (line_addr < 0x10000) Serial.print("0");
    if (line_addr < 0x1000) Serial.print("0");
    if (line_addr < 0x100) Serial.print("0");
    if (line_addr < 0x10) Serial.print("0");
    Serial.print(line_addr, HEX);
    Serial.print(": ");
    
    for (int i = 0; i < 16; i++) {
      if (buffer[i] < 0x10) Serial.print("0");
      Serial.print(buffer[i], HEX);
      Serial.print(" ");
    }
    
    Serial.print(" | ");
    for (int i = 0; i < 16; i++) {
      char c = buffer[i];
      Serial.print((c >= 32 && c <= 126) ? c : '.');
    }
    Serial.println();
  }
}

void fillPattern() {
  Serial.println("\n=== Fill Pattern ===");
  Serial.print("Enter start address (hex): 0x");
  
  while (!Serial.available());
  String addrStr = Serial.readStringUntil('\n');
  uint32_t addr = strtol(addrStr.c_str(), NULL, 16);
  
  Serial.print("Enter length (bytes): ");
  while (!Serial.available());
  uint16_t len = Serial.parseInt();
  
  Serial.print("Enter pattern (0=incremental, 1=0x55, 2=0xAA): ");
  while (!Serial.available());
  uint8_t pattern_type = Serial.parseInt();
  
  uint8_t buffer[256];
  Serial.println("\nFilling...");
  
  uint16_t written = 0;
  while (written < len) {
    uint16_t chunk = min((uint16_t)(len - written), (uint16_t)256);
    
    for (uint16_t i = 0; i < chunk; i++) {
      if (pattern_type == 0) {
        buffer[i] = (written + i) & 0xFF;
      } else if (pattern_type == 1) {
        buffer[i] = 0x55;
      } else {
        buffer[i] = 0xAA;
      }
    }
    
    flash.writeData(addr + written, buffer, chunk);
    written += chunk;
    
    Serial.print(".");
  }
  
  Serial.println("\n✓ Pattern filled");
}

void powerControl() {
  Serial.println("\n=== Power Control ===");
  Serial.println("1. Power Down");
  Serial.println("2. Wake Up");
  Serial.print("Select: ");
  
  while (!Serial.available());
  int choice = Serial.parseInt();
  
  if (choice == 1) {
    flash.powerDown();
    Serial.println("✓ Chip in power-down mode");
  } else if (choice == 2) {
    flash.releasePowerDown();
    delay(3);
    Serial.println("✓ Chip awake");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("    W25Q16JV Flash Full Demo");
  Serial.println("========================================\n");
  
  Serial.println("Initializing...");
  if (flash.begin()) {
    Serial.println("✓ W25Q16JV initialized successfully!");
    readChipInfo();
  } else {
    Serial.println("✗ Initialization failed!");
    Serial.println("Check wiring and reset.");
    while (1) delay(1000);
  }
  
  printMenu();
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    // Clear remaining newline
    while (Serial.available() && Serial.peek() == '\n') {
      Serial.read();
    }
    
    switch (cmd) {
      case '1': readChipInfo(); break;
      case '2': readStatusRegisters(); break;
      case '3': readMemory(); break;
      case '4': writeByte(); break;
      case '5': writePage(); break;
      case '6': eraseSector(); break;
      case '7': eraseBlock(); break;
      case '8': hexDump(); break;
      case '9': fillPattern(); break;
      case '0': powerControl(); break;
      case 'h':
      case 'H': printMenu(); break;
      case '\n':
      case '\r': break;
      default:
        Serial.println("Invalid command. Press 'h' for menu.");
        break;
    }
    
    if (cmd != '\n' && cmd != '\r' && cmd != 'h' && cmd != 'H') {
      Serial.print("\n> ");
    }
  }
}
