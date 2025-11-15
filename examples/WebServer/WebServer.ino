/*
 * W25Q16JV Web Server
 * 
 * Web interface để điều khiển SPI Flash W25Q16JV qua WiFi
 * Tính năng:
 * - Đọc/ghi byte
 * - Xóa sector/block
 * - Xem thông tin chip
 * - Hex viewer
 * - Upload/download file
 */

#include <Arduino.h>
#include <SPI.h>
#include <W25Q16JV.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ========== CẤU HÌNH WIFI ==========
const char* WIFI_SSID = "YOUR_WIFI_SSID";      // Thay bằng tên WiFi của bạn
const char* WIFI_PASSWORD = "YOUR_WIFI_PASS";  // Thay bằng mật khẩu WiFi

// ========== CẤU HÌNH SPI ==========
#define SCK_PIN  2
#define MISO_PIN 7
#define MOSI_PIN 6
#define CS_PIN   10

W25Q16JV flash(CS_PIN);
WebServer server(80);

// ========== HTML/CSS/JS ==========
const char HTML_HEADER[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>W25Q16JV Flash Controller</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        .header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        .header h1 { font-size: 2.5em; margin-bottom: 10px; }
        .header p { opacity: 0.9; }
        .tabs {
            display: flex;
            background: #f5f5f5;
            border-bottom: 2px solid #ddd;
        }
        .tab {
            flex: 1;
            padding: 15px;
            text-align: center;
            cursor: pointer;
            background: #f5f5f5;
            border: none;
            font-size: 16px;
            transition: all 0.3s;
        }
        .tab:hover { background: #e0e0e0; }
        .tab.active {
            background: white;
            border-bottom: 3px solid #667eea;
            font-weight: bold;
        }
        .content {
            padding: 30px;
            display: none;
        }
        .content.active { display: block; }
        .card {
            background: #f9f9f9;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            border: 1px solid #e0e0e0;
        }
        .card h3 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 1.3em;
        }
        .form-group {
            margin-bottom: 15px;
        }
        .form-group label {
            display: block;
            margin-bottom: 5px;
            font-weight: 600;
            color: #333;
        }
        .form-group input,
        .form-group select {
            width: 100%;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
        }
        .form-group input:focus,
        .form-group select:focus {
            outline: none;
            border-color: #667eea;
        }
        .btn {
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
            font-weight: 600;
            transition: all 0.3s;
            margin-right: 10px;
        }
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        .btn-danger {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
        }
        .btn-danger:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(245, 87, 108, 0.4);
        }
        .btn-success {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
        }
        .result {
            margin-top: 20px;
            padding: 15px;
            border-radius: 5px;
            display: none;
        }
        .result.success {
            background: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .result.error {
            background: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .hex-viewer {
            font-family: 'Courier New', monospace;
            background: #1e1e1e;
            color: #d4d4d4;
            padding: 20px;
            border-radius: 5px;
            overflow-x: auto;
            font-size: 14px;
            line-height: 1.6;
        }
        .hex-address { color: #569cd6; }
        .hex-data { color: #ce9178; }
        .hex-ascii { color: #4ec9b0; }
        .info-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
        }
        .info-item {
            background: white;
            padding: 15px;
            border-radius: 5px;
            border-left: 4px solid #667eea;
        }
        .info-item label {
            font-size: 12px;
            color: #666;
            text-transform: uppercase;
        }
        .info-item .value {
            font-size: 20px;
            font-weight: bold;
            color: #333;
            margin-top: 5px;
        }
        .loading {
            display: none;
            text-align: center;
            padding: 20px;
        }
        .spinner {
            border: 4px solid #f3f3f3;
            border-top: 4px solid #667eea;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🔷 W25Q16JV Flash Controller</h1>
            <p>Web Interface - ESP32C3 + SPI Flash Memory</p>
        </div>
        
        <div class="tabs">
            <button class="tab active" onclick="showTab('info')">📊 Thông Tin</button>
            <button class="tab" onclick="showTab('read')">📖 Đọc Dữ Liệu</button>
            <button class="tab" onclick="showTab('write')">✍️ Ghi Dữ Liệu</button>
            <button class="tab" onclick="showTab('erase')">🗑️ Xóa</button>
            <button class="tab" onclick="showTab('hex')">🔍 Hex Viewer</button>
        </div>
)rawliteral";

const char HTML_FOOTER[] PROGMEM = R"rawliteral(
    </div>
    
    <script>
        function showTab(tabName) {
            // Hide all contents
            document.querySelectorAll('.content').forEach(el => el.classList.remove('active'));
            document.querySelectorAll('.tab').forEach(el => el.classList.remove('active'));
            
            // Show selected
            document.getElementById(tabName).classList.add('active');
            event.target.classList.add('active');
        }
        
        function showResult(elementId, message, isError = false) {
            const result = document.getElementById(elementId);
            result.textContent = message;
            result.className = 'result ' + (isError ? 'error' : 'success');
            result.style.display = 'block';
        }
        
        function toggleWriteMode() {
            const mode = document.getElementById('writeMode').value;
            if (mode === 'single') {
                document.getElementById('singleByteForm').style.display = 'block';
                document.getElementById('multipleByteForm').style.display = 'none';
            } else {
                document.getElementById('singleByteForm').style.display = 'none';
                document.getElementById('multipleByteForm').style.display = 'block';
            }
        }
        
        function showLoading(show) {
            document.getElementById('loading').style.display = show ? 'block' : 'none';
        }
        
        // Đọc byte
        function readByte() {
            const addr = document.getElementById('readAddr').value;
            showLoading(true);
            
            fetch('/api/read?addr=' + addr)
                .then(r => r.json())
                .then(data => {
                    showLoading(false);
                    if (data.success) {
                        showResult('readResult', 
                            `✓ Địa chỉ 0x${parseInt(addr,16).toString(16).toUpperCase()}: 0x${data.value.toString(16).toUpperCase().padStart(2,'0')} (${data.value})`);
                    } else {
                        showResult('readResult', '✗ ' + data.error, true);
                    }
                })
                .catch(e => {
                    showLoading(false);
                    showResult('readResult', '✗ Lỗi: ' + e, true);
                });
        }
        
        // Ghi dữ liệu (1 byte hoặc nhiều byte)
        function writeData() {
            const addr = document.getElementById('writeAddr').value;
            const mode = document.getElementById('writeMode').value;
            
            if (mode === 'single') {
                // Ghi 1 byte
                const value = document.getElementById('writeValue').value;
                showLoading(true);
                
                fetch('/api/write?addr=' + addr + '&value=' + value)
                    .then(r => r.json())
                    .then(data => {
                        showLoading(false);
                        if (data.success) {
                            showResult('writeResult', `✓ Đã ghi 0x${parseInt(value,16).toString(16).toUpperCase()} vào 0x${parseInt(addr,16).toString(16).toUpperCase()}`);
                        } else {
                            showResult('writeResult', '✗ ' + data.error, true);
                        }
                    })
                    .catch(e => {
                        showLoading(false);
                        showResult('writeResult', '✗ Lỗi: ' + e, true);
                    });
            } else {
                // Ghi nhiều byte
                const dataStr = document.getElementById('writeData').value;
                const hexBytes = dataStr.replace(/\s+/g, '').match(/.{1,2}/g);
                
                if (!hexBytes || hexBytes.length === 0) {
                    showResult('writeResult', '✗ Vui lòng nhập dữ liệu hex!', true);
                    return;
                }
                
                if (hexBytes.length > 256) {
                    showResult('writeResult', '✗ Tối đa 256 bytes! Bạn nhập ' + hexBytes.length + ' bytes.', true);
                    return;
                }
                
                const dataHex = hexBytes.join('');
                showLoading(true);
                
                fetch('/api/writemulti?addr=' + addr + '&data=' + dataHex)
                    .then(r => r.json())
                    .then(data => {
                        showLoading(false);
                        if (data.success) {
                            showResult('writeResult', `✓ Đã ghi ${hexBytes.length} bytes vào 0x${parseInt(addr,16).toString(16).toUpperCase()}`);
                        } else {
                            showResult('writeResult', '✗ ' + data.error, true);
                        }
                    })
                    .catch(e => {
                        showLoading(false);
                        showResult('writeResult', '✗ Lỗi: ' + e, true);
                    });
            }
        }
        
        // Xóa
        function eraseMemory() {
            const type = document.getElementById('eraseType').value;
            const addr = document.getElementById('eraseAddr').value;
            
            if (!confirm('Bạn chắc chắn muốn xóa? Dữ liệu sẽ mất!')) return;
            
            showLoading(true);
            
            fetch('/api/erase?type=' + type + '&addr=' + addr)
                .then(r => r.json())
                .then(data => {
                    showLoading(false);
                    if (data.success) {
                        showResult('eraseResult', `✓ Đã xóa ${type} tại 0x${parseInt(addr,16).toString(16).toUpperCase()}`);
                    } else {
                        showResult('eraseResult', '✗ ' + data.error, true);
                    }
                })
                .catch(e => {
                    showLoading(false);
                    showResult('eraseResult', '✗ Lỗi: ' + e, true);
                });
        }
        
        // Hex viewer
        function loadHexView() {
            const addr = document.getElementById('hexAddr').value;
            const len = document.getElementById('hexLen').value;
            showLoading(true);
            
            fetch('/api/hexdump?addr=' + addr + '&len=' + len)
                .then(r => r.json())
                .then(data => {
                    showLoading(false);
                    if (data.success) {
                        document.getElementById('hexOutput').innerHTML = data.html;
                    } else {
                        showResult('hexResult', '✗ ' + data.error, true);
                    }
                })
                .catch(e => {
                    showLoading(false);
                    showResult('hexResult', '✗ Lỗi: ' + e, true);
                });
        }
        
        // Load info khi trang load
        window.onload = function() {
            fetch('/api/info')
                .then(r => r.json())
                .then(data => {
                    document.getElementById('deviceId').textContent = '0x' + data.deviceId.toString(16).toUpperCase();
                    document.getElementById('jedecId').textContent = '0x' + data.jedecId.toString(16).toUpperCase().padStart(6,'0');
                    document.getElementById('uniqueId').textContent = data.uniqueId;
                    document.getElementById('capacity').textContent = (data.capacity / 1024 / 1024).toFixed(2) + ' MB';
                    document.getElementById('status').textContent = data.busy ? 'Busy' : 'Ready';
                });
        };
    </script>
</body>
</html>
)rawliteral";

// ========== API HANDLERS ==========

void handleRoot() {
  String html = FPSTR(HTML_HEADER);
  
  // Tab: Thông tin
  html += R"rawliteral(
        <div id="info" class="content active">
            <div class="card">
                <h3>📊 Thông Tin Chip</h3>
                <div class="info-grid">
                    <div class="info-item">
                        <label>Device ID</label>
                        <div class="value" id="deviceId">-</div>
                    </div>
                    <div class="info-item">
                        <label>JEDEC ID</label>
                        <div class="value" id="jedecId">-</div>
                    </div>
                    <div class="info-item">
                        <label>Unique ID</label>
                        <div class="value" id="uniqueId">-</div>
                    </div>
                    <div class="info-item">
                        <label>Dung Lượng</label>
                        <div class="value" id="capacity">-</div>
                    </div>
                    <div class="info-item">
                        <label>Trạng Thái</label>
                        <div class="value" id="status">-</div>
                    </div>
                </div>
            </div>
        </div>
  )rawliteral";
  
  // Tab: Đọc
  html += R"rawliteral(
        <div id="read" class="content">
            <div class="card">
                <h3>📖 Đọc Dữ Liệu</h3>
                <div class="form-group">
                    <label>Địa chỉ (hex, ví dụ: 1000)</label>
                    <input type="text" id="readAddr" placeholder="0x1000" value="1000">
                </div>
                <button class="btn btn-primary" onclick="readByte()">Đọc Byte</button>
                <div id="readResult" class="result"></div>
            </div>
        </div>
  )rawliteral";
  
  // Tab: Ghi
  html += R"rawliteral(
        <div id="write" class="content">
            <div class="card">
                <h3>✍️ Ghi Dữ Liệu</h3>
                <div class="form-group">
                    <label>Địa chỉ (hex)</label>
                    <input type="text" id="writeAddr" placeholder="0x1000" value="1000">
                </div>
                <div class="form-group">
                    <label>Chế độ ghi</label>
                    <select id="writeMode" onchange="toggleWriteMode()">
                        <option value="single">Ghi 1 Byte</option>
                        <option value="multiple">Ghi Nhiều Byte (tối đa 256)</option>
                    </select>
                </div>
                <div id="singleByteForm">
                    <div class="form-group">
                        <label>Giá trị (hex, 00-FF)</label>
                        <input type="text" id="writeValue" placeholder="0xAA" value="AA">
                    </div>
                </div>
                <div id="multipleByteForm" style="display:none;">
                    <div class="form-group">
                        <label>Dữ liệu (hex, cách nhau bởi dấu cách, tối đa 256 bytes)</label>
                        <textarea id="writeData" placeholder="AA BB CC DD EE FF..." rows="4" style="width:100%;padding:10px;border:2px solid #ddd;border-radius:5px;font-family:monospace;"></textarea>
                        <small style="color:#666;">Ví dụ: AA BB CC DD hoặc AABBCCDD (tối đa 256 bytes)</small>
                    </div>
                </div>
                <button class="btn btn-primary" onclick="writeData()">Ghi Dữ Liệu</button>
                <div id="writeResult" class="result"></div>
                <p style="margin-top:15px; color:#666; font-size:14px;">
                    ⚠️ <strong>Ghi 1 byte:</strong> Sử dụng Read-Modify-Write (không ảnh hưởng byte khác)<br>
                    ⚠️ <strong>Ghi nhiều byte:</strong> Cần xóa sector trước (dữ liệu cũ sẽ mất)
                </p>
            </div>
        </div>
  )rawliteral";
  
  // Tab: Xóa
  html += R"rawliteral(
        <div id="erase" class="content">
            <div class="card">
                <h3>🗑️ Xóa Dữ Liệu</h3>
                <div class="form-group">
                    <label>Loại xóa</label>
                    <select id="eraseType">
                        <option value="sector">Sector (4KB)</option>
                        <option value="block32">Block 32KB</option>
                        <option value="block64">Block 64KB</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Địa chỉ (hex)</label>
                    <input type="text" id="eraseAddr" placeholder="0x1000" value="1000">
                </div>
                <button class="btn btn-danger" onclick="eraseMemory()">Xóa</button>
                <div id="eraseResult" class="result"></div>
                <p style="margin-top:15px; color:#d9534f; font-size:14px;">
                    ⚠️ CẢNH BÁO: Dữ liệu sẽ bị xóa vĩnh viễn! Thao tác không thể hoàn tác.
                </p>
            </div>
        </div>
  )rawliteral";
  
  // Tab: Hex Viewer
  html += R"rawliteral(
        <div id="hex" class="content">
            <div class="card">
                <h3>🔍 Hex Viewer</h3>
                <div class="form-group">
                    <label>Địa chỉ bắt đầu (hex)</label>
                    <input type="text" id="hexAddr" placeholder="0x0000" value="0">
                </div>
                <div class="form-group">
                    <label>Số byte (tối đa 256 - hiển thị 16 hàng x 16 bytes)</label>
                    <input type="number" id="hexLen" value="256" min="1" max="256">
                </div>
                <button class="btn btn-primary" onclick="loadHexView()">Xem</button>
                <div id="hexResult" class="result"></div>
                <div id="hexOutput" class="hex-viewer" style="margin-top:20px;"></div>
            </div>
        </div>
        
        <div id="loading" class="loading">
            <div class="spinner"></div>
            <p style="margin-top:10px;">Đang xử lý...</p>
        </div>
  )rawliteral";
  
  html += FPSTR(HTML_FOOTER);
  
  server.send(200, "text/html", html);
}

