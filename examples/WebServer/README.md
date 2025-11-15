# Hướng Dẫn Sử Dụng Web Server

## 🌐 Tổng Quan

Web Server cho phép bạn điều khiển chip W25Q16JV Flash qua giao diện web thông qua WiFi.

## ⚙️ Cài Đặt

### 1. Cấu Hình WiFi

Mở file `WebServer.ino` và sửa thông tin WiFi của bạn:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";      // Tên WiFi
const char* WIFI_PASSWORD = "YOUR_WIFI_PASS";  // Mật khẩu WiFi
```

### 2. Upload Code

1. Mở PlatformIO
2. Chọn project `W25Q16JVS`
3. Build và Upload vào ESP32C3
4. Mở Serial Monitor (115200 baud)

### 3. Lấy Địa Chỉ IP

Sau khi ESP32C3 khởi động, Serial Monitor sẽ hiển thị:

```
╔════════════════════════════════════════╗
║        🌐 SERVER RUNNING! 🌐          ║
╚════════════════════════════════════════╝

📱 Truy cập:
   http://192.168.1.xxx
   hoặc http://w25q16jv.local
```

## 🖥️ Sử Dụng Giao Diện Web

### 1. Mở Trình Duyệt

Truy cập một trong hai địa chỉ:
- `http://192.168.1.xxx` (IP được hiển thị trong Serial)
- `http://w25q16jv.local` (mDNS - có thể không hoạt động trên mọi mạng)

### 2. Các Tab Chức Năng

#### 📊 **Tab "Thông Tin"**

Hiển thị thông tin chip tự động:
- **Device ID**: Mã nhận dạng thiết bị (0xEF14)
- **JEDEC ID**: Mã JEDEC (0xEF4015)
- **Unique ID**: ID duy nhất 64-bit
- **Dung Lượng**: 2.00 MB
- **Trạng Thái**: Ready/Busy

#### 📖 **Tab "Đọc Dữ Liệu"**

Đọc 1 byte từ địa chỉ bất kỳ:

1. Nhập địa chỉ (hex, không cần 0x)
   - Ví dụ: `1000` cho địa chỉ 0x1000
2. Click **"Đọc Byte"**
3. Kết quả hiển thị: `✓ Địa chỉ 0x1000: 0xFF (255)`

**Ví dụ:**
```
Địa chỉ: 1000
→ Kết quả: ✓ Địa chỉ 0x1000: 0xAA (170)
```

#### ✍️ **Tab "Ghi Dữ Liệu"**

Ghi 1 byte với Read-Modify-Write tự động:

1. Nhập địa chỉ (hex): `1000`
2. Nhập giá trị (hex, 00-FF): `AA`
3. Click **"Ghi Byte"**
4. Kết quả: `✓ Đã ghi 0xAA vào 0x1000`

**⚠️ Lưu ý:**
- Tự động sử dụng Read-Modify-Write
- Các byte khác trong sector KHÔNG bị ảnh hưởng
- Mất khoảng 150-200ms (do phải đọc-xóa-ghi 4KB)

**Ví dụ:**
```
Địa chỉ: 2000
Giá trị: 55
→ Ghi 0x55 vào 0x2000, các byte khác giữ nguyên
```

#### 🗑️ **Tab "Xóa"**

Xóa vùng nhớ:

1. Chọn loại xóa:
   - **Sector (4KB)**: Xóa 4096 bytes (~50-100ms)
   - **Block 32KB**: Xóa 32768 bytes (~150-200ms)
   - **Block 64KB**: Xóa 65536 bytes (~200-300ms)

2. Nhập địa chỉ (bất kỳ trong vùng): `3000`
3. Click **"Xóa"**
4. Xác nhận trong popup
5. Kết quả: `✓ Đã xóa sector tại 0x3000`

**⚠️ CẢNH BÁO:**
- Dữ liệu sẽ bị xóa VĨNH VIỄN
- Không thể hoàn tác
- Tất cả byte trong vùng = 0xFF sau khi xóa

#### 🔍 **Tab "Hex Viewer"**

Xem dữ liệu dạng Hex Dump chuyên nghiệp:

1. Nhập địa chỉ bắt đầu: `0`
2. Nhập số byte (1-512): `256`
3. Click **"Xem"**

**Định dạng hiển thị:**
```
000000: 48 65 6C 6C 6F 20 57 6F 72 6C 64 21 00 FF FF FF | Hello World!....
000010: AA BB CC DD EE FF 11 22 33 44 55 66 77 88 99 00 | ......."3DUfw...
```

- **Địa chỉ**: Màu xanh dương
- **Hex Data**: Màu cam
- **ASCII**: Màu xanh lá (ký tự không in được = `.`)

## 🔧 API REST

Web server cũng cung cấp API REST để tích hợp:

### **GET /api/info**

Lấy thông tin chip:

```bash
curl http://192.168.1.xxx/api/info
```

Response:
```json
{
  "deviceId": 61204,
  "jedecId": 15663125,
  "uniqueId": "E6681CB2976E7628",
  "capacity": 2097152,
  "busy": false
}
```

### **GET /api/read?addr=1000**

Đọc 1 byte:

```bash
curl http://192.168.1.xxx/api/read?addr=1000
```

Response:
```json
{
  "success": true,
  "value": 170
}
```

### **GET /api/write?addr=1000&value=AA**

Ghi 1 byte:

```bash
curl http://192.168.1.xxx/api/write?addr=1000&value=AA
```

