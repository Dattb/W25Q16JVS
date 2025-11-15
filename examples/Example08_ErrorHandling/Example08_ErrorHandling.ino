/*
 * Example 8: Xử Lý Lỗi (Error Handling)
 * 
 * Ví dụ này hướng dẫn cách kiểm tra và xử lý các lỗi thường gặp:
 * - Kiểm tra địa chỉ hợp lệ
 * - Kiểm tra chip busy
 * - Xử lý lỗi ghi/đọc
 * - Verify dữ liệu
 * - Timeout handling
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

void printErrorCode(int errorCode) {
  Serial.printf("Mã lỗi: %d - ", errorCode);
  
  switch(errorCode) {
    case 0:  // W25Q_OK
      Serial.println("✅ Thành công!");
      break;
      
    case -1:  // W25Q_ERROR_INIT
      Serial.println("❌ Lỗi khởi tạo!");
      Serial.println("   → Không thể cấp phát bộ nhớ sector buffer");
      Serial.println("   → Kiểm tra RAM còn trống");
      break;
      
    case -2:  // W25Q_ERROR_BUSY_TIMEOUT
      Serial.println("❌ Timeout!");
      Serial.println("   → Chip không phản hồi");
      Serial.println("   → Kiểm tra kết nối phần cứng");
      break;
      
    case -3:  // W25Q_ERROR_VERIFY_FAILED
      Serial.println("❌ Verify thất bại!");
      Serial.println("   → Dữ liệu ghi không khớp với dữ liệu đọc");
      Serial.println("   → Có thể do lỗi phần cứng hoặc nhiễu");
      break;
      
    case -4:  // W25Q_ERROR_INVALID_ADDRESS
      Serial.println("❌ Địa chỉ không hợp lệ!");
      Serial.println("   → Địa chỉ vượt quá 0x1FFFFF (2MB)");
      Serial.println("   → W25Q16JV chỉ có 2MB bộ nhớ");
      break;
      
    case -5:  // W25Q_ERROR_INVALID_LENGTH
      Serial.println("❌ Độ dài không hợp lệ!");
      Serial.println("   → Độ dài quá lớn hoặc = 0");
      Serial.println("   → Kiểm tra tham số length");
      break;
      
    default:
      Serial.printf("❌ Lỗi không xác định: %d\n", errorCode);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  delay(500);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║       XỬ LÝ LỖI - ERROR HANDLING      ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Khởi tạo SPI
  Serial.println("Khởi tạo SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("✓ SPI đã sẵn sàng\n");
  
  // Khởi tạo flash
  Serial.println("Khởi tạo W25Q16JV...");
  if (!flash.begin()) {
    Serial.println("✗ Không thể khởi tạo flash!");
    Serial.println("\n🔧 TROUBLESHOOTING:");
    Serial.println("   1. Kiểm tra nguồn 3.3V");
    Serial.println("   2. Kiểm tra GND");
    Serial.println("   3. HOLD# (pin 7) phải nối 3.3V");
    Serial.println("   4. WP# (pin 3) phải nối 3.3V");
    Serial.println("   5. Kiểm tra các chân SPI:");
    Serial.println("      - SCK = GPIO 2");
    Serial.println("      - MISO = GPIO 7");
    Serial.println("      - MOSI = GPIO 6");
    Serial.println("      - CS = GPIO 10");
    while (1) delay(1000);
  }
  Serial.println("✓ Flash đã sẵn sàng\n");
  
  delay(100);
  
  // ========== TEST 1: ĐỊA CHỈ HỢP LỆ ==========
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 1: KIỂM TRA ĐỊA CHỈ HỢP LỆ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  uint32_t testAddresses[] = {
    0x000000,   // Địa chỉ đầu - OK
    0x100000,   // Giữa chip - OK
    0x1FFFFF,   // Địa chỉ cuối - OK
    0x200000,   // Vượt quá 2MB - ERROR!
    0x300000,   // Quá xa - ERROR!
    0xFFFFFFFF  // Max uint32 - ERROR!
  };
  
  for (int i = 0; i < 6; i++) {
    uint32_t addr = testAddresses[i];
    bool valid = flash.isValidAddress(addr);
    
    Serial.printf("Địa chỉ 0x%08X: ", addr);
    if (valid) {
      Serial.println("✅ Hợp lệ");
    } else {
      Serial.println("❌ KHÔNG hợp lệ!");
      Serial.printf("   → Vượt quá 0x1FFFFF (địa chỉ max = %u bytes)\n", 
                   flash.getCapacity() - 1);
    }
  }
  
  // ========== TEST 2: GHI VỚI ĐỊA CHỈ KHÔNG HỢP LỆ ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 2: GHI VỚI ĐỊA CHỈ KHÔNG HỢP LỆ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  Serial.println("Thử ghi vào địa chỉ 0x300000 (vượt quá 2MB)...");
  int result = flash.writeByte(0x300000, 0xAA);
  printErrorCode(result);
  
  // ========== TEST 3: KIỂM TRA CHIP BUSY ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 3: KIỂM TRA CHIP BUSY");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  Serial.println("1. Kiểm tra trạng thái ban đầu...");
  if (flash.isBusy()) {
    Serial.println("   ⏳ Chip đang bận");
  } else {
    Serial.println("   ✅ Chip sẵn sàng");
  }
  
  Serial.println("\n2. Thực hiện thao tác xóa (chip sẽ busy)...");
  uint32_t testAddr = 0x8000;
  flash.eraseSector(testAddr);
  
  Serial.println("   Kiểm tra ngay sau khi xóa:");
  if (flash.isBusy()) {
    Serial.println("   ⏳ Chip đang bận (xóa sector ~50-100ms)");
    
    // Đợi chip sẵn sàng
    Serial.print("   Đang đợi");
    while (flash.isBusy()) {
      Serial.print(".");
      delay(10);
    }
    Serial.println("\n   ✅ Chip đã sẵn sàng!");
  }
  
  // ========== TEST 4: VERIFY SAU KHI GHI ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 4: VERIFY SAU KHI GHI");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  testAddr = 0x9000;
  uint8_t writeValue = 0x55;
  
  Serial.printf("1. Ghi giá trị 0x%02X vào địa chỉ 0x%06X...\n", writeValue, testAddr);
  result = flash.writeByte(testAddr, writeValue);
  printErrorCode(result);
  
  if (result == 0) {  // W25Q_OK
    Serial.println("\n2. Verify bằng cách đọc lại...");
    uint8_t readValue = flash.readByte(testAddr);
    Serial.printf("   Giá trị ghi: 0x%02X\n", writeValue);
    Serial.printf("   Giá trị đọc: 0x%02X\n", readValue);
    
    if (readValue == writeValue) {
      Serial.println("   ✅ Verify thành công!");
    } else {
      Serial.println("   ❌ Verify thất bại - giá trị không khớp!");
    }
  }
  
  // ========== TEST 5: XỬ LÝ LỖI TRONG THỰC TẾ ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 5: VÍ DỤ XỬ LÝ LỖI THỰC TẾ");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  Serial.println("Function: safeWriteByte()");
  Serial.println("Tự động xử lý lỗi và retry\n");
  
  // Hàm ghi an toàn với retry
  auto safeWriteByte = [](uint32_t addr, uint8_t data, int maxRetries = 3) -> bool {
    for (int retry = 0; retry < maxRetries; retry++) {
      // Kiểm tra địa chỉ trước
      if (!flash.isValidAddress(addr)) {
        Serial.printf("❌ Địa chỉ 0x%06X không hợp lệ!\n", addr);
        return false;
      }
      
      // Thử ghi
      int result = flash.writeByte(addr, data);
      
      if (result == 0) {  // W25Q_OK
        Serial.printf("✅ Ghi thành công tại 0x%06X\n", addr);
        return true;
      }
      
      // Xử lý lỗi
      Serial.printf("⚠️  Lần thử %d/%d thất bại: ", retry + 1, maxRetries);
      printErrorCode(result);
      
      if (result == -4) {  // W25Q_ERROR_INVALID_ADDRESS
        return false;  // Không retry với lỗi địa chỉ
      }
      
      if (retry < maxRetries - 1) {
        Serial.println("   → Thử lại sau 100ms...");
        delay(100);
      }
    }
    
    Serial.println("❌ Ghi thất bại sau tất cả các lần thử!");
    return false;
  };
  
  // Test với địa chỉ hợp lệ
  Serial.println("Test 1: Địa chỉ hợp lệ (0xA000)");
  safeWriteByte(0xA000, 0x77);
  
  Serial.println("\nTest 2: Địa chỉ không hợp lệ (0x300000)");
  safeWriteByte(0x300000, 0x88);
  
  // ========== TEST 6: ĐỌC AN TOÀN ==========
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.println("TEST 6: ĐỌC AN TOÀN VỚI KIỂM TRA");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
  
  auto safeReadByte = [](uint32_t addr, uint8_t* value) -> bool {
    // Kiểm tra địa chỉ
    if (!flash.isValidAddress(addr)) {
      Serial.printf("❌ Địa chỉ 0x%06X không hợp lệ!\n", addr);
      return false;
    }
    
    // Kiểm tra chip ready
    if (flash.isBusy()) {
      Serial.println("⏳ Đang đợi chip sẵn sàng...");
      int timeout = 1000;  // 1 giây
      while (flash.isBusy() && timeout > 0) {
        delay(1);
        timeout--;
      }
      
      if (timeout == 0) {
        Serial.println("❌ Timeout - chip không phản hồi!");
        return false;
      }
    }
    
    // Đọc dữ liệu
    *value = flash.readByte(addr);
    Serial.printf("✅ Đọc thành công: 0x%02X tại 0x%06X\n", *value, addr);
    return true;
  };
  
  uint8_t value;
  Serial.println("Test đọc địa chỉ 0xA000:");
  safeReadByte(0xA000, &value);
  
  Serial.println("\nTest đọc địa chỉ không hợp lệ:");
  safeReadByte(0x300000, &value);
  
  // ========== KẾT LUẬN ==========
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║          📋 BEST PRACTICES            ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("✅ Luôn kiểm tra:");
  Serial.println("   1. Địa chỉ hợp lệ (< 0x200000)");
  Serial.println("   2. Chip busy/ready");
  Serial.println("   3. Mã lỗi trả về");
  Serial.println("   4. Verify sau khi ghi");
  
  Serial.println("\n✅ Xử lý lỗi:");
  Serial.println("   1. Sử dụng switch-case cho mã lỗi");
  Serial.println("   2. Retry với timeout");
  Serial.println("   3. Log chi tiết lỗi");
  Serial.println("   4. Không bỏ qua giá trị trả về");
  
  Serial.println("\n✅ Code ví dụ:");
  Serial.println("   int result = flash.writeByte(addr, data);");
  Serial.println("   if (result != W25Q_OK) {");
  Serial.println("       handleError(result);");
  Serial.println("   }");
  
  Serial.println("\n✅ Hoàn tất!");
}

void loop() {
  delay(1000);
}
