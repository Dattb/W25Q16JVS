#ifndef W25Q16JV_H
#define W25Q16JV_H

#include <Arduino.h>
#include <SPI.h>

// W25Q16JV Commands
#define W25Q16JV_CMD_WRITE_ENABLE           0x06
#define W25Q16JV_CMD_WRITE_DISABLE          0x04
#define W25Q16JV_CMD_READ_STATUS_REG1       0x05
#define W25Q16JV_CMD_READ_STATUS_REG2       0x35
#define W25Q16JV_CMD_READ_STATUS_REG3       0x15
#define W25Q16JV_CMD_WRITE_STATUS_REG1      0x01
#define W25Q16JV_CMD_WRITE_STATUS_REG2      0x31
#define W25Q16JV_CMD_WRITE_STATUS_REG3      0x11
#define W25Q16JV_CMD_PAGE_PROGRAM           0x02
#define W25Q16JV_CMD_SECTOR_ERASE           0x20
#define W25Q16JV_CMD_BLOCK_ERASE_32K        0x52
#define W25Q16JV_CMD_BLOCK_ERASE_64K        0xD8
#define W25Q16JV_CMD_CHIP_ERASE             0xC7
#define W25Q16JV_CMD_READ_DATA              0x03
#define W25Q16JV_CMD_FAST_READ              0x0B
#define W25Q16JV_CMD_RELEASE_POWER_DOWN     0xAB
#define W25Q16JV_CMD_POWER_DOWN             0xB9
#define W25Q16JV_CMD_READ_DEVICE_ID         0x90
#define W25Q16JV_CMD_READ_JEDEC_ID          0x9F
#define W25Q16JV_CMD_READ_UNIQUE_ID         0x4B

// Memory specifications
#define W25Q16JV_MEMORY_SIZE                0x200000  // 2MB
#define W25Q16JV_PAGE_SIZE                  256
#define W25Q16JV_SECTOR_SIZE                4096
#define W25Q16JV_BLOCK_SIZE_32K             32768
#define W25Q16JV_BLOCK_SIZE_64K             65536

// Status Register bits
#define W25Q16JV_STATUS_BUSY                0x01
#define W25Q16JV_STATUS_WEL                 0x02

// Error codes
enum W25Q16JV_Error {
    W25Q_OK = 0,
    W25Q_ERROR_INIT = -1,
    W25Q_ERROR_BUSY_TIMEOUT = -2,
    W25Q_ERROR_VERIFY_FAILED = -3,
    W25Q_ERROR_INVALID_ADDRESS = -4,
    W25Q_ERROR_INVALID_LENGTH = -5
};

class W25Q16JV {
public:
    /**
     * @brief Constructor
     * @param cs_pin Chip Select pin number
     * @param spi_freq SPI frequency in Hz (default: 4MHz)
     */
    W25Q16JV(uint8_t cs_pin, uint32_t spi_freq = 4000000);
    
    /**
     * @brief Initialize the flash chip
     * @param spi Pointer to SPI instance (default: &SPI)
     * @return true if successful, false otherwise
     */
    bool begin(SPIClass* spi = &SPI);
    
    // ========== Milestone 1: Device Information ==========
    
    /**
     * @brief Read manufacturer and device ID
     * @return 16-bit ID (Manufacturer | Device ID)
     * Expected: 0xEF14 for W25Q16JV
     */
    uint16_t readDeviceID();
    
    /**
     * @brief Read JEDEC ID (Manufacturer, Memory Type, Capacity)
     * @return 24-bit JEDEC ID
     * Expected: 0xEF4015 for W25Q16JV
     */
    uint32_t readJEDECID();
    
    /**
     * @brief Read unique 64-bit ID
     * @return 64-bit unique ID
     */
    uint64_t readUniqueID();
    
    // ========== Milestone 2: Status Register Operations ==========
    
    /**
     * @brief Read Status Register 1
     * @return Status register 1 value
     */
    uint8_t readStatusReg1();
    
    /**
     * @brief Read Status Register 2
     * @return Status register 2 value
     */
    uint8_t readStatusReg2();
    
    /**
     * @brief Read Status Register 3
     * @return Status register 3 value
     */
    uint8_t readStatusReg3();
    
    /**
     * @brief Write Status Register 1
     * @param data Value to write
     */
    void writeStatusReg1(uint8_t data);
    