void handleInfo() {
  uint16_t deviceId = flash.readDeviceID();
  uint32_t jedecId = flash.readJEDECID();
  uint64_t uniqueId = flash.readUniqueID();
  uint32_t capacity = flash.getCapacity();
  bool busy = flash.isBusy();
  
  char uniqueIdStr[20];
  sprintf(uniqueIdStr, "%08X%08X", (uint32_t)(uniqueId >> 32), (uint32_t)(uniqueId & 0xFFFFFFFF));
  
  String json = "{";
  json += "\"deviceId\":" + String(deviceId) + ",";
  json += "\"jedecId\":" + String(jedecId) + ",";
  json += "\"uniqueId\":\"" + String(uniqueIdStr) + "\",";
  json += "\"capacity\":" + String(capacity) + ",";
  json += "\"busy\":" + String(busy ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleRead() {
  if (!server.hasArg("addr")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing addr\"}");
    return;
  }
  
  uint32_t addr = strtol(server.arg("addr").c_str(), NULL, 16);
  
  if (!flash.isValidAddress(addr)) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Địa chỉ không hợp lệ\"}");
    return;
  }
  
  uint8_t value = flash.readByte(addr);
  
  String json = "{\"success\":true,\"value\":" + String(value) + "}";
  server.send(200, "application/json", json);
}

