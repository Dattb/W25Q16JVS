châ# Hardware Setup Guide - W25Q16JV with ESP32C3

## Tổng quan (Overview)

Hướng dẫn kết nối phần cứng giữa ESP32C3 và chip flash W25Q16JV thông qua giao thức SPI.

## Sơ đồ chân (Pinout)

### ESP32C3 Pinout

ESP32C3 có 2 bộ SPI:
- **SPI0/SPI1**: Dành cho Flash nội bộ (không dùng)
- **SPI2 (HSPI/VSPI)**: Dùng cho thiết bị ngoại vi

Chân SPI mặc định của ESP32C3:
- **CLK**: GPIO2
- **MISO**: GPIO7  
- **MOSI**: GPIO6
- **CS**: Có thể chọn bất kỳ GPIO (khuyến nghị GPIO10)

### W25Q16JV Pinout (SOIC-8)

```
     ┌─────────┐
CS  -│1      8│- VCC (3.3V)
DO  -│2      7│- HOLD# (pull-up to VCC)
WP# -│3      6│- CLK
GND -│4      5│- DI
     └─────────┘
```

| Pin | Tên | Chức năng |
|-----|-----|-----------|
| 1 | CS# | Chip Select (Active Low) |
| 2 | DO (IO1) | Data Out / MISO |
| 3 | WP# | Write Protect (pull-up to VCC) |
| 4 | GND | Ground |
| 5 | DI (IO0) | Data In / MOSI |
| 6 | CLK | Serial Clock |
| 7 | HOLD# | Hold (pull-up to VCC) |
| 8 | VCC | Power Supply (2.7V - 3.6V) |

## Sơ đồ kết nối (Connection Diagram)

```
ESP32C3                      W25Q16JV
┌─────────┐                 ┌─────────┐
│         │                 │         │
│ GPIO2   ├─────────────────┤ CLK (6) │
│         │                 │         │
│ GPIO7   ├─────────────────┤ DO  (2) │
│         │                 │         │
│ GPIO6   ├─────────────────┤ DI  (5) │
│         │                 │         │
│ GPIO10  ├─────────────────┤ CS# (1) │
│         │                 │         │
│ 3.3V    ├────┬────────────┤ VCC (8) │
│         │    │            │         │
│         │    └────────────┤ HOLD(7) │ ← Nối lên 3.3V
│         │                 │         │
│         │    ┌────────────┤ WP# (3) │ ← Nối lên 3.3V  
│         │    │            │         │
│ GND     ├────┴────────────┤ GND (4) │
│         │                 │         │
└─────────┘                 └─────────┘

Lưu ý: HOLD# (7) và WP# (3) đều nối lên 3.3V (không nối xuống GND)
```

## Bảng kết nối chi tiết (Detailed Connections)

| ESP32C3 | Tín hiệu | W25Q16JV | Pin # | Ghi chú |
|---------|----------|----------|-------|---------|
| GPIO2 | SPI CLK | CLK | 6 | Serial Clock (lên đến 104MHz) |
| GPIO7 | SPI MISO | DO (IO1) | 2 | Master In Slave Out |
| GPIO6 | SPI MOSI | DI (IO0) | 5 | Master Out Slave In |
| GPIO10 | CS | CS# | 1 | Chip Select (Active Low) |
| 3.3V | Power | VCC | 8 | Nguồn cung cấp |
| 3.3V | Pull-up | HOLD# | 7 | Nối với VCC để vô hiệu hóa Hold |
| 3.3V | Pull-up | WP# | 3 | Nối với VCC để vô hiệu hóa Write Protect |
| GND | Ground | GND | 4 | Đất chung |

## Lưu ý quan trọng (Important Notes)

### 1. Điện áp (Voltage)
- W25Q16JV hoạt động ở **3.3V** (2.7V - 3.6V)
- **KHÔNG** kết nối với 5V - sẽ làm hỏng chip!
- ESP32C3 sử dụng logic 3.3V - phù hợp hoàn toàn

### 2. Pull-up Resistors
- **HOLD#** và **WP#** cần pull-up lên VCC
- Có thể dùng điện trở 10kΩ hoặc nối trực tiếp lên 3.3V
- Nếu không nối, chip có thể hoạt động không ổn định

### 3. Decoupling Capacitor
- Khuyến nghị thêm tụ 100nF gần chân VCC và GND
- Giúp ổn định nguồn và giảm nhiễu

### 4. Độ dài dây nối (Wire Length)
- Giữ dây nối ngắn nhất có thể (< 10cm)
- Dây dài có thể gây nhiễu và giảm tốc độ SPI
- Sử dụng dây có chất lượng tốt

### 5. Tốc độ SPI (SPI Speed)
- W25Q16JV hỗ trợ lên đến 104MHz
- ESP32C3 SPI có thể chạy lên đến 80MHz
- Thư viện mặc định: 40MHz (an toàn và ổn định)
- Có thể tăng nếu dây nối ngắn và chất lượng tốt

