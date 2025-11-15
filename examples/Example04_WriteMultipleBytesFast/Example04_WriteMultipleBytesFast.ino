/*
 * Example 4: Ghi Nhiều Byte Nhanh
 * 
 * Ví dụ này hướng dẫn cách ghi nhiều byte liên tiếp với tốc độ nhanh
 * ⚠️ Cần xóa sector trước khi ghi
 * ⚡ Tự động xử lý page boundary
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

void printData(const char* title, uint32_t addr, uint8_t* data, uint16_t len) {
  Serial.printf("\n%s:\n", title);
  for (uint16_t i = 0; i < len; i++) {
    if (i % 16 == 0) {
      Serial.printf("0x%06X: ", addr + i);
    }
    Serial.printf("%02X ", data[i]);
    if ((i + 1) % 16 == 0) {
      Serial.println();
    }
  }
  if (len % 16 != 0) Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n=== VÍ DỤ: GHI NHIỀU BYTE NHANH ===\n");
  
  // Khởi tạo SPI
  Serial.println("1. Khởi tạo SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("   ✓ SPI đã sẵn sàng\n");
  
  // Khởi tạo flash
  Serial.println("2. Khởi tạo W25Q16JV...");
  if (!flash.begin()) {
    Serial.println("   ✗ Không thể khởi tạo flash!");
    while (1) delay(1000);
  }
  Serial.println("   ✓ Flash đã sẵn sàng\n");
  
  // Chuẩn bị dữ liệu test
  uint32_t addr = 0x2000;
  uint8_t writeData[256];
  uint8_t readData[256];
  
  // Tạo pattern test
  Serial.println("3. Tạo dữ liệu test (256 bytes)...");
  for (int i = 0; i < 256; i++) {
    writeData[i] = i;  // 0x00, 0x01, 0x02, ... 0xFF
  }
  Serial.println("   ✓ Dữ liệu: 0x00 → 0xFF\n");
  
  // Bước 1: Xóa sector
  Serial.println("4. Xóa sector trước khi ghi...");
  Serial.printf("   Địa chỉ sector: 0x%06X\n", addr);
  uint32_t start = millis();
  flash.eraseSector(addr);
  uint32_t eraseTime = millis() - start;
  Serial.printf("   ✓ Xóa xong! (Mất %u ms)\n\n", eraseTime);
  delay(100);  // Chờ xóa hoàn tất
  
  // Bước 2: Ghi dữ liệu
  Serial.println("5. Ghi 256 bytes...");
  start = millis();
  flash.writeData(addr, writeData, 256);
  uint32_t writeTime = millis() - start;
  Serial.printf("   ✓ Ghi xong! (Mất %u ms)\n", writeTime);
  delay(10);  // Chờ ghi hoàn tất
  
  // Bước 3: Đọc lại để verify
  Serial.println("\n6. Đọc lại để verify...");
  start = millis();
  flash.readData(addr, readData, 256);
  uint32_t readTime = millis() - start;
  Serial.printf("   ✓ Đọc xong! (Mất %u ms)\n", readTime);
  
  // Bước 4: So sánh dữ liệu
  Serial.println("\n7. Verify dữ liệu...");
  bool ok = true;
  int errorCount = 0;
  
  for (int i = 0; i < 256; i++) {
    if (readData[i] != writeData[i]) {
      if (errorCount < 10) {  // Chỉ in 10 lỗi đầu tiên
        Serial.printf("   ✗ Lỗi tại offset %d: ghi=0x%02X, đọc=0x%02X\n", 
                     i, writeData[i], readData[i]);
      }
      errorCount++;
      ok = false;
    }
  }
  
  if (ok) {
    Serial.println("   ✓ Tất cả 256 bytes đều chính xác!\n");
  } else {
    Serial.printf("   ✗ Có %d lỗi!\n\n", errorCount);
  }
  
  // Hiển thị 32 bytes đầu tiên
  printData("📄 32 bytes đầu tiên", addr, readData, 32);
  
  // Thống kê
  Serial.println("\n📊 THỐNG KÊ:");
  Serial.printf("   Xóa sector:  %u ms\n", eraseTime);
  Serial.printf("   Ghi 256 B:   %u ms (%.2f KB/s)\n", 
               writeTime, (256.0 / writeTime));
  Serial.printf("   Đọc 256 B:   %u ms (%.2f KB/s)\n", 
               readTime, (256.0 / readTime));
  Serial.printf("   Tổng thời gian: %u ms\n", eraseTime + writeTime + readTime);
  
  // Test ghi dữ liệu dài (cross-page boundary)
  Serial.println("\n--- TEST GHI DỮ LIỆU DÀI (1000 bytes) ---");
  uint8_t longData[1000];
  for (int i = 0; i < 1000; i++) {
    longData[i] = (i % 256);
  }
  
  Serial.println("Xóa sector...");
  flash.eraseSector(0x3000);
  delay(100);
  
  Serial.println("Ghi 1000 bytes (tự động chia 4 pages)...");
  start = millis();
  flash.writeData(0x3000, longData, 1000);
  uint32_t longWriteTime = millis() - start;
  Serial.printf("✓ Ghi xong! (Mất %u ms)\n", longWriteTime);
  
  Serial.println("\n✅ Hoàn tất!");
  Serial.println("\n📝 LƯU Ý:");
  Serial.println("   - Phải xóa sector TRƯỚC khi ghi");
  Serial.println("   - writeData() tự động xử lý page boundary");
  Serial.println("   - Delay 5-10ms sau khi ghi để đảm bảo hoàn tất");
}

void loop() {
  delay(1000);
}
