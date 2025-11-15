/*
 * Example 1: Đọc 1 Byte
 * 
 * Ví dụ này hướng dẫn cách đọc 1 byte từ địa chỉ bất kỳ
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

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n=== VÍ DỤ: ĐỌC 1 BYTE ===\n");
  
  // Khởi tạo SPI với chân cụ thể (QUAN TRỌNG cho ESP32C3!)
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
  
  // Đọc 1 byte tại địa chỉ 0x1000
  Serial.println("3. Đọc 1 byte tại địa chỉ 0x1000...");
  uint32_t addr = 0x1000;
  uint8_t value = flash.readByte(addr);
  
  Serial.printf("   Địa chỉ: 0x%06X\n", addr);
  Serial.printf("   Giá trị: 0x%02X", value);
  
  // Hiển thị dưới dạng binary và decimal
  Serial.printf(" (binary: ");
  for (int i = 7; i >= 0; i--) {
    Serial.print((value >> i) & 1);
  }
  Serial.printf(", decimal: %d)\n\n", value);
  
  // Đọc thêm vài byte liên tiếp
  Serial.println("4. Đọc thêm 10 byte liên tiếp:");
  for (int i = 0; i < 10; i++) {
    uint8_t val = flash.readByte(addr + i);
    Serial.printf("   [0x%06X] = 0x%02X\n", addr + i, val);
  }
  
  Serial.println("\n✓ Hoàn thành!");
  Serial.println("\n📝 Lưu ý:");
  Serial.println("   - Nếu flash chưa ghi dữ liệu, giá trị mặc định là 0xFF");
  Serial.println("   - Địa chỉ hợp lệ: 0x000000 đến 0x1FFFFF (2MB)");
}

void loop() {
  // Không làm gì trong loop
  delay(1000);
}
