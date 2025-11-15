/*
 * W25Q16JV Web Server
 * 
 * Web interface để điều khiển SPI Flash W25Q16JV qua WiFi
 * Tính năng:
 * - Đọc/ghi byte
 * - Xóa sector/block
 * - Xem thông tin chip
 * - Hex viewer (256 bytes max)
 * - Ghi nhiều byte (256 bytes max)
 */

#include <Arduino.h>
#include <SPI.h>
#include <W25Q16JV.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

// ========== CẤU HÌNH WIFI ==========
// ESP32C3 sẽ phát WiFi AP (Access Point)
const char* AP_SSID = "W25Q16JV_Flash";     // Tên WiFi phát ra
const char* AP_PASSWORD = "Test123456";     // Mật khẩu WiFi (tối thiểu 8 ký tự)
IPAddress AP_IP(192, 168, 4, 1);            // IP của ESP32C3
IPAddress AP_GATEWAY(192, 168, 4, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

// ========== CẤU HÌNH SPI ==========
#define SCK_PIN  2
#define MISO_PIN 7
#define MOSI_PIN 6
#define CS_PIN   10
#define SPI_FREQ 10000000  // 10MHz

W25Q16JV flash(CS_PIN, SPI_FREQ);
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
            line-height: 1.8;
            white-space: pre-wrap;
            word-break: keep-all;
            overflow-wrap: normal;
        }
        .hex-address { 
            color: #569cd6; 
            font-weight: bold;
            display: inline-block;
            white-space: nowrap;
        }
        .hex-data { 
            color: #ce9178;
            letter-spacing: 1px;
            display: inline;
            white-space: nowrap;
        }
        .hex-ascii { 
            color: #4ec9b0;
            margin-left: 10px;
            display: inline;
            white-space: nowrap;
        }
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
            <button class="tab active" onclick="showTab('info')">📊 Info</button>
            <button class="tab" onclick="showTab('read')">📖 Read</button>
            <button class="tab" onclick="showTab('write')">✍️ Write</button>
            <button class="tab" onclick="showTab('erase')">🗑️ Erase</button>
            <button class="tab" onclick="showTab('hex')">🔍 Hex Viewer</button>
        </div>
)rawliteral";

const char HTML_FOOTER[] PROGMEM = R"rawliteral(
    </div>
    
    <script>
        function showTab(tabName) {
            document.querySelectorAll('.content').forEach(el => el.classList.remove('active'));
            document.querySelectorAll('.tab').forEach(el => el.classList.remove('active'));
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
        
        function readByte() {
            const addr = document.getElementById('readAddr').value;
            showLoading(true);
            
            fetch('/api/read?addr=' + addr)
                .then(r => r.json())
                .then(data => {
                    showLoading(false);
                    if (data.success) {
                        showResult('readResult', 
                            `✓ Address 0x${parseInt(addr,16).toString(16).toUpperCase()}: 0x${data.value.toString(16).toUpperCase().padStart(2,'0')} (${data.value})`);
                    } else {
                        showResult('readResult', '✗ ' + data.error, true);
                    }
                })
                .catch(e => {
                    showLoading(false);
                    showResult('readResult', '✗ Error: ' + e, true);
                });
        }
        
        function writeData() {
            const addr = document.getElementById('writeAddr').value;
            const mode = document.getElementById('writeMode').value;
            
            if (mode === 'single') {
                const value = document.getElementById('writeValue').value;
                showLoading(true);
                
                fetch('/api/write?addr=' + addr + '&value=' + value)
                    .then(r => r.json())
                    .then(data => {
                        showLoading(false);
                        if (data.success) {
                            showResult('writeResult', `✓ Wrote 0x${parseInt(value,16).toString(16).toUpperCase()} to 0x${parseInt(addr,16).toString(16).toUpperCase()}`);
                        } else {
                            showResult('writeResult', '✗ ' + data.error, true);
                        }
                    })
                    .catch(e => {
                        showLoading(false);
                        showResult('writeResult', '✗ Error: ' + e, true);
                    });
            } else {
                const dataStr = document.getElementById('writeData').value;
                const hexBytes = dataStr.replace(/\s+/g, '').match(/.{1,2}/g);
                
                if (!hexBytes || hexBytes.length === 0) {
                    showResult('writeResult', '✗ Please enter hex data!', true);
                    return;
                }
                
                if (hexBytes.length > 256) {
                    showResult('writeResult', '✗ Max 256 bytes! You entered ' + hexBytes.length + ' bytes.', true);
                    return;
                }
                
                const dataHex = hexBytes.join('');
                showLoading(true);
                
                fetch('/api/writemulti?addr=' + addr + '&data=' + dataHex)
                    .then(r => r.json())
                    .then(data => {
                        showLoading(false);
                        if (data.success) {
                            showResult('writeResult', `✓ Wrote ${hexBytes.length} bytes to 0x${parseInt(addr,16).toString(16).toUpperCase()}`);
                        } else {
                            showResult('writeResult', '✗ ' + data.error, true);
                        }
                    })
                    .catch(e => {
                        showLoading(false);
                        showResult('writeResult', '✗ Error: ' + e, true);
                    });
            }
        }
        
        function eraseMemory() {
            const type = document.getElementById('eraseType').value;
            const addr = document.getElementById('eraseAddr').value;
            
            if (!confirm('Are you sure you want to erase? Data will be lost!')) return;
            
            showLoading(true);
            
            fetch('/api/erase?type=' + type + '&addr=' + addr)
                .then(r => r.json())
                .then(data => {
                    showLoading(false);
                    if (data.success) {
                        showResult('eraseResult', `✓ Erased ${type} at 0x${parseInt(addr,16).toString(16).toUpperCase()}`);
                    } else {
                        showResult('eraseResult', '✗ ' + data.error, true);
                    }
                })
                .catch(e => {
                    showLoading(false);
                    showResult('eraseResult', '✗ Error: ' + e, true);
                });
        }
        
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
                    showResult('hexResult', '✗ ' + e, true);
                });
        }
        
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

