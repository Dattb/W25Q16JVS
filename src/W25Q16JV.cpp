#include "W25Q16JV.h"

// Constructor
W25Q16JV::W25Q16JV(uint8_t cs_pin, uint32_t spi_freq) {
    _cs_pin = cs_pin;
    _spi_freq = spi_freq;
    _spi = nullptr;
    _detected_capacity = W25Q16JV_MEMORY_SIZE;  // Default 2MB
    _sector_buffer = nullptr;
    _buffer_allocated = false;
}

// ========== Initialization ==========

bool W25Q16JV::begin(SPIClass* spi) {
    _spi = spi;
    
    // Configure CS pin
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin, HIGH);
    
    // Initialize SPI
    _spi->begin();
    
    // Release from power-down mode
    releasePowerDown();
    delayMicroseconds(3);
    
    // Read JEDEC ID to detect chip capacity
    uint32_t jedecId = readJEDECID();
    uint8_t capacityCode = jedecId & 0xFF;
    
    // Capacity code format: 2^capacityCode bytes
    // W25Q16: 0x15 = 2^21 = 2MB
    // W25Q32: 0x16 = 2^22 = 4MB
    // W25Q64: 0x17 = 2^23 = 8MB
    // W25Q128: 0x18 = 2^24 = 16MB
    if (capacityCode >= 0x14 && capacityCode <= 0x19) {
        _detected_capacity = 1UL << capacityCode;
    } else {
        _detected_capacity = W25Q16JV_MEMORY_SIZE;  // Default 2MB
    }
    
    // Verify chip connection by reading device ID
    uint16_t id = readDeviceID();
    
    // W25Q16JV/W25Q32JVS should return 0xEF14/0xEF15 or reversed
    return ((id >> 8) == 0xEF || (id & 0xFF) == 0xEF);
}

// ========== Milestone 1: Device Information ==========

uint16_t W25Q16JV::readDeviceID() {
    _select();
    _sendCommand(W25Q16JV_CMD_READ_DEVICE_ID);
    
    // Send 3 address bytes (0x000000)
    _transfer(0x00);
    _transfer(0x00);
    _transfer(0x00);
    
    // Read manufacturer and device ID
    uint8_t mfg = _transfer(0x00);
    uint8_t dev = _transfer(0x00);
    
    _deselect();
    
    return (mfg << 8) | dev;
}

uint32_t W25Q16JV::readJEDECID() {
    _select();
    _sendCommand(W25Q16JV_CMD_READ_JEDEC_ID);
    
    uint8_t mfg = _transfer(0x00);
    uint8_t type = _transfer(0x00);
    uint8_t capacity = _transfer(0x00);
    
    _deselect();
    
    return ((uint32_t)mfg << 16) | ((uint32_t)type << 8) | capacity;
}

uint64_t W25Q16JV::readUniqueID() {
    _select();
    _sendCommand(W25Q16JV_CMD_READ_UNIQUE_ID);
    
    // Send 4 dummy bytes
    _transfer(0x00);
    _transfer(0x00);
    _transfer(0x00);
    _transfer(0x00);
    
    // Read 8 bytes unique ID
    uint64_t uniqueID = 0;
    for (int i = 0; i < 8; i++) {
        uniqueID = (uniqueID << 8) | _transfer(0x00);
    }
    
    _deselect();
    return uniqueID;
}

// ========== Milestone 2: Status Register Operations ==========

uint8_t W25Q16JV::readStatusReg1() {
    _select();
    _sendCommand(W25Q16JV_CMD_READ_STATUS_REG1);
    uint8_t status = _transfer(0x00);
    _deselect();
    return status;
}

uint8_t W25Q16JV::readStatusReg2() {
    _select();
    _sendCommand(W25Q16JV_CMD_READ_STATUS_REG2);
    uint8_t status = _transfer(0x00);
    _deselect();
    return status;
}

uint8_t W25Q16JV::readStatusReg3() {
    _select();
    _sendCommand(W25Q16JV_CMD_READ_STATUS_REG3);
    uint8_t status = _transfer(0x00);
    _deselect();
    return status;
}

void W25Q16JV::writeStatusReg1(uint8_t data) {
    writeEnable();
    _select();
    _sendCommand(W25Q16JV_CMD_WRITE_STATUS_REG1);
    _transfer(data);
    _deselect();
    _waitBusy();
}

void W25Q16JV::writeStatusReg2(uint8_t data) {
    writeEnable();
    _select();
    _sendCommand(W25Q16JV_CMD_WRITE_STATUS_REG2);
    _transfer(data);
    _deselect();
    _waitBusy();
}

bool W25Q16JV::isBusy() {
    return (readStatusReg1() & W25Q16JV_STATUS_BUSY) != 0;
}

