# W25Q16JV SPI Flash Library for ESP32C3

## Tổng quan dự án (Project Overview)

Thư viện Arduino để giao tiếp với chip nhớ flash W25Q16JV (16Mbit/2MB) thông qua giao thức SPI trên ESP32C3.

Library for interfacing W25Q16JV SPI Flash Memory (16Mbit/2MB) with ESP32C3 using Arduino framework.

### Thông số kỹ thuật chính (Key Specifications)

- **Dung lượng (Capacity)**: 16Mbit (2,097,152 bytes / 2MB)
- **Giao thức (Interface)**: SPI (Standard, Dual, Quad)
- **Điện áp hoạt động (Voltage)**: 2.7V - 3.6V
- **Tốc độ SPI (SPI Speed)**: Lên đến 104MHz
- **Tổ chức bộ nhớ (Memory Organization)**:
  - 512 Sectors (4KB each)
  - 32 Blocks (64KB each)
  - 8,192 Pages (256 bytes each)

### Tính năng chính (Key Features)

1. ✅ Đọc/Ghi theo byte với cơ chế Read-Modify-Write
2. ✅ Đọc/Ghi trang (page programming)
3. ✅ Xóa theo sector (4KB), block (64KB), hoặc toàn bộ chip
4. ✅ Đọc thông tin ID và trạng thái
5. ✅ Bảo vệ dữ liệu với status register
6. ✅ Chế độ power-down để tiết kiệm năng lượng

## Cấu trúc dự án (Project Structure)

```
W25Q16JVS/
├── README.md                          # File này
├── DESIGN_DOCUMENT.md                 # Tài liệu thiết kế chi tiết
├── docs/
│   ├── HARDWARE_SETUP.md             # Hướng dẫn kết nối phần cứng
│   ├── API_REFERENCE.md              # Tài liệu API
│   └── MILESTONES.md                 # Chi tiết các milestone
├── src/
│   ├── W25Q16JV.h                    # Header file chính
│   └── W25Q16JV.cpp                  # Implementation
├── examples/
│   ├── Milestone1_BasicID/           # Đọc ID cơ bản
│   ├── Milestone2_StatusRegister/    # Thao tác status register
│   ├── Milestone3_ReadOperations/    # Các phép đọc
│   ├── Milestone4_EraseOperations/   # Các phép xóa
│   ├── Milestone5_WriteOperations/   # Các phép ghi
│   ├── Milestone6_ByteReadWrite/     # Đọc/ghi byte
│   └── FullDemo/                     # Demo đầy đủ tính năng
└── keywords.txt                       # Arduino keywords
```

## Kết nối phần cứng (Hardware Connections)

### Sơ đồ chân ESP32C3 - W25Q16JV

| ESP32C3 Pin | W25Q16JV Pin | Chức năng |
|-------------|--------------|-----------|
| GPIO2       | CLK (Pin 6)  | SPI Clock |
| GPIO7       | DO/IO1 (Pin 5) | MISO (Data Out) |
| GPIO6       | DI/IO0 (Pin 5) | MOSI (Data In) |
| GPIO10      | CS# (Pin 1)  | Chip Select |
| 3.3V        | VCC (Pin 8)  | Power Supply |
| GND         | GND (Pin 4)  | Ground |

**Lưu ý**: W25Q16JV có thể dùng chân HSPI hoặc VSPI của ESP32C3. Thư viện mặc định dùng VSPI.

## Cài đặt (Installation)

### Cách 1: Cài đặt thủ công

1. Tải về thư mục dự án
2. Copy thư mục `W25Q16JVS` vào `Arduino/libraries/`
3. Khởi động lại Arduino IDE

### Cách 2: Clone từ repository

```bash
cd ~/Arduino/libraries/
git clone <repository-url> W25Q16JVS
```

## Sử dụng nhanh (Quick Start)

```cpp
#include <W25Q16JV.h>

// Khởi tạo với chân CS = GPIO10
W25Q16JV flash(10);

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo SPI và flash
  if (flash.begin()) {
    Serial.println("Flash initialized!");
    
    // Đọc thông tin chip
    uint16_t id = flash.readDeviceID();
    Serial.printf("Device ID: 0x%04X\n", id);
  }
}

void loop() {
  // Ghi một byte tại địa chỉ 0x1000
  flash.writeByte(0x1000, 0xAB);
  
  // Đọc byte vừa ghi
  uint8_t data = flash.readByte(0x1000);
  Serial.printf("Data: 0x%02X\n", data);
  
  delay(1000);
}
```

## Lộ trình phát triển (Development Roadmap)

### ✅ Milestone 1: Basic SPI & ID Reading (Tuần 1)
- Khởi tạo SPI
- Đọc Manufacturer/Device ID
- Đọc JEDEC ID
- Kiểm tra kết nối

### ✅ Milestone 2: Status Register Operations (Tuần 1)
- Đọc/Ghi Status Register 1, 2, 3
- Kiểm tra trạng thái BUSY
- Chế độ bảo vệ block

### ✅ Milestone 3: Read Operations (Tuần 2)
- Read Data (chuẩn 0x03)
- Fast Read (0x0B)
- Đọc nhiều byte

### ✅ Milestone 4: Erase Operations (Tuần 2)
- Sector Erase (4KB)
- Block Erase 32KB & 64KB
- Chip Erase

### ✅ Milestone 5: Write Operations (Tuần 3)
- Write Enable/Disable
- Page Program (256 bytes)
- Ghi nhiều byte

### ✅ Milestone 6: Byte-Level Read-Modify-Write (Tuần 3)
- Đọc page hiện tại
- Sửa byte cần thiết
- Xóa và ghi lại
- Verify dữ liệu

### 🔄 Milestone 7: Advanced Features (Tuần 4)
- Power Down mode
- Unique ID reading
- SFDP register reading
- Performance optimization

## Giấy phép (License)

MIT License - Xem file LICENSE để biết chi tiết

## Tác giả (Author)

Được phát triển cho ESP32C3 với Arduino framework

## Đóng góp (Contributing)

Mọi đóng góp đều được chào đón! Vui lòng tạo issue hoặc pull request.

## Tài liệu tham khảo (References)

- W25Q16JV Datasheet Rev. I
- ESP32C3 Technical Reference Manual
- SPI Protocol Specification