void handleWrite() {
  if (!server.hasArg("addr") || !server.hasArg("value")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
    return;
  }
  
  uint32_t addr = strtol(server.arg("addr").c_str(), NULL, 16);
  uint8_t value = strtol(server.arg("value").c_str(), NULL, 16);
  
  if (!flash.isValidAddress(addr)) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Địa chỉ không hợp lệ\"}");
    return;
  }
  
  int result = flash.writeByte(addr, value);
  
  if (result == 0) {
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    String json = "{\"success\":false,\"error\":\"Lỗi ghi: " + String(result) + "\"}";
    server.send(200, "application/json", json);
  }
}

void handleWriteMulti() {
  if (!server.hasArg("addr") || !server.hasArg("data")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
    return;
  }
  
  uint32_t addr = strtol(server.arg("addr").c_str(), NULL, 16);
  String dataStr = server.arg("data");
  
  // Chuyển hex string thành byte array
  uint16_t len = dataStr.length() / 2;
  
  if (len == 0 || len > 256) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Độ dài không hợp lệ (1-256 bytes)\"}");
    return;
  }
  
  if (!flash.isValidAddress(addr)) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Địa chỉ không hợp lệ\"}");
    return;
  }
  
  uint8_t buffer[256];
  for (uint16_t i = 0; i < len; i++) {
    String byteStr = dataStr.substring(i * 2, i * 2 + 2);
    buffer[i] = strtol(byteStr.c_str(), NULL, 16);
  }
  
  // Ghi dữ liệu
  flash.writeData(addr, buffer, len);
  
  String json = "{\"success\":true,\"length\":" + String(len) + "}";
  server.send(200, "application/json", json);
}