    /**
     * @brief Write Status Register 2
     * @param data Value to write
     */
    void writeStatusReg2(uint8_t data);
    
    /**
     * @brief Check if chip is busy
     * @return true if busy, false if ready
     */
    bool isBusy();
    
    // ========== Milestone 3: Read Operations ==========
    
    /**
     * @brief Read a single byte
     * @param addr Address to read from
     * @return Byte value
     */
    uint8_t readByte(uint32_t addr);
    
    /**
     * @brief Read multiple bytes (standard read)
     * @param addr Starting address
     * @param buffer Buffer to store data
     * @param len Number of bytes to read
     */
    void readData(uint32_t addr, uint8_t* buffer, uint32_t len);
    
    /**
     * @brief Fast read multiple bytes
     * @param addr Starting address
     * @param buffer Buffer to store data
     * @param len Number of bytes to read
     */
    void fastRead(uint32_t addr, uint8_t* buffer, uint32_t len);
    
    // ========== Milestone 4: Erase Operations ==========
    
    /**
     * @brief Erase a 4KB sector
     * @param addr Any address within the sector
     */
    void eraseSector(uint32_t addr);
    
    /**
     * @brief Erase a 32KB block
     * @param addr Any address within the block
     */
    void eraseBlock32K(uint32_t addr);
    
    /**
     * @brief Erase a 64KB block
     * @param addr Any address within the block
     */
    void eraseBlock64K(uint32_t addr);
    
    /**
     * @brief Erase entire chip
     * Warning: This takes 5-20 seconds!
     */
    void eraseChip();
    
    // ========== Milestone 5: Write Operations ==========
    
    /**
     * @brief Enable write operations
     */
    void writeEnable();
    
    /**
     * @brief Disable write operations
     */
    void writeDisable();
    
    /**
     * @brief Write up to 256 bytes to a page
     * @param addr Starting address (must be page-aligned recommended)
     * @param buffer Data to write
     * @param len Number of bytes (max 256)
     */
    void writePage(uint32_t addr, const uint8_t* buffer, uint16_t len);
    
    /**
     * @brief Write multiple bytes (handles page boundaries)
     * @param addr Starting address
     * @param buffer Data to write
     * @param len Number of bytes
     */
    void writeData(uint32_t addr, const uint8_t* buffer, uint32_t len);
    
    // ========== Milestone 6: Advanced Byte Operations ==========
    
    /**
     * @brief Write a single byte with read-modify-write
     * This function reads the entire sector, modifies the byte,
     * erases the sector, and writes it back
     * @param addr Address to write
     * @param data Byte value
     * @return Error code (0 = success)
     */
    int writeByte(uint32_t addr, uint8_t data);
    
    /**
     * @brief Modify multiple bytes with read-modify-write
     * @param addr Starting address
     * @param data Data to write
     * @param len Number of bytes
     * @return Error code (0 = success)
     */
    int modifyBytes(uint32_t addr, const uint8_t* data, uint16_t len);
    
    // ========== Power Management ==========
    
    /**
     * @brief Enter power-down mode
     */
    void powerDown();
    
    /**
     * @brief Exit power-down mode
     */
    void releasePowerDown();
    
    // ========== Utility Functions ==========
    
    /**
     * @brief Get chip capacity in bytes
     * @return Capacity detected from JEDEC ID
     */
    uint32_t getCapacity() { return _detected_capacity; }
    
    /**
     * @brief Check if address is valid
     * @param addr Address to check
     * @return true if valid, false otherwise
     */
    bool isValidAddress(uint32_t addr);

private:
    uint8_t _cs_pin;
    uint32_t _spi_freq;
    SPIClass* _spi;
    uint32_t _detected_capacity;  // Auto-detected capacity from JEDEC ID
    
    // Buffer for sector read-modify-write
    uint8_t* _sector_buffer;
    bool _buffer_allocated;
    
    // Low-level SPI operations
    void _select();
    void _deselect();
    uint8_t _transfer(uint8_t data);
    void _sendCommand(uint8_t cmd);
    void _sendAddress(uint32_t addr);
    
    // Wait operations
    void _waitBusy();
    bool _waitBusyTimeout(uint32_t timeout_ms);
    
    // Helper functions
    uint32_t _getSectorAddress(uint32_t addr);
    uint32_t _getPageAddress(uint32_t addr);
    bool _allocateBuffer();
    void _freeBuffer();
};

#endif // W25Q16JV_H
