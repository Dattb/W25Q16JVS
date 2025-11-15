/*
 * Example 6: Thông Tin Chip
 * 
 * Ví dụ này hướng dẫn cách đọc thông tin chi tiết về chip flash:
 * - Device ID
 * - JEDEC ID (Manufacturer, Memory Type, Capacity)
 * - Unique ID (64-bit)
 * - Status Registers
 * - Dung lượng
 */

#include <Arduino.h>
#include <SPI.h>
#include <W25Q16JV.h>

// Định nghĩa chân SPI cho ESP32C3
#define SCK_PIN  2
#define MISO_PIN 7
#define MOSI_PIN 6
#define CS_PIN   10

W25Q16JV flash(CS_PIN);

void printBinary(uint8_t value) {
  for (int i = 7; i >= 0; i--) {
    Serial.print((value >> i) & 1);
    if (i == 4) Serial.print(" ");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   THÔNG TIN CHI TIẾT CHIP W25Q16JV    ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Khởi tạo SPI
  Serial.println("Khởi tạo SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("✓ SPI đã sẵn sàng\n");
  
  // Khởi tạo flash
  Serial.println("Khởi tạo W25Q16JV...");
  if (!flash.begin()) {
    Serial.println("✗ Không thể khởi tạo flash!");
    Serial.println("\nKiểm tra:");
    Serial.println("  - Nguồn 3.3V");
    Serial.println("  - Đường GND");
    Serial.println("  - HOLD# và WP# nối 3.3V");
    Serial.println("  - Các chân SPI đúng");
    while (1) delay(1000);
  }
  Serial.println("✓ Flash đã sẵn sàng\n");
  
  delay(100);
  
  // ========== DEVICE ID ==========
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("📇 DEVICE IDENTIFICATION");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint16_t deviceID = flash.readDeviceID();
  Serial.printf("Device ID:        0x%04X\n", deviceID);
  Serial.printf("  Manufacturer:   0x%02X ", (deviceID >> 8) & 0xFF);
  if (((deviceID >> 8) & 0xFF) == 0xEF) {
    Serial.println("(Winbond Electronics)");
  } else {
    Serial.println("(Unknown)");
  }
  Serial.printf("  Device:         0x%02X ", deviceID & 0xFF);
  if ((deviceID & 0xFF) == 0x14) {
    Serial.println("(W25Q16)");
  } else {
    Serial.println("(Unknown)");
  }
  
  // ========== JEDEC ID ==========
  Serial.println();
  uint32_t jedecID = flash.readJEDECID();
  Serial.printf("JEDEC ID:         0x%06X\n", jedecID);
  
  uint8_t mfg = (jedecID >> 16) & 0xFF;
  uint8_t type = (jedecID >> 8) & 0xFF;
  uint8_t cap = jedecID & 0xFF;
  
  Serial.printf("  Manufacturer:   0x%02X ", mfg);
  if (mfg == 0xEF) {
    Serial.println("(Winbond)");
  } else {
    Serial.println("(Unknown)");
  }
  
  Serial.printf("  Memory Type:    0x%02X ", type);
  if (type == 0x40) {
    Serial.println("(SPI Flash)");
  } else {
    Serial.println("(Unknown)");
  }
  
  Serial.printf("  Capacity:       0x%02X ", cap);
  if (cap == 0x15) {
    Serial.println("(16Mbit / 2MB)");
  } else {
    Serial.println("(Unknown)");
  }
  
  // ========== UNIQUE ID ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("🔑 UNIQUE IDENTIFIER");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint64_t uniqueID = flash.readUniqueID();
  Serial.printf("Unique ID (64-bit): 0x%08X%08X\n", 
               (uint32_t)(uniqueID >> 32),
               (uint32_t)(uniqueID & 0xFFFFFFFF));
  Serial.println("ℹ️  Mỗi chip có một ID duy nhất, không trùng lặp");
  
  // ========== CAPACITY ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("💾 MEMORY CAPACITY");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint32_t capacity = flash.getCapacity();
  Serial.printf("Total Capacity:   %u bytes\n", capacity);
  Serial.printf("                  %u KB\n", capacity / 1024);
  Serial.printf("                  %.2f MB\n", capacity / 1048576.0);
  Serial.printf("                  %u Mbit\n", (capacity * 8) / 1048576);
  
  Serial.println("\nMemory Organization:");
  Serial.printf("  Page size:      256 bytes\n");
  Serial.printf("  Total pages:    %u pages\n", capacity / 256);
  Serial.printf("  Sector size:    4 KB (4096 bytes)\n");
  Serial.printf("  Total sectors:  %u sectors\n", capacity / 4096);
  Serial.printf("  Block 32K size: 32 KB\n");
  Serial.printf("  Total 32K:      %u blocks\n", capacity / 32768);
  Serial.printf("  Block 64K size: 64 KB\n");
  Serial.printf("  Total 64K:      %u blocks\n", capacity / 65536);
  
  // ========== STATUS REGISTERS ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("📊 STATUS REGISTERS");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint8_t sr1 = flash.readStatusReg1();
  Serial.printf("Status Register 1: 0x%02X (", sr1);
  printBinary(sr1);
  Serial.println(")");
  Serial.printf("  Bit 0 - BUSY:   %d ", (sr1 & 0x01) ? 1 : 0);
  Serial.println((sr1 & 0x01) ? "(Busy)" : "(Ready)");
  Serial.printf("  Bit 1 - WEL:    %d ", (sr1 & 0x02) ? 1 : 0);
  Serial.println((sr1 & 0x02) ? "(Write Enabled)" : "(Write Disabled)");
  Serial.printf("  Bit 5-2 - BP:   0x%X (Block Protection)\n", (sr1 >> 2) & 0x0F);
  Serial.printf("  Bit 6 - TB:     %d (Top/Bottom)\n", (sr1 >> 6) & 1);
  Serial.printf("  Bit 7 - SRP:    %d (Status Protect)\n", (sr1 >> 7) & 1);
  
  uint8_t sr2 = flash.readStatusReg2();
  Serial.printf("\nStatus Register 2: 0x%02X (", sr2);
  printBinary(sr2);
  Serial.println(")");
  Serial.printf("  Bit 0 - SRL:    %d (Status Reg Lock)\n", (sr2 & 0x01) ? 1 : 0);
  Serial.printf("  Bit 1 - QE:     %d (Quad Enable)\n", (sr2 >> 1) & 1);
  Serial.printf("  Bit 6 - CMP:    %d (Complement)\n", (sr2 >> 6) & 1);
  
  uint8_t sr3 = flash.readStatusReg3();
  Serial.printf("\nStatus Register 3: 0x%02X (", sr3);
  printBinary(sr3);
  Serial.println(")");
  Serial.printf("  Bit 5 - DRV0:   %d (Output Driver)\n", (sr3 >> 5) & 1);
  Serial.printf("  Bit 6 - DRV1:   %d (Output Driver)\n", (sr3 >> 6) & 1);
  
  // ========== CHIP STATUS ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("⚡ CHIP STATUS");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  bool busy = flash.isBusy();
  Serial.printf("Chip Status:      %s\n", busy ? "⏳ BUSY" : "✅ READY");
  Serial.printf("Write Enable:     %s\n", (sr1 & 0x02) ? "✅ Enabled" : "❌ Disabled");
  
  // Kiểm tra vài địa chỉ
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("🔍 MEMORY SAMPLE (First 32 bytes)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint8_t sample[32];
  flash.readData(0x000000, sample, 32);
  
  Serial.print("0x000000: ");
  for (int i = 0; i < 32; i++) {
    Serial.printf("%02X ", sample[i]);
    if (i == 15) Serial.print("\n0x000010: ");
  }
  Serial.println();
  
  // Đếm số byte 0xFF
  int ffCount = 0;
  for (int i = 0; i < 32; i++) {
    if (sample[i] == 0xFF) ffCount++;
  }
  
  if (ffCount == 32) {
    Serial.println("\nℹ️  Vùng này trống (chưa ghi dữ liệu hoặc đã xóa)");
  } else {
    Serial.printf("\nℹ️  Có %d/%d bytes đã được ghi dữ liệu\n", 32 - ffCount, 32);
  }
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║            ✅ HOÀN TẤT!               ║");
  Serial.println("╚════════════════════════════════════════╝");
}

void loop() {
  delay(1000);
}