void handleErase() {
  if (!server.hasArg("type") || !server.hasArg("addr")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
    return;
  }
  
  String type = server.arg("type");
  uint32_t addr = strtol(server.arg("addr").c_str(), NULL, 16);
  
  if (!flash.isValidAddress(addr)) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Địa chỉ không hợp lệ\"}");
    return;
  }
  
  if (type == "sector") {
    flash.eraseSector(addr);
  } else if (type == "block32") {
    flash.eraseBlock32K(addr);
  } else if (type == "block64") {
    flash.eraseBlock64K(addr);
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid type\"}");
    return;
  }
  
  server.send(200, "application/json", "{\"success\":true}");
}

void handleHexDump() {
  if (!server.hasArg("addr") || !server.hasArg("len")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
    return;
  }
  
  uint32_t addr = strtol(server.arg("addr").c_str(), NULL, 16);
  uint16_t len = server.arg("len").toInt();
  
  if (len > 256) len = 256;
  if (!flash.isValidAddress(addr)) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Địa chỉ không hợp lệ\"}");
    return;
  }
  
  uint8_t buffer[256];
  flash.readData(addr, buffer, len);
  
  String html = "";
  for (uint16_t i = 0; i < len; i++) {
    if (i % 16 == 0) {
      if (i > 0) {
        // ASCII
        html += " <span class='hex-ascii'>| ";
        for (int j = i - 16; j < i; j++) {
          char c = (buffer[j] >= 32 && buffer[j] <= 126) ? buffer[j] : '.';
          html += String(c);
        }
        html += "</span>";
      }
      html += "\n<span class='hex-address'>";
      char addrStr[10];
      sprintf(addrStr, "%06X", addr + i);
      html += String(addrStr) + ":</span> ";
    }
    
    char hexStr[4];
    sprintf(hexStr, "%02X ", buffer[i]);
    html += "<span class='hex-data'>" + String(hexStr) + "</span>";
  }
  
  // Last line ASCII
  if (len % 16 != 0) {
    for (int p = 0; p < (16 - (len % 16)); p++) {
      html += "   ";
    }
  }
  html += " <span class='hex-ascii'>| ";
  int start = (len / 16) * 16;
  for (int j = start; j < len; j++) {
    char c = (buffer[j] >= 32 && buffer[j] <= 126) ? buffer[j] : '.';
    html += String(c);
  }
  html += "</span>";
  
  String json = "{\"success\":true,\"html\":\"" + html + "\"}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   W25Q16JV Web Server - ESP32C3       ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  // Khởi tạo SPI
  Serial.println("1. Khởi tạo SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("   ✓ SPI đã sẵn sàng\n");
  
  // Khởi tạo Flash
  Serial.println("2. Khởi tạo W25Q16JV...");
  if (!flash.begin()) {
    Serial.println("   ✗ Không thể khởi tạo flash!");
    while (1) delay(1000);
  }
  Serial.println("   ✓ Flash đã sẵn sàng\n");
  
  // Kết nối WiFi
  Serial.println("3. Kết nối WiFi...");
  Serial.printf("   SSID: %s\n", WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n   ✗ Không thể kết nối WiFi!");
    Serial.println("   → Kiểm tra SSID và password trong code");
    while (1) delay(1000);
  }
  
  Serial.println("\n   ✓ Đã kết nối WiFi!\n");
  Serial.print("   IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("   Signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm\n");
  
  // Khởi động mDNS
  if (MDNS.begin("w25q16jv")) {
    Serial.println("4. mDNS responder started");
    Serial.println("   Access: http://w25q16jv.local\n");
  }
  
  // Đăng ký routes
  Serial.println("5. Đăng ký API routes...");
  server.on("/", handleRoot);
  server.on("/api/info", handleInfo);
  server.on("/api/read", handleRead);
  server.on("/api/write", handleWrite);
  server.on("/api/writemulti", handleWriteMulti);
  server.on("/api/erase", handleErase);
  server.on("/api/hexdump", handleHexDump);
  Serial.println("   ✓ Routes đã sẵn sàng\n");
  
  // Start server
  server.begin();
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║        🌐 SERVER RUNNING! 🌐          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  Serial.println("📱 Truy cập:");
  Serial.print("   http://");
  Serial.println(WiFi.localIP());
  Serial.println("   hoặc http://w25q16jv.local\n");
  
  // Hiển thị info
  uint16_t deviceId = flash.readDeviceID();
  uint32_t jedecId = flash.readJEDECID();
  Serial.println("📊 Chip Info:");
  Serial.printf("   Device ID: 0x%04X\n", deviceId);
  Serial.printf("   JEDEC ID: 0x%06X\n", jedecId);
  Serial.printf("   Capacity: %.2f MB\n\n", flash.getCapacity() / 1048576.0);
}

void loop() {
  server.handleClient();
  delay(2);
}