void handleRoot();
void handleInfo();
void handleRead();
void handleWrite();
void handleWriteMulti();
void handleErase();
void handleHexDump();

void handleRoot() {
  String html = FPSTR(HTML_HEADER);
  
  html += R"rawliteral(
        <div id="info" class="content active">
            <div class="card">
                <h3>📊 Chip Information</h3>
                <div class="info-grid">
                    <div class="info-item"><label>Device ID</label><div class="value" id="deviceId">-</div></div>
                    <div class="info-item"><label>JEDEC ID</label><div class="value" id="jedecId">-</div></div>
                    <div class="info-item"><label>Unique ID</label><div class="value" id="uniqueId">-</div></div>
                    <div class="info-item"><label>Capacity</label><div class="value" id="capacity">-</div></div>
                    <div class="info-item"><label>Status</label><div class="value" id="status">-</div></div>
                </div>
            </div>
        </div>
        <div id="read" class="content">
            <div class="card">
                <h3>📖 Read Data</h3>
                <div class="form-group"><label>Address (hex)</label><input type="text" id="readAddr" value="1000"></div>
                <button class="btn btn-primary" onclick="readByte()">Read Byte</button>
                <div id="readResult" class="result"></div>
            </div>
        </div>
        <div id="write" class="content">
            <div class="card">
                <h3>✍️ Write Data</h3>
                <div class="form-group"><label>Address (hex)</label><input type="text" id="writeAddr" value="1000"></div>
                <div class="form-group"><label>Mode</label><select id="writeMode" onchange="toggleWriteMode()"><option value="single">Write 1 Byte</option><option value="multiple">Write Multiple Bytes (max 256)</option></select></div>
                <div id="singleByteForm"><div class="form-group"><label>Value (hex)</label><input type="text" id="writeValue" value="AA"></div></div>
                <div id="multipleByteForm" style="display:none;"><div class="form-group"><label>Data (hex, max 256 bytes)</label><textarea id="writeData" rows="4" style="width:100%;padding:10px;border:2px solid #ddd;border-radius:5px;font-family:monospace;"></textarea><small style="color:#666;">Example: AA BB CC DD</small></div></div>
                <button class="btn btn-primary" onclick="writeData()">Write</button>
                <div id="writeResult" class="result"></div>
                <p style="margin-top:15px;color:#666;font-size:14px;">⚠️ Single byte: R-M-W safe | Multiple: Auto erase before write</p>
            </div>
        </div>
        <div id="erase" class="content">
            <div class="card">
                <h3>🗑️ Erase</h3>
                <div class="form-group"><label>Type</label><select id="eraseType"><option value="sector">Sector (4KB)</option><option value="block32">Block 32KB</option><option value="block64">Block 64KB</option></select></div>
                <div class="form-group"><label>Address (hex)</label><input type="text" id="eraseAddr" value="1000"></div>
                <button class="btn btn-danger" onclick="eraseMemory()">Erase</button>
                <div id="eraseResult" class="result"></div>
            </div>
        </div>
        <div id="hex" class="content">
            <div class="card">
                <h3>🔍 Hex Viewer</h3>
                <div class="form-group"><label>Address (hex)</label><input type="text" id="hexAddr" value="0"></div>
                <div class="form-group"><label>Length (max 256)</label><input type="number" id="hexLen" value="256" min="1" max="256"></div>
                <button class="btn btn-primary" onclick="loadHexView()">View</button>
                <div id="hexResult" class="result"></div>
                <div id="hexOutput" class="hex-viewer" style="margin-top:20px;"></div>
            </div>
        </div>
        <div id="loading" class="loading"><div class="spinner"></div><p>Processing...</p></div>
  )rawliteral";
  
  html += FPSTR(HTML_FOOTER);
  server.send(200, "text/html", html);
}

