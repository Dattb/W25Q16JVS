/*
 * Example 5: Xóa Dữ Liệu
 * 
 * Ví dụ này hướng dẫn các cách xóa dữ liệu:
 * - Xóa Sector (4KB)
 * - Xóa Block 32KB
 * - Xóa Block 64KB
 * - Xóa toàn bộ Chip (2MB)
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

void checkErased(uint32_t addr, uint32_t len, const char* name) {
  Serial.printf("\nKiểm tra %s đã xóa...\n", name);
  
  bool allFF = true;
  uint8_t buffer[256];
  
  // Kiểm tra từng chunk 256 bytes
  for (uint32_t i = 0; i < len; i += 256) {
    uint32_t chunkSize = min(256, (int)(len - i));
    flash.readData(addr + i, buffer, chunkSize);
    
    for (uint32_t j = 0; j < chunkSize; j++) {
      if (buffer[j] != 0xFF) {
        Serial.printf("✗ Tìm thấy byte không phải 0xFF tại 0x%06X: 0x%02X\n", 
                     addr + i + j, buffer[j]);
        allFF = false;
        break;
      }
    }
    
    if (!allFF) break;
  }
  
  if (allFF) {
    Serial.printf("✓ %s đã xóa sạch (tất cả = 0xFF)\n", name);
  } else {
    Serial.printf("✗ %s chưa xóa hết!\n", name);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n=== VÍ DỤ: XÓA DỮ LIỆU ===\n");
  
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
  
  // ========== TEST 1: XÓA SECTOR (4KB) ==========
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 1: XÓA SECTOR (4KB = 4096 bytes)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint32_t sectorAddr = 0x10000;
  
  // Ghi dữ liệu test trước
  Serial.println("1. Ghi dữ liệu test vào sector...");
  uint8_t testData[256];
  for (int i = 0; i < 256; i++) testData[i] = 0xAA;
  flash.eraseSector(sectorAddr);
  delay(100);
  flash.writeData(sectorAddr, testData, 256);
  delay(10);
  Serial.printf("   ✓ Đã ghi 256 bytes vào 0x%06X\n", sectorAddr);
  
  // Kiểm tra trước khi xóa
  uint8_t beforeErase = flash.readByte(sectorAddr);
  Serial.printf("   Giá trị trước khi xóa: 0x%02X\n", beforeErase);
  
  // Xóa sector
  Serial.println("\n2. Xóa sector...");
  Serial.printf("   Địa chỉ: 0x%06X\n", sectorAddr);
  uint32_t start = millis();
  flash.eraseSector(sectorAddr);
  uint32_t elapsed = millis() - start;
  Serial.printf("   ✓ Xóa xong! (Mất %u ms)\n", elapsed);
  delay(100);
  
  // Kiểm tra sau khi xóa
  uint8_t afterErase = flash.readByte(sectorAddr);
  Serial.printf("\n3. Giá trị sau khi xóa: 0x%02X\n", afterErase);
  
  checkErased(sectorAddr, 4096, "Sector");
  
  // ========== TEST 2: XÓA BLOCK 32KB ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 2: XÓA BLOCK 32KB (8 sectors)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint32_t block32Addr = 0x20000;
  
  Serial.println("1. Ghi dữ liệu test...");
  flash.eraseSector(block32Addr);
  delay(100);
  flash.writeData(block32Addr, testData, 256);
  delay(10);
  Serial.printf("   ✓ Đã ghi tại 0x%06X\n", block32Addr);
  
  Serial.println("\n2. Xóa block 32KB...");
  start = millis();
  flash.eraseBlock32K(block32Addr);
  elapsed = millis() - start;
  Serial.printf("   ✓ Xóa xong! (Mất %u ms)\n", elapsed);
  delay(100);
  
  checkErased(block32Addr, 32768, "Block 32KB");
  
  // ========== TEST 3: XÓA BLOCK 64KB ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 3: XÓA BLOCK 64KB (16 sectors)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint32_t block64Addr = 0x30000;
  
  Serial.println("1. Ghi dữ liệu test...");
  flash.eraseSector(block64Addr);
  delay(100);
  flash.writeData(block64Addr, testData, 256);
  delay(10);
  Serial.printf("   ✓ Đã ghi tại 0x%06X\n", block64Addr);
  
  Serial.println("\n2. Xóa block 64KB...");
  start = millis();
  flash.eraseBlock64K(block64Addr);
  elapsed = millis() - start;
  Serial.printf("   ✓ Xóa xong! (Mất %u ms)\n", elapsed);
  delay(100);
  
  checkErased(block64Addr, 65536, "Block 64KB");
  
  // ========== SO SÁNH TỐC ĐỘ ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("📊 SO SÁNH TỐC ĐỘ XÓA");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  // Xóa 1 sector
  start = millis();
  flash.eraseSector(0x40000);
  uint32_t time1Sector = millis() - start;
  
  // Xóa 8 sectors riêng lẻ
  start = millis();
  for (int i = 0; i < 8; i++) {
    flash.eraseSector(0x48000 + i * 4096);
    delay(50);
  }
  uint32_t time8Sectors = millis() - start;
  
  // Xóa 1 block 32KB
  start = millis();
  flash.eraseBlock32K(0x50000);
  uint32_t time32K = millis() - start;
  
  Serial.printf("Xóa 1 sector (4KB):        %u ms\n", time1Sector);
  Serial.printf("Xóa 8 sectors riêng lẻ:    %u ms\n", time8Sectors);
  Serial.printf("Xóa 1 block 32KB:          %u ms\n", time32K);
  Serial.printf("Tiết kiệm thời gian:       %u ms (%.1f%%)\n", 
               time8Sectors - time32K,
               100.0 * (time8Sectors - time32K) / time8Sectors);
  
  Serial.println("\n✅ Hoàn tất!");
  Serial.println("\n📝 KẾT LUẬN:");
  Serial.println("   - Xóa sector: ~50-100ms");
  Serial.println("   - Xóa block 32KB: ~150-200ms (nhanh hơn xóa 8 sectors)");
  Serial.println("   - Xóa block 64KB: ~200-300ms (nhanh hơn xóa 16 sectors)");
  Serial.println("   - Sau khi xóa, tất cả byte = 0xFF");
  
  Serial.println("\n⚠️  LƯU Ý:");
  Serial.println("   - KHÔNG sử dụng eraseChip() trong ví dụ này");
  Serial.println("   - eraseChip() mất 5-20 giây và xóa TOÀN BỘ 2MB!");
}

void loop() {
  delay(1000);
}
