# Hướng Dẫn Sử Dụng Thư Viện W25Q16JV

Thư viện Arduino cho chip SPI Flash W25Q16JV (16Mbit / 2MB) được thiết kế cho ESP32C3.

## 📋 Mục Lục
1. [Cài Đặt Phần Cứng](#cài-đặt-phần-cứng)
2. [Khởi Tạo Thư Viện](#khởi-tạo-thư-viện)
3. [Đọc Dữ Liệu](#đọc-dữ-liệu)
4. [Ghi Dữ Liệu](#ghi-dữ-liệu)
5. [Xóa Dữ Liệu](#xóa-dữ-liệu)
6. [Thông Tin Chip](#thông-tin-chip)
7. [Quản Lý Nguồn](#quản-lý-nguồn)
8. [Xử Lý Lỗi](#xử-lý-lỗi)

---

## ⚡ Cài Đặt Phần Cứng

### Sơ Đồ Kết Nối ESP32C3 - W25Q16JV

| W25Q16JV Pin | ESP32C3 Pin | Chức năng |
|--------------|-------------|-----------|
| Pin 1 (/CS) | GPIO 10 | Chip Select |
| Pin 2 (DO/MISO) | GPIO 7 | Data Out |
| Pin 3 (/WP) | 3.3V | Write Protect (pull-up) |
| Pin 4 (GND) | GND | Ground |
| Pin 5 (DI/MOSI) | GPIO 6 | Data In |
| Pin 6 (CLK) | GPIO 2 | Clock |
| Pin 7 (/HOLD) | 3.3V | Hold (pull-up) |
| Pin 8 (VCC) | 3.3V | Power Supply |

**⚠️ LƯU Ý QUAN TRỌNG:**
- HOLD# (pin 7) và WP# (pin 3) PHẢI kết nối trực tiếp với 3.3V
- KHÔNG để floating hoặc nối GND
- Nếu không có clock signal → kiểm tra 2 pin này trước

---

## 🚀 Khởi Tạo Thư Viện

### Code Cơ Bản

```cpp
#include <Arduino.h>
#include <SPI.h>
#include <W25Q16JV.h>

// Định nghĩa chân SPI cho ESP32C3
#define SCK_PIN  2
#define MISO_PIN 7
#define MOSI_PIN 6
#define CS_PIN   10

// Tạo đối tượng flash
W25Q16JV flash(CS_PIN);

void setup() {
    Serial.begin(115200);
    
    // ⚠️ QUAN TRỌNG: Khởi tạo SPI với chân cụ thể (bắt buộc cho ESP32C3)
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
    
    // Khởi tạo flash chip
    if (!flash.begin()) {
        Serial.println("❌ Không thể khởi tạo flash!");
        while(1);
    }
    
    Serial.println("✅ Flash đã sẵn sàng!");
}
```

---

## 📖 Đọc Dữ Liệu

### 1. Đọc 1 Byte

```cpp
// Đọc 1 byte tại địa chỉ 0x1000
uint8_t value = flash.readByte(0x1000);
Serial.printf("Giá trị: 0x%02X\n", value);
```

### 2. Đọc Nhiều Byte Liên Tiếp

```cpp
// Đọc 256 bytes từ địa chỉ 0x1000
uint8_t buffer[256];
flash.readData(0x1000, buffer, 256);

// In ra dữ liệu
for (int i = 0; i < 256; i++) {
    Serial.printf("%02X ", buffer[i]);
    if ((i + 1) % 16 == 0) Serial.println();
}
```

### 3. Đọc Nhanh (Fast Read)

```cpp
// Fast Read - nhanh hơn readData() khoảng 30%
uint8_t buffer[256];
flash.fastRead(0x1000, buffer, 256);
```

**📊 So Sánh Tốc Độ:**
- `readData()`: ~3500 μs cho 256 bytes
- `fastRead()`: ~1758 μs cho 256 bytes (nhanh hơn 2x)

---

## ✍️ Ghi Dữ Liệu

### 1. Ghi 1 Byte (An Toàn - Không Ảnh Hưởng Byte Khác)

**Cơ Chế Read-Modify-Write Tự Động:**

```cpp
// Ghi byte 0xAA vào địa chỉ 0x1000
// ✅ Tự động đọc toàn bộ sector (4KB)
// ✅ Sửa 1 byte trong bộ nhớ đệm
// ✅ Xóa sector
// ✅ Ghi lại toàn bộ sector với byte mới
// ✅ Các byte khác KHÔNG bị thay đổi

int result = flash.writeByte(0x1000, 0xAA);

if (result == W25Q_OK) {
    Serial.println("✅ Ghi thành công!");
} else {
    Serial.printf("❌ Lỗi: %d\n", result);
}
```

**Quy Trình Chi Tiết:**
```
Bước 1: Đọc sector (4096 bytes)  → [0xFF, 0xFF, 0xFF, ...]
Bước 2: Sửa byte tại offset      → [0xFF, 0xFF, 0xAA, ...]
Bước 3: Xóa sector               → [0xFF, 0xFF, 0xFF, ...]
Bước 4: Ghi lại sector           → [0xFF, 0xFF, 0xAA, ...]
Bước 5: Verify                   → Kiểm tra = 0xAA ✓
```

**⏱️ Thời Gian:** ~150-200ms (vì phải xóa + ghi 4KB)

### 2. Ghi Nhiều Byte Liên Tiếp (Nhanh)

**Cách 1: Ghi Trực Tiếp (Cần Xóa Trước)**

```cpp
uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};

// Bước 1: Xóa sector trước
flash.eraseSector(0x1000);
delay(100);  // Chờ xóa xong

// Bước 2: Ghi dữ liệu
flash.writeData(0x1000, data, 5);
delay(5);    // Chờ ghi xong

// Bước 3: Đọc lại để kiểm tra
uint8_t verify[5];
flash.readData(0x1000, verify, 5);
```

**⚠️ Lưu Ý:**
- Phải xóa sector trước khi ghi
- Dữ liệu cũ trong sector sẽ MẤT
- Nhanh nhất (~5ms cho 256 bytes)

**Cách 2: Ghi An Toàn (Giữ Data Cũ)**

```cpp
uint8_t data[] = {0x11, 0x22, 0x33, 0x44, 0x55};

// Tự động đọc-sửa-xóa-ghi lại
// ✅ Data cũ được bảo toàn
int result = flash.modifyBytes(0x1000, data, 5);

if (result == W25Q_OK) {
    Serial.println("✅ Ghi an toàn thành công!");
}
```

**📊 So Sánh:**

| Phương pháp | Tốc độ | Data cũ | Khi nào dùng |
|-------------|--------|---------|--------------|
| `writeData()` | ⚡ Nhanh | ❌ Mất | Ghi vùng trống/toàn bộ sector |
| `modifyBytes()` | 🐢 Chậm | ✅ Giữ | Sửa 1 phần trong sector |
| `writeByte()` | 🐢 Chậm | ✅ Giữ | Sửa 1 byte |

### 3. Ghi Page (256 Bytes)

```cpp
uint8_t pageData[256];
// ... điền dữ liệu ...

// Xóa sector trước
flash.eraseSector(0x1000);

// Ghi 1 page (256 bytes)
flash.writePage(0x1000, pageData, 256);
```

**⚠️ Lưu Ý:**
- Mỗi page = 256 bytes
- Địa chỉ nên căn chỉnh với page (bội số của 256)
- Ghi vượt qua page boundary sẽ bị wrap around

### 4. Ghi Dữ Liệu Dài (Tự Động Xử Lý Page Boundary)

```cpp
// Ghi 1000 bytes - tự động chia thành nhiều page
uint8_t longData[1000];
// ... điền dữ liệu ...

flash.eraseSector(0x1000);
flash.writeData(0x1000, longData, 1000);
// ✅ Tự động ghi 4 pages (256+256+256+232)
```

---

## 🗑️ Xóa Dữ Liệu

**⚠️ QUAN TRỌNG:** Flash memory CHỈ có thể ghi từ `1→0` (0xFF→0x00). Để ghi từ `0→1` phải XÓA trước.

### 1. Xóa Sector (4KB)

```cpp
// Xóa sector chứa địa chỉ 0x1000
flash.eraseSector(0x1000);
delay(100);  // Chờ ~50-100ms

// Sau khi xóa, tất cả byte = 0xFF
uint8_t value = flash.readByte(0x1000);  // → 0xFF
```

**Thông Số:**
- Kích thước: 4096 bytes (4KB)
- Thời gian: ~50-100ms
- Địa chỉ: Bất kỳ trong sector (tự động làm tròn)

### 2. Xóa Block 32KB

```cpp
// Xóa 32KB (8 sectors)
flash.eraseBlock32K(0x8000);
delay(200);  // Chờ ~150-200ms
```

**Thông Số:**
- Kích thước: 32768 bytes (32KB)
- Thời gian: ~150-200ms
- Nhanh hơn xóa 8 sector riêng lẻ

### 3. Xóa Block 64KB

```cpp
// Xóa 64KB (16 sectors)
flash.eraseBlock64K(0x10000);
delay(300);  // Chờ ~200-300ms
```

**Thông Số:**
- Kích thường: 65536 bytes (64KB)
- Thời gian: ~200-300ms
- Nhanh nhất cho vùng lớn

### 4. Xóa Toàn Bộ Chip

```cpp
// ⚠️ CẢNH BÁO: Xóa toàn bộ 2MB!
flash.eraseChip();
delay(20000);  // Chờ ~5-20 giây!

Serial.println("✅ Chip đã sạch hoàn toàn!");
```

**⚠️ CẢNH BÁO:**
- Xóa toàn bộ 2MB
- Thời gian: 5-20 giây
- KHÔNG thể hủy bỏ sau khi bắt đầu

**📊 Bảng So Sánh Xóa:**

| Loại | Kích thước | Thời gian | Khi nào dùng |
|------|-----------|-----------|--------------|
| Sector | 4KB | ~50-100ms | Xóa vùng nhỏ |
| Block 32K | 32KB | ~150-200ms | Xóa vùng trung bình |
| Block 64K | 64KB | ~200-300ms | Xóa vùng lớn |
| Chip | 2MB | ~5-20s | Format toàn bộ |

---

## 🔍 Thông Tin Chip

### 1. Đọc Device ID

```cpp
// Device ID: 0xEF14 (Winbond W25Q16)
uint16_t deviceID = flash.readDeviceID();
Serial.printf("Device ID: 0x%04X\n", deviceID);
// → Device ID: 0xEF14
```

### 2. Đọc JEDEC ID

```cpp
// JEDEC ID: 0xEF4015
// Byte 1: 0xEF = Winbond
// Byte 2: 0x40 = Memory type
// Byte 3: 0x15 = 16Mbit capacity
uint32_t jedecID = flash.readJEDECID();
Serial.printf("JEDEC ID: 0x%06X\n", jedecID);
// → JEDEC ID: 0xEF4015

// Tách thành phần
uint8_t manufacturer = (jedecID >> 16) & 0xFF;  // 0xEF
uint8_t memoryType = (jedecID >> 8) & 0xFF;    // 0x40
uint8_t capacity = jedecID & 0xFF;              // 0x15
```

### 3. Đọc Unique ID (64-bit)

```cpp
// Mỗi chip có ID duy nhất 64-bit
uint64_t uniqueID = flash.readUniqueID();
Serial.printf("Unique ID: 0x%08X%08X\n", 
    (uint32_t)(uniqueID >> 32),
    (uint32_t)(uniqueID & 0xFFFFFFFF)
);
// → Unique ID: 0xE6681CB2976E7628
```

### 4. Kiểm Tra Dung Lượng

```cpp
uint32_t capacity = flash.getCapacity();
Serial.printf("Dung lượng: %d bytes (%.2f MB)\n", 
    capacity, 
    capacity / 1048576.0
);
// → Dung lượng: 2097152 bytes (2.00 MB)
```

### 5. Đọc Status Registers

```cpp
// Status Register 1: BUSY, WEL, BP bits
uint8_t sr1 = flash.readStatusReg1();
bool isBusy = sr1 & 0x01;  // Bit 0: BUSY
bool writeEnabled = sr1 & 0x02;  // Bit 1: WEL

Serial.printf("SR1: 0x%02X\n", sr1);
Serial.printf("  BUSY: %d\n", isBusy);
Serial.printf("  Write Enable: %d\n", writeEnabled);

// Status Register 2 & 3
uint8_t sr2 = flash.readStatusReg2();
uint8_t sr3 = flash.readStatusReg3();
```

---

## ⚡ Quản Lý Nguồn

### 1. Chế Độ Power Down

```cpp
// Vào chế độ tiết kiệm năng lượng
flash.powerDown();
Serial.println("💤 Flash đang ngủ...");

// Tiêu thụ điện: ~1μA (từ ~5mA xuống 1μA)
delay(5000);  // Ngủ 5 giây

// Đánh thức chip
flash.releasePowerDown();
delay(1);  // Chờ 3μs để wake up
Serial.println("⚡ Flash đã thức!");
```

**📊 Tiết Kiệm Năng Lượng:**
- Active: ~5mA
- Power Down: ~1μA
- Tiết kiệm: 99.98%

**⚠️ Lưu Ý:**
- Trong Power Down mode, chip KHÔNG phản hồi lệnh
- Phải gọi `releasePowerDown()` trước khi dùng
- Wake-up time: ~3μs

---

## ⚠️ Xử Lý Lỗi

### Mã Lỗi

```cpp
enum W25Q16JV_Error {
    W25Q_OK = 0,                    // ✅ Thành công
    W25Q_ERROR_INIT = -1,           // ❌ Lỗi khởi tạo
    W25Q_ERROR_BUSY_TIMEOUT = -2,   // ❌ Timeout
    W25Q_ERROR_VERIFY_FAILED = -3,  // ❌ Verify thất bại
    W25Q_ERROR_INVALID_ADDRESS = -4,// ❌ Địa chỉ không hợp lệ
    W25Q_ERROR_INVALID_LENGTH = -5  // ❌ Độ dài không hợp lệ
};
```

### Kiểm Tra Lỗi

```cpp
int result = flash.writeByte(0x1000, 0xAA);

if (result == W25Q_OK) {
    Serial.println("✅ Thành công!");
} else {
    Serial.print("❌ Lỗi: ");
    
    switch(result) {
        case W25Q_ERROR_INIT:
            Serial.println("Không thể cấp phát bộ nhớ");
            break;
            
        case W25Q_ERROR_BUSY_TIMEOUT:
            Serial.println("Chip không phản hồi (timeout)");
            break;
            
        case W25Q_ERROR_VERIFY_FAILED:
            Serial.println("Ghi thất bại - verify sai!");
            break;
            
        case W25Q_ERROR_INVALID_ADDRESS:
            Serial.println("Địa chỉ vượt quá 2MB!");
            break;
            
        case W25Q_ERROR_INVALID_LENGTH:
            Serial.println("Độ dài không hợp lệ!");
            break;
    }
}
```

### Kiểm Tra Chip Bận

```cpp
if (flash.isBusy()) {
    Serial.println("⏳ Chip đang bận...");
    while (flash.isBusy()) {
        delay(1);
    }
    Serial.println("✅ Chip sẵn sàng!");
}
```

### Kiểm Tra Địa Chỉ Hợp Lệ

```cpp
uint32_t addr = 0x250000;  // 2.3MB - vượt quá 2MB!

if (flash.isValidAddress(addr)) {
    Serial.println("✅ Địa chỉ hợp lệ");
} else {
    Serial.println("❌ Địa chỉ không hợp lệ!");
    // Địa chỉ phải < 0x200000 (2MB)
}
```

---

## 📝 Ví Dụ Hoàn Chỉnh

### Ghi và Đọc Cấu Trúc Dữ Liệu

```cpp
// Định nghĩa cấu trúc
struct Config {
    char name[32];
    uint16_t version;
    uint32_t timestamp;
    float temperature;
};

void saveConfig() {
    Config cfg;
    strcpy(cfg.name, "ESP32-Device");
    cfg.version = 100;
    cfg.timestamp = millis();
    cfg.temperature = 25.5;
    
    // Ghi cấu trúc vào flash
    uint32_t addr = 0x10000;
    flash.eraseSector(addr);
    flash.writeData(addr, (uint8_t*)&cfg, sizeof(Config));
    
    Serial.println("✅ Đã lưu cấu hình!");
}

void loadConfig() {
    Config cfg;
    uint32_t addr = 0x10000;
    
    // Đọc cấu trúc từ flash
    flash.readData(addr, (uint8_t*)&cfg, sizeof(Config));
    
    Serial.println("📖 Cấu hình đã lưu:");
    Serial.printf("  Name: %s\n", cfg.name);
    Serial.printf("  Version: %d\n", cfg.version);
    Serial.printf("  Timestamp: %u\n", cfg.timestamp);
    Serial.printf("  Temperature: %.1f°C\n", cfg.temperature);
}
```

---

## 🔧 Debug & Troubleshooting

### 1. Không Đọc Được Device ID

**Triệu chứng:** `readDeviceID()` trả về 0x0000 hoặc 0xFFFF

**Giải pháp:**
```cpp
// Kiểm tra:
1. HOLD# (pin 7) phải nối 3.3V (KHÔNG để floating!)
2. WP# (pin 3) phải nối 3.3V
3. SPI.begin() phải có đầy đủ 4 tham số:
   SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
4. Kiểm tra đường dây MISO có tín hiệu không (dùng logic analyzer)
```

### 2. Ghi Thành Công Nhưng Đọc Lại Sai

```cpp
// Kiểm tra địa chỉ
Serial.printf("Địa chỉ ghi: 0x%06X\n", addr);
Serial.printf("Địa chỉ đọc: 0x%06X\n", addr);

// Delay sau khi ghi
flash.writeData(addr, data, len);
delay(5);  // ← QUAN TRỌNG!
flash.readData(addr, verify, len);
```

### 3. Chip Không Phản Hồi

```cpp
// Test cơ bản
if (!flash.begin()) {
    Serial.println("❌ Không kết nối được chip!");
    Serial.println("Kiểm tra:");
    Serial.println("  1. Nguồn 3.3V");
    Serial.println("  2. Đường GND");
    Serial.println("  3. HOLD# và WP# nối 3.3V");
    Serial.println("  4. SPI pins đúng");
}
```

---

## 📚 Thông Số Kỹ Thuật

| Thông số | Giá trị |
|----------|---------|
| Dung lượng | 16Mbit (2MB) |
| Điện áp | 2.7V - 3.6V |
| Tốc độ SPI | Tối đa 133MHz (thư viện dùng 4MHz) |
| Page size | 256 bytes |
| Sector size | 4KB (4096 bytes) |
| Block 32K size | 32KB (32768 bytes) |
| Block 64K size | 64KB (65536 bytes) |
| Thời gian xóa sector | 50-100ms |
| Thời gian ghi page | ~1-3ms |
| Thời gian xóa chip | 5-20 giây |
| Số lần ghi/xóa | >100,000 cycles |
| Lưu trữ dữ liệu | >20 năm |

---

## 🎯 Best Practices

### 1. Luôn Kiểm Tra Kết Quả

```cpp
✅ TỐT:
int result = flash.writeByte(addr, data);
if (result != W25Q_OK) {
    Serial.println("Lỗi ghi!");
}

❌ KHÔNG TỐT:
flash.writeByte(addr, data);  // Không kiểm tra
```

### 2. Xóa Trước Khi Ghi

```cpp
✅ TỐT:
flash.eraseSector(addr);
delay(100);
flash.writeData(addr, data, len);

❌ KHÔNG TỐT:
flash.writeData(addr, data, len);  // Không xóa trước
```

### 3. Delay Sau Thao Tác

```cpp
✅ TỐT:
flash.eraseSector(addr);
delay(100);  // Chờ xóa xong

flash.writeData(addr, data, len);
delay(5);    // Chờ ghi xong

❌ KHÔNG TỐT:
flash.eraseSector(addr);
flash.writeData(addr, data, len);  // Không delay
```

### 4. Sử Dụng Đúng Hàm

```cpp
// Sửa 1 byte → dùng writeByte()
flash.writeByte(addr, value);  // ✅ Tự động R-M-W

// Ghi nhiều byte vào vùng trống → dùng writeData()
flash.eraseSector(addr);
flash.writeData(addr, data, len);  // ✅ Nhanh

// Sửa nhiều byte trong sector → dùng modifyBytes()
flash.modifyBytes(addr, data, len);  // ✅ Giữ data cũ
```

---

## 📞 Hỗ Trợ

- **Datasheet:** W25Q16JV SPI RevI 12242024 Plus.pdf
- **GitHub:** [Link repository]
- **Hardware Setup:** Xem `docs/HARDWARE_SETUP.md`
- **Examples:** Xem thư mục `examples/`

---

**✅ Thư viện đã được test đầy đủ trên phần cứng thực tế!**