Response:
```json
{
  "success": true
}
```

### **GET /api/erase?type=sector&addr=1000**

Xóa vùng nhớ:

```bash
curl http://192.168.1.xxx/api/erase?type=sector&addr=1000
```

Parameters:
- `type`: `sector` | `block32` | `block64`
- `addr`: Địa chỉ (hex)

Response:
```json
{
  "success": true
}
```

### **GET /api/hexdump?addr=0&len=256**

Lấy hex dump:

```bash
curl http://192.168.1.xxx/api/hexdump?addr=0&len=256
```

Response:
```json
{
  "success": true,
  "html": "<span class='hex-address'>000000:</span> ..."
}
```

## 🎨 Tính Năng Giao Diện

### 1. **Responsive Design**
- Tự động điều chỉnh trên mobile/tablet/desktop
- Gradient đẹp mắt
- Animations mượt mà

### 2. **Real-time Feedback**
- Loading spinner khi xử lý
- Thông báo success/error rõ ràng
- Hiển thị kết quả tức thì

### 3. **User-Friendly**
- Placeholder hướng dẫn
- Validate input
- Confirm trước khi xóa
- Tooltip giải thích

### 4. **Professional Hex Viewer**
- Màu sắc phân biệt rõ ràng
- Hiển thị cả hex và ASCII
- Địa chỉ 6 chữ số hex
- Font monospace chuyên nghiệp

## 📱 Truy Cập Từ Mobile

1. Đảm bảo điện thoại kết nối cùng WiFi với ESP32C3
2. Mở trình duyệt trên điện thoại
3. Nhập địa chỉ IP
4. Giao diện tự động responsive!

## 🔒 Bảo Mật

**⚠️ LƯU Ý AN TOÀN:**
- Web server KHÔNG có authentication
- Ai cũng có thể truy cập nếu biết IP
- KHÔNG expose ra Internet công cộng
- Chỉ dùng trong mạng local đáng tin cậy

**Để bảo mật hơn:**
1. Đổi SSID WiFi phức tạp
2. Sử dụng WiFi riêng cho IoT
3. Thêm basic authentication (nếu cần)

## 🐛 Troubleshooting

### **Không kết nối được WiFi**

Triệu chứng: Serial hiển thị "Không thể kết nối WiFi"

Giải pháp:
1. Kiểm tra SSID và password đúng
2. Đảm bảo WiFi 2.4GHz (ESP32C3 không hỗ trợ 5GHz)
3. Kiểm tra signal mạnh
4. Thử restart router

### **Không truy cập được web**

Triệu chứng: Browser hiển thị "Cannot connect"

Giải pháp:
1. Kiểm tra IP đúng
2. Ping IP từ máy tính: `ping 192.168.1.xxx`
3. Đảm bảo máy tính cùng mạng với ESP32
4. Thử tắt firewall tạm thời
5. Dùng `http://` không dùng `https://`

### **mDNS không hoạt động**

Triệu chứng: `http://w25q16jv.local` không mở được

Giải pháp:
1. Dùng IP trực tiếp thay vì mDNS
2. mDNS không hoạt động trên một số mạng doanh nghiệp
3. Windows cần cài Bonjour Service
4. Android thường không hỗ trợ mDNS

### **Ghi/Xóa bị lỗi**

Triệu chứng: "Lỗi ghi: -4" hoặc timeout

Giải pháp:
1. Kiểm tra địa chỉ < 0x1FFFFF (2MB)
2. Đảm bảo chip không busy
3. Kiểm tra kết nối phần cứng
4. HOLD# và WP# phải nối 3.3V

## 💡 Mẹo & Thủ Thuật

### **1. Bookmark trang**
Thêm bookmark trong browser để truy cập nhanh

### **2. Sử dụng API với script**
Viết script Python/Bash để tự động hóa:

```python
import requests

# Ghi nhiều byte
for addr in range(0x1000, 0x1010):
    requests.get(f'http://192.168.1.xxx/api/write?addr={addr:X}&value=AA')
```

### **3. Monitor Serial**
Mở Serial Monitor song song để debug

### **4. Backup dữ liệu**
Dùng Hex Viewer để xem và backup dữ liệu quan trọng

### **5. Test trước khi xóa**
Luôn đọc dữ liệu trước khi xóa để đảm bảo

## 📊 Hiệu Năng

- **Đọc byte**: ~10-50ms
- **Ghi byte**: ~150-200ms (Read-Modify-Write)
- **Xóa sector**: ~50-100ms
- **Hex dump 256 bytes**: ~100-200ms
- **Kết nối WiFi**: ~2-5 giây

## 🔄 Cập Nhật Firmware

1. Chỉnh sửa code trong `WebServer.ino`
2. Build lại project
3. Upload qua USB (OTA sẽ được thêm sau)
4. Kiểm tra Serial Monitor

## 📝 Ghi Chú

- Server chạy trên port 80 (HTTP)
- Hỗ trợ tối đa 4 clients đồng thời
- Timeout mỗi request: 30 giây
- Max data per request: 512 bytes (hex viewer)

## ✅ Checklist Triển Khai

- [ ] Sửa SSID và password WiFi
- [ ] Upload code thành công
- [ ] Kết nối WiFi OK
- [ ] Lấy được IP address
- [ ] Truy cập web thành công
- [ ] Test đọc/ghi/xóa
- [ ] Kiểm tra Hex Viewer
- [ ] Bookmark trang web

---

**🎉 Chúc bạn sử dụng thành công!**
