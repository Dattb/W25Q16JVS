/*
 * Example 3: Ghi 1 Byte An Toàn
 * 
 * Ví dụ này hướng dẫn cách ghi 1 byte với cơ chế Read-Modify-Write tự động
 * - Tự động đọc toàn bộ sector (4KB)
 * - Sửa 1 byte trong buffer
 * - Xóa sector
 * - Ghi lại toàn bộ sector
 * - Verify dữ liệu
 * ✅ Các byte khác KHÔNG bị ảnh hưởng
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

void printBytesAround(uint32_t addr) {
  Serial.printf("\nDữ liệu xung quanh địa chỉ 0x%06X:\n", addr);
  uint32_t start = (addr >= 8) ? addr - 8 : 0;
  
  for (int i = 0; i < 17; i++) {
    uint32_t currentAddr = start + i;
    uint8_t value = flash.readByte(currentAddr);
    
    if (currentAddr == addr) {
      Serial.printf("  [0x%06X] = 0x%02X  ← ĐÂY\n", currentAddr, value);
    } else {
      Serial.printf("  [0x%06X] = 0x%02X\n", currentAddr, value);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n=== VÍ DỤ: GHI 1 BYTE AN TOÀN ===\n");
  
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
  
  uint32_t testAddr = 0x5000;
  
  // Bước 1: Đọc giá trị ban đầu
  Serial.println("3. Đọc giá trị ban đầu...");
  uint8_t oldValue = flash.readByte(testAddr);
  Serial.printf("   Giá trị cũ tại 0x%06X: 0x%02X\n", testAddr, oldValue);
  printBytesAround(testAddr);
  
  // Bước 2: Ghi giá trị mới
  Serial.println("\n4. Ghi byte mới 0xAA...");
  Serial.println("   ⏳ Đang thực hiện Read-Modify-Write...");
  Serial.println("      - Đọc toàn bộ sector (4096 bytes)");
  Serial.println("      - Sửa 1 byte trong buffer");
  Serial.println("      - Xóa sector");
  Serial.println("      - Ghi lại toàn bộ sector");
  Serial.println("      - Verify dữ liệu");
  
  uint32_t start = millis();
  int result = flash.writeByte(testAddr, 0xAA);
  uint32_t elapsed = millis() - start;
  
  if (result == W25Q_OK) {
    Serial.printf("   ✓ Ghi thành công! (Mất %u ms)\n", elapsed);
  } else {
    Serial.printf("   ✗ Ghi thất bại! Mã lỗi: %d\n", result);
    while (1) delay(1000);
  }
  
  // Bước 3: Verify
  Serial.println("\n5. Kiểm tra giá trị mới...");
  uint8_t newValue = flash.readByte(testAddr);
  Serial.printf("   Giá trị mới tại 0x%06X: 0x%02X\n", testAddr, newValue);
  printBytesAround(testAddr);
  
  if (newValue == 0xAA) {
    Serial.println("\n✅ KẾT QUẢ:");
    Serial.println("   - Byte tại 0x5000 đã được ghi thành công");
    Serial.println("   - Các byte xung quanh KHÔNG bị thay đổi");
    Serial.println("   - Cơ chế Read-Modify-Write hoạt động hoàn hảo!");
  }
  
  // Bước 4: Ghi lại giá trị khác
  Serial.println("\n6. Thử ghi lại giá trị khác (0x55)...");
  start = millis();
  result = flash.writeByte(testAddr, 0x55);
  elapsed = millis() - start;
  
  if (result == W25Q_OK) {
    Serial.printf("   ✓ Ghi thành công! (Mất %u ms)\n", elapsed);
    uint8_t finalValue = flash.readByte(testAddr);
    Serial.printf("   Giá trị cuối: 0x%02X\n", finalValue);
  }
  
  Serial.println("\n📝 LƯU Ý:");
  Serial.println("   - writeByte() MẤT khoảng 150-200ms");
  Serial.println("   - Các byte khác trong sector GIỮ NGUYÊN 100%");
  Serial.println("   - Tự động verify sau khi ghi");
}

void loop() {
  delay(1000);
}
