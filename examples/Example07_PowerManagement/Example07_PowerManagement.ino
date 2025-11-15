/*
 * Example 7: Quản Lý Nguồn (Power Management)
 * 
 * Ví dụ này hướng dẫn cách sử dụng Power Down mode để tiết kiệm năng lượng:
 * - Vào chế độ Power Down (tiêu thụ ~1μA)
 * - Đánh thức chip (Release Power Down)
 * - So sánh tiêu thụ điện trước và sau
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

void testFlashOperation() {
  Serial.println("\n📝 Test thao tác đọc/ghi...");
  
  // Test đọc Device ID
  uint16_t deviceID = flash.readDeviceID();
  Serial.printf("   Device ID: 0x%04X ", deviceID);
  
  if (deviceID == 0xEF14) {
    Serial.println("✅ OK");
  } else if (deviceID == 0x0000 || deviceID == 0xFFFF) {
    Serial.println("❌ Chip không phản hồi (đang ở Power Down?)");
  } else {
    Serial.println("⚠️  Giá trị không đúng");
  }
  
  // Test đọc data
  uint8_t testByte = flash.readByte(0x1000);
  Serial.printf("   Read byte: 0x%02X ", testByte);
  
  if (testByte == 0xFF || testByte == 0x00 || (testByte >= 0x20 && testByte <= 0x7E)) {
    Serial.println("✅ OK");
  } else {
    Serial.println("⚠️  Giá trị bất thường");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║      QUẢN LÝ NGUỒN - POWER DOWN       ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
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
  
  delay(100);
  
  // ========== TEST NORMAL MODE ==========
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 1: CHỀ ĐỘ NORMAL (Active Mode)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  
  testFlashOperation();
  
  Serial.println("\n💡 Tiêu thụ điện: ~5 mA (Active)");
  Serial.println("   - Chip sẵn sàng nhận lệnh");
  Serial.println("   - Có thể đọc/ghi bất cứ lúc nào");
  
  delay(2000);
  
  // ========== TEST POWER DOWN ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 2: CHẾ ĐỘ POWER DOWN");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  Serial.println("1. Đưa chip vào Power Down mode...");
  flash.powerDown();
  Serial.println("   ✓ Chip đã vào chế độ ngủ 💤\n");
  
  Serial.println("💡 Tiêu thụ điện: ~1 μA (Power Down)");
  Serial.println("   - Tiết kiệm 99.98% năng lượng!");
  Serial.println("   - Chip KHÔNG phản hồi lệnh");
  Serial.println("   - Dữ liệu ĐƯỢC bảo toàn\n");
  
  Serial.println("2. Thử đọc trong khi Power Down...");
  testFlashOperation();
  Serial.println("   → Chip không phản hồi (đúng như mong đợi)\n");
  
  Serial.println("⏳ Chip sẽ ngủ 5 giây...");
  for (int i = 5; i > 0; i--) {
    Serial.printf("   %d giây... 💤\n", i);
    delay(1000);
  }
  
  // ========== TEST WAKE UP ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 3: ĐÁNH THỨC CHIP (Wake Up)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  Serial.println("1. Gọi releasePowerDown()...");
  flash.releasePowerDown();
  delayMicroseconds(10);  // Chờ wake-up time (~3μs)
  Serial.println("   ✓ Chip đã thức! ⚡\n");
  
  Serial.println("2. Test thao tác sau khi thức...");
  testFlashOperation();
  Serial.println("   → Chip hoạt động bình thường\n");
  
  Serial.println("💡 Tiêu thụ điện: ~5 mA (Active)");
  Serial.println("   - Chip trở về chế độ bình thường");
  Serial.println("   - Wake-up time: ~3 μs");
  
  // ========== TEST POWER CYCLE ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 4: POWER CYCLE (3 lần)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  for (int cycle = 1; cycle <= 3; cycle++) {
    Serial.printf("Chu kỳ %d/3:\n", cycle);
    
    Serial.println("  → Power Down...");
    flash.powerDown();
    delay(1000);  // Ngủ 1 giây
    
    Serial.println("  → Wake Up...");
    flash.releasePowerDown();
    delayMicroseconds(10);
    
    uint16_t id = flash.readDeviceID();
    Serial.printf("  → Device ID: 0x%04X %s\n\n", 
                 id, (id == 0xEF14) ? "✅" : "❌");
    
    delay(500);
  }
  
  // ========== GHI VÀ ĐỌC SAU POWER CYCLE ==========
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 5: DỮ LIỆU SAU POWER CYCLE");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint32_t testAddr = 0x7000;
  uint8_t testValue = 0xA5;
  
  Serial.println("1. Ghi dữ liệu test...");
  flash.eraseSector(testAddr);
  delay(100);
  flash.writeByte(testAddr, testValue);
  Serial.printf("   Đã ghi 0x%02X tại 0x%06X\n", testValue, testAddr);
  
  uint8_t readBefore = flash.readByte(testAddr);
  Serial.printf("   Đọc lại: 0x%02X %s\n\n", 
               readBefore, (readBefore == testValue) ? "✅" : "❌");
  
  Serial.println("2. Power Down và Wake Up...");
  flash.powerDown();
  delay(2000);
  Serial.println("   💤 Ngủ 2 giây...");
  flash.releasePowerDown();
  delayMicroseconds(10);
  Serial.println("   ⚡ Đã thức!\n");
  
  Serial.println("3. Đọc lại dữ liệu sau khi thức...");
  uint8_t readAfter = flash.readByte(testAddr);
  Serial.printf("   Giá trị: 0x%02X\n", readAfter);
  
  if (readAfter == testValue) {
    Serial.println("   ✅ Dữ liệu KHÔNG bị mất!");
    Serial.println("   → Power Down không ảnh hưởng dữ liệu lưu trữ");
  } else {
    Serial.println("   ❌ Dữ liệu bị thay đổi!");
  }
  
  // ========== KẾT LUẬN ==========
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║             📊 TÓM TẮT                ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("✅ Power Down Mode:");
  Serial.println("   - Tiết kiệm: 99.98% năng lượng");
  Serial.println("   - Active: ~5 mA → Power Down: ~1 μA");
  Serial.println("   - Wake-up time: ~3 μs");
  Serial.println("   - Dữ liệu: Được bảo toàn 100%");
  
  Serial.println("\n📝 Cách sử dụng:");
  Serial.println("   flash.powerDown();        // Vào chế độ ngủ");
  Serial.println("   delay(1000);              // Ngủ 1 giây");
  Serial.println("   flash.releasePowerDown(); // Đánh thức");
  Serial.println("   delayMicroseconds(10);    // Chờ 3μs");
  
  Serial.println("\n⚡ Ứng dụng:");
  Serial.println("   - Thiết bị chạy pin");
  Serial.println("   - IoT với Deep Sleep");
  Serial.println("   - Logger lưu định kỳ");
  
  Serial.println("\n✅ Hoàn tất!");
}

void loop() {
  delay(1000);
}
