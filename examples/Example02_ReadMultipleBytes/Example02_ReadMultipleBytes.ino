/*
 * Example 2: Đọc Nhiều Byte
 * 
 * Ví dụ này hướng dẫn cách đọc nhiều byte liên tiếp và hiển thị dạng hex dump
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

void printHexDump(uint32_t addr, uint8_t* data, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    // In địa chỉ ở đầu mỗi dòng (16 bytes/dòng)
    if (i % 16 == 0) {
      Serial.printf("\n0x%06X: ", addr + i);
    }
    
    // In giá trị hex
    Serial.printf("%02X ", data[i]);
    
    // In ASCII ở cuối dòng
    if ((i + 1) % 16 == 0 || i == len - 1) {
      // Padding nếu dòng cuối không đủ 16 bytes
      int padding = (i % 16 == 15) ? 0 : (15 - (i % 16));
      for (int p = 0; p < padding; p++) {
        Serial.print("   ");
      }
      
      Serial.print(" | ");
      
      // In ASCII
      int start = (i / 16) * 16;
      int end = i + 1;
      for (int j = start; j < end; j++) {
        char c = (data[j] >= 32 && data[j] <= 126) ? data[j] : '.';
        Serial.print(c);
      }
    }
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n=== VÍ DỤ: ĐỌC NHIỀU BYTE ===\n");
  
  // Khởi tạo SPI
  Serial.println("Khởi tạo SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("✓ SPI đã sẵn sàng\n");
  
  // Khởi tạo flash
  Serial.println("Khởi tạo W25Q16JV...");
  if (!flash.begin()) {
    Serial.println("✗ Không thể khởi tạo flash!");
    while (1) delay(1000);
  }
  Serial.println("✓ Flash đã sẵn sàng\n");
  
  // Đọc 256 bytes từ địa chỉ 0x1000
  uint32_t addr = 0x1000;
  uint16_t len = 256;
  uint8_t buffer[256];
  
  Serial.printf("Đọc %d bytes từ địa chỉ 0x%06X...\n", len, addr);
  
  // Đo thời gian đọc
  uint32_t start = micros();
  flash.readData(addr, buffer, len);
  uint32_t elapsed = micros() - start;
  
  // Hiển thị hex dump
  printHexDump(addr, buffer, len);
  
  Serial.printf("\n✓ Đọc hoàn tất!");
  Serial.printf("\n⏱️  Thời gian: %u μs (%.2f ms)\n", elapsed, elapsed / 1000.0);
  Serial.printf("📊 Tốc độ: %.2f KB/s\n", (len * 1000000.0 / elapsed) / 1024.0);
  
  // So sánh với Fast Read
  Serial.println("\n--- So sánh với Fast Read ---");
  start = micros();
  flash.fastRead(addr, buffer, len);
  elapsed = micros() - start;
  
  Serial.printf("⏱️  Fast Read: %u μs (%.2f ms)\n", elapsed, elapsed / 1000.0);
  Serial.printf("📊 Tốc độ: %.2f KB/s\n", (len * 1000000.0 / elapsed) / 1024.0);
}

void loop() {
  delay(1000);
}