void handleInfo() {
  uint16_t deviceId = flash.readDeviceID();
  uint32_t jedecId = flash.readJEDECID();
  uint64_t uniqueId = flash.readUniqueID();
  
  char buf[200];
  sprintf(buf, "{\"deviceId\":%d,\"jedecId\":%u,\"uniqueId\":\"%08X%08X\",\"capacity\":%u,\"busy\":%s}",
          deviceId, jedecId, (uint32_t)(uniqueId >> 32), (uint32_t)(uniqueId & 0xFFFFFFFF),
          flash.getCapacity(), flash.isBusy() ? "true" : "false");
  
  server.send(200, "application/json", buf);
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
  
  char buf[50];
  sprintf(buf, "{\"success\":true,\"value\":%d}", value);
  server.send(200, "application/json", buf);
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
    char buf[100];
    sprintf(buf, "{\"success\":false,\"error\":\"Lỗi ghi: %d\"}", result);
    server.send(200, "application/json", buf);
  }
}

void handleWriteMulti() {
  if (!server.hasArg("addr") || !server.hasArg("data")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
    return;
  }
  
  uint32_t addr = strtol(server.arg("addr").c_str(), NULL, 16);
  String dataStr = server.arg("data");
  uint16_t len = dataStr.length() / 2;
  
  if (len == 0 || len > 256) {
    server.send(200, "application/json", "{\"success\":false,\"error\":\"Độ dài không hợp lệ (1-256)\"}");
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
  
  // Tính các sector cần xử lý
  uint32_t startSector = addr / 4096;
  uint32_t endSector = (addr + len - 1) / 4096;
  
  // Xử lý từng sector
  for (uint32_t sectorNum = startSector; sectorNum <= endSector; sectorNum++) {
    uint32_t sectorAddr = sectorNum * 4096;
    uint8_t sectorBuffer[4096];
    
    // Đọc toàn bộ sector ra
    flash.readData(sectorAddr, sectorBuffer, 4096);
    
    // Merge dữ liệu mới vào sector buffer
    uint32_t writeStart = (addr > sectorAddr) ? (addr - sectorAddr) : 0;
    uint32_t dataStart = (sectorAddr > addr) ? (sectorAddr - addr) : 0;
    uint32_t writeLen = min((uint32_t)len - dataStart, 4096 - writeStart);
    
    memcpy(sectorBuffer + writeStart, buffer + dataStart, writeLen);
    
    // Erase và ghi lại toàn bộ sector
    flash.eraseSector(sectorAddr);
    flash.writeData(sectorAddr, sectorBuffer, 4096);
  }
  
  char buf[50];
  sprintf(buf, "{\"success\":true,\"length\":%d}", len);
  server.send(200, "application/json", buf);
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
  
  if (type == "sector") flash.eraseSector(addr);
  else if (type == "block32") flash.eraseBlock32K(addr);
  else if (type == "block64") flash.eraseBlock64K(addr);
  else {
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
    // Bắt đầu dòng mới (mỗi dòng 16 bytes)
    if (i % 16 == 0) {
      // In phần ASCII của dòng trước (nếu có)
      if (i > 0) {
        html += "  <span class=\\\"hex-ascii\\\">| ";
        for (int j = i - 16; j < i; j++) {
          char c = (buffer[j] >= 32 && buffer[j] <= 126) ? buffer[j] : '.';
          // Escape đặc biệt cho JSON
          if (c == '\\') html += "\\\\\\\\";
          else if (c == '"') html += "\\\\\\\"";
          else html += String(c);
        }
        html += " |</span><br>";
      }
      // In địa chỉ
      char addrStr[10];
      sprintf(addrStr, "%06X", addr + i);
      html += "<span class=\\\"hex-address\\\">" + String(addrStr) + ":</span>  ";
    }
    
    // In byte hex
    char hexStr[4];
    sprintf(hexStr, "%02X", buffer[i]);
    html += "<span class=\\\"hex-data\\\">" + String(hexStr) + "</span> ";
    
    // Thêm khoảng cách giữa 2 nhóm 8 bytes
    if (i % 16 == 7) {
      html += " ";
    }
  }
  
  // Xử lý dòng cuối cùng (nếu không đủ 16 bytes)
  int remaining = len % 16;
  if (remaining != 0) {
    // Padding để căn chỉnh phần ASCII
    for (int p = remaining; p < 16; p++) {
      html += "   ";
      if (p == 7) html += " ";
    }
  }
  
  // In phần ASCII của dòng cuối
  html += "  <span class=\\\"hex-ascii\\\">| ";
  int start = (len / 16) * 16;
  for (int j = start; j < len; j++) {
    char c = (buffer[j] >= 32 && buffer[j] <= 126) ? buffer[j] : '.';
    // Escape đặc biệt cho JSON
    if (c == '\\') html += "\\\\\\\\";
    else if (c == '"') html += "\\\\\\\"";
    else html += String(c);
  }
  html += " |</span>";
  
  server.send(200, "application/json", "{\"success\":true,\"html\":\"" + html + "\"}");
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║   W25Q16JV Web Server - ESP32C3       ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  
  Serial.println("1. Khởi tạo SPI...");
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  Serial.println("   ✓ SPI sẵn sàng\n");
  
  Serial.println("2. Khởi tạo W25Q16JV...");
  if (!flash.begin()) {
    Serial.println("   ✗ Lỗi khởi tạo flash!");
    while (1) delay(1000);
  }
  Serial.println("   ✓ Flash sẵn sàng\n");
  
  Serial.println("3. Khởi động WiFi Access Point...");
  Serial.printf("   SSID: %s\n", AP_SSID);
  Serial.printf("   Password: %s\n", AP_PASSWORD);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  
  if (!WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.println("\n   ✗ Không thể khởi động AP!");
    while (1) delay(1000);
  }
  
  delay(1000);
  
  Serial.println("\n   ✓ WiFi AP đã khởi động!\n");
  Serial.print("   IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.printf("   Kết nối WiFi: %s\n", AP_SSID);
  Serial.println("   Sau đó truy cập: http://192.168.4.1\n");
  
  Serial.println("4. Đăng ký API routes...");
  server.on("/", handleRoot);
  server.on("/api/info", handleInfo);
  server.on("/api/read", handleRead);
  server.on("/api/write", handleWrite);
  server.on("/api/writemulti", handleWriteMulti);
  server.on("/api/erase", handleErase);
  server.on("/api/hexdump", handleHexDump);
  Serial.println("   ✓ Routes sẵn sàng\n");
  
  server.begin();
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║        🌐 SERVER RUNNING! 🌐          ║");
  Serial.println("╚════════════════════════════════════════╝\n");
  Serial.print("📱 http://");
  Serial.println(WiFi.softAPIP());
  Serial.println("   hoặc http://192.168.4.1\n");
  Serial.println("📶 Kết nối WiFi AP:");
  Serial.printf("   SSID: %s\n", AP_SSID);
  Serial.printf("   Password: %s\n\n", AP_PASSWORD);
  
  uint16_t deviceId = flash.readDeviceID();
  uint32_t jedecId = flash.readJEDECID();
  Serial.println("📊 Chip:");
  Serial.printf("   Device ID: 0x%04X\n", deviceId);
  Serial.printf("   JEDEC ID: 0x%06X\n\n", jedecId);
}

void loop() {
  server.handleClient();
  delay(2);
}