// ========== Milestone 3: Read Operations ==========

uint8_t W25Q16JV::readByte(uint32_t addr) {
    if (!isValidAddress(addr)) return 0xFF;
    
    uint8_t data;
    readData(addr, &data, 1);
    return data;
}

void W25Q16JV::readData(uint32_t addr, uint8_t* buffer, uint32_t len) {
    if (!isValidAddress(addr)) return;
    
    _select();
    _sendCommand(W25Q16JV_CMD_READ_DATA);
    _sendAddress(addr);
    
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = _transfer(0x00);
    }
    
    _deselect();
}

void W25Q16JV::fastRead(uint32_t addr, uint8_t* buffer, uint32_t len) {
    if (!isValidAddress(addr)) return;
    
    _select();
    _sendCommand(W25Q16JV_CMD_FAST_READ);
    _sendAddress(addr);
    _transfer(0x00);  // Dummy byte
    
    for (uint32_t i = 0; i < len; i++) {
        buffer[i] = _transfer(0x00);
    }
    
    _deselect();
}

// ========== Milestone 4: Erase Operations ==========

void W25Q16JV::eraseSector(uint32_t addr) {
    if (!isValidAddress(addr)) return;
    
    _waitBusy();
    writeEnable();
    
    _select();
    _sendCommand(W25Q16JV_CMD_SECTOR_ERASE);
    _sendAddress(addr);
    _deselect();
    
    _waitBusy();
}

void W25Q16JV::eraseBlock32K(uint32_t addr) {
    if (!isValidAddress(addr)) return;
    
    _waitBusy();
    writeEnable();
    
    _select();
    _sendCommand(W25Q16JV_CMD_BLOCK_ERASE_32K);
    _sendAddress(addr);
    _deselect();
    
    _waitBusy();
}

void W25Q16JV::eraseBlock64K(uint32_t addr) {
    if (!isValidAddress(addr)) return;
    
    _waitBusy();
    writeEnable();
    
    _select();
    _sendCommand(W25Q16JV_CMD_BLOCK_ERASE_64K);
    _sendAddress(addr);
    _deselect();
    
    _waitBusy();
}

void W25Q16JV::eraseChip() {
    _waitBusy();
    writeEnable();
    
    _select();
    _sendCommand(W25Q16JV_CMD_CHIP_ERASE);
    _deselect();
    
    _waitBusy();  // This can take 5-20 seconds!
}

// ========== Milestone 5: Write Operations ==========

void W25Q16JV::writeEnable() {
    _select();
    _sendCommand(W25Q16JV_CMD_WRITE_ENABLE);
    _deselect();
    delayMicroseconds(1);
}

void W25Q16JV::writeDisable() {
    _select();
    _sendCommand(W25Q16JV_CMD_WRITE_DISABLE);
    _deselect();
}

void W25Q16JV::writePage(uint32_t addr, const uint8_t* buffer, uint16_t len) {
    if (!isValidAddress(addr)) return;
    
    // Limit to page size
    if (len > W25Q16JV_PAGE_SIZE) {
        len = W25Q16JV_PAGE_SIZE;
    }
    
    // Check page boundary
    uint16_t page_offset = addr & 0xFF;
    if (page_offset + len > W25Q16JV_PAGE_SIZE) {
        len = W25Q16JV_PAGE_SIZE - page_offset;
    }
    
    _waitBusy();
    writeEnable();
    
    _select();
    _sendCommand(W25Q16JV_CMD_PAGE_PROGRAM);
    _sendAddress(addr);
    
    for (uint16_t i = 0; i < len; i++) {
        _transfer(buffer[i]);
    }
    
    _deselect();
    _waitBusy();
}

void W25Q16JV::writeData(uint32_t addr, const uint8_t* buffer, uint32_t len) {
    if (!isValidAddress(addr)) return;
    
    uint32_t written = 0;
    
    while (written < len) {
        uint16_t page_offset = (addr + written) & 0xFF;
        uint16_t chunk = min((uint32_t)(len - written), (uint32_t)(W25Q16JV_PAGE_SIZE - page_offset));
        
        writePage(addr + written, buffer + written, chunk);
        written += chunk;
        
        // Small delay between page writes
        if (written < len) {
            delay(5);
        }
    }
}

// ========== Milestone 6: Advanced Byte Operations ==========