## Sơ đồ mạch đầy đủ (Complete Circuit)

```
                          +3.3V
                            │
                    ┌───────┴───────┐
                    │               │
                  [10kΩ]          [10kΩ]
                    │               │
         ┌──────────┼───────────────┼──────────┐
         │          │               │          │
         │          │               │          │
    ┌────┴────┐   ┌─┴─┐           ┌─┴─┐     ┌──┴──┐
    │ W25Q16JV│   │C1 │           │C2 │     │ C3  │
    │         │   │100│           │100│     │ 10µ │
    │ HOLD# 7 ├───┤nF ├─┐       ┌─┤nF ├─┐ ┌─┤ F   ├─┐
    │ WP#   3 ├───┘   │ │       │ └───┘ │ │ └─────┘ │
    │         │       │ │       │       │ │         │
    │ VCC   8 ├───────┘ │       │       │ │         │
    │ GND   4 ├─────────┴───────┴───────┴─┴─────────┘
    │         │                                   GND
    │ CLK   6 ├───────── GPIO2 (ESP32C3)
    │ DO    2 ├───────── GPIO7
    │ DI    5 ├───────── GPIO6
    │ CS#   1 ├───────── GPIO10
    └─────────┘
```

**Giải thích:**
- C1: Decoupling capacitor cho W25Q16JV (100nF)
- C2: Decoupling capacitor cho ESP32C3 (100nF)
- C3: Bulk capacitor (10µF)

## Kiểm tra kết nối (Connection Testing)

### Test 1: Kiểm tra nguồn
```cpp
void setup() {
  Serial.begin(115200);
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  Serial.println("Check W25Q16JV power:");
  Serial.println("VCC pin should be ~3.3V");
  Serial.println("GND pin should be 0V");
}
```

### Test 2: Kiểm tra SPI
```cpp
#include <W25Q16JV.h>

W25Q16JV flash(10);

void setup() {
  Serial.begin(115200);
  
  if (flash.begin()) {
    Serial.println("✓ SPI communication OK!");
    uint16_t id = flash.readDeviceID();
    Serial.print("Device ID: 0x");
    Serial.println(id, HEX);
  } else {
    Serial.println("✗ SPI communication failed!");
  }
}
```

### Test 3: Kiểm tra đầy đủ
Chạy example `Milestone1_BasicID` để kiểm tra toàn bộ kết nối.

## Khắc phục sự cố (Troubleshooting)

### Vấn đề: Không đọc được ID

**Nguyên nhân có thể:**
1. Kết nối sai chân
2. Thiếu nguồn 3.3V
3. HOLD# hoặc WP# không được pull-up
4. Dây nối quá dài hoặc kém chất lượng

**Giải pháp:**
- Kiểm tra lại từng dây kết nối
- Dùng đồng hồ vạn năng đo điện áp VCC
- Thêm điện trở pull-up 10kΩ cho HOLD# và WP#
- Rút ngắn dây kết nối

### Vấn đề: Đọc được ID nhưng không ghi được

**Nguyên nhân có thể:**
1. WP# không được pull-up (bị bảo vệ ghi)
2. Software write protection đã bật
3. Sector chưa được xóa trước khi ghi

**Giải pháp:**
- Kiểm tra WP# đã nối với VCC
- Đọc Status Register kiểm tra các bit bảo vệ
- Luôn xóa (erase) trước khi ghi

### Vấn đề: Hoạt động không ổn định

**Nguyên nhân có thể:**
1. Nguồn cung cấp yếu hoặc nhiễu
2. Tốc độ SPI quá cao
3. Thiếu decoupling capacitor

**Giải pháp:**
- Thêm tụ 100nF gần chip
- Giảm tốc độ SPI xuống 20MHz hoặc 10MHz
- Sử dụng nguồn ổn định hơn

## Mạch PCB khuyến nghị (Recommended PCB Layout)

### Guidelines:
1. **Ground plane**: Sử dụng lớp đất liền mạch
2. **Trace width**: >= 0.3mm cho tín hiệu SPI
3. **Trace length**: Ngắn nhất có thể, cân bằng độ dài các tín hiệu
4. **Decoupling**: Đặt tụ 100nF càng gần VCC/GND càng tốt (< 5mm)
5. **Via**: Sử dụng via kích thước >= 0.3mm

## Module có sẵn (Pre-made Modules)

Một số module W25Q16 có sẵn trên thị trường:
- **W25Q16 Breakout Board**: Đã tích hợp pull-up và decoupling cap
- **SOIC-8 to DIP Adapter**: Cho việc prototype trên breadboard

**Ưu điểm:**
- Dễ kết nối
- Đã có mạch phụ trợ
- Phù hợp cho học tập và phát triển nhanh

## Tài liệu tham khảo (References)

- W25Q16JV Datasheet Rev. I
- ESP32-C3 Datasheet
- Application Note: SPI Flash Best Practices

---

**Cập nhật**: 2025-01-15  
**Phiên bản**: 1.0