int W25Q16JV::writeByte(uint32_t addr, uint8_t data) {
    if (!isValidAddress(addr)) {
        return W25Q_ERROR_INVALID_ADDRESS;
    }
    
    // Allocate sector buffer if needed
    if (!_allocateBuffer()) {
        return W25Q_ERROR_INIT;
    }
    
    // Calculate sector start address
    uint32_t sector_start = _getSectorAddress(addr);
    uint32_t offset = addr - sector_start;
    
    // Read entire sector (4KB)
    readData(sector_start, _sector_buffer, W25Q16JV_SECTOR_SIZE);
    
    // Check if byte needs to be changed
    if (_sector_buffer[offset] == data) {
        return W25Q_OK;  // No change needed
    }
    
    // Modify the byte
    _sector_buffer[offset] = data;
    
    // Erase the sector
    eraseSector(sector_start);
    
    // Write back the entire sector
    writeData(sector_start, _sector_buffer, W25Q16JV_SECTOR_SIZE);
    
    // Verify
    uint8_t verify = readByte(addr);
    if (verify != data) {
        return W25Q_ERROR_VERIFY_FAILED;
    }
    
    return W25Q_OK;
}

int W25Q16JV::modifyBytes(uint32_t addr, const uint8_t* data, uint16_t len) {
    if (!isValidAddress(addr)) {
        return W25Q_ERROR_INVALID_ADDRESS;
    }
    
    if (len == 0 || len > W25Q16JV_SECTOR_SIZE) {
        return W25Q_ERROR_INVALID_LENGTH;
    }
    
    // Allocate sector buffer if needed
    if (!_allocateBuffer()) {
        return W25Q_ERROR_INIT;
    }
    
    // Calculate sector start address
    uint32_t sector_start = _getSectorAddress(addr);
    uint32_t offset = addr - sector_start;
    
    // Check if modification stays within one sector
    if (offset + len > W25Q16JV_SECTOR_SIZE) {
        len = W25Q16JV_SECTOR_SIZE - offset;
    }
    
    // Read entire sector
    readData(sector_start, _sector_buffer, W25Q16JV_SECTOR_SIZE);
    
    // Modify bytes
    for (uint16_t i = 0; i < len; i++) {
        _sector_buffer[offset + i] = data[i];
    }
    
    // Erase sector
    eraseSector(sector_start);
    
    // Write back entire sector
    writeData(sector_start, _sector_buffer, W25Q16JV_SECTOR_SIZE);
    
    // Verify
    for (uint16_t i = 0; i < len; i++) {
        uint8_t verify = readByte(addr + i);
        if (verify != data[i]) {
            return W25Q_ERROR_VERIFY_FAILED;
        }
    }
    
    return W25Q_OK;
}

// ========== Power Management ==========

void W25Q16JV::powerDown() {
    _select();
    _sendCommand(W25Q16JV_CMD_POWER_DOWN);
    _deselect();
    delayMicroseconds(3);
}

void W25Q16JV::releasePowerDown() {
    _select();
    _sendCommand(W25Q16JV_CMD_RELEASE_POWER_DOWN);
    _deselect();
    delayMicroseconds(3);
}

// ========== Utility Functions ==========

bool W25Q16JV::isValidAddress(uint32_t addr) {
    return addr < _detected_capacity;
}

// ========== Private Functions ==========

void W25Q16JV::_select() {
    _spi->beginTransaction(SPISettings(_spi_freq, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs_pin, LOW);
}

void W25Q16JV::_deselect() {
    digitalWrite(_cs_pin, HIGH);
    _spi->endTransaction();
}

uint8_t W25Q16JV::_transfer(uint8_t data) {
    return _spi->transfer(data);
}

void W25Q16JV::_sendCommand(uint8_t cmd) {
    _transfer(cmd);
}

void W25Q16JV::_sendAddress(uint32_t addr) {
    _transfer((addr >> 16) & 0xFF);
    _transfer((addr >> 8) & 0xFF);
    _transfer(addr & 0xFF);
}

void W25Q16JV::_waitBusy() {
    while (isBusy()) {
        delayMicroseconds(100);
    }
}

bool W25Q16JV::_waitBusyTimeout(uint32_t timeout_ms) {
    uint32_t start = millis();
    while (isBusy()) {
        if (millis() - start > timeout_ms) {
            return false;  // Timeout
        }
        delayMicroseconds(100);
    }
    return true;
}

uint32_t W25Q16JV::_getSectorAddress(uint32_t addr) {
    return addr & 0xFFFFF000;  // Align to 4KB boundary
}

uint32_t W25Q16JV::_getPageAddress(uint32_t addr) {
    return addr & 0xFFFFFF00;  // Align to 256-byte boundary
}

bool W25Q16JV::_allocateBuffer() {
    if (!_buffer_allocated) {
        _sector_buffer = (uint8_t*)malloc(W25Q16JV_SECTOR_SIZE);
        if (_sector_buffer == nullptr) {
            return false;
        }
        _buffer_allocated = true;
    }
    return true;
}

void W25Q16JV::_freeBuffer() {
    if (_buffer_allocated) {
        free(_sector_buffer);
        _sector_buffer = nullptr;
        _buffer_allocated = false;
    }
}
