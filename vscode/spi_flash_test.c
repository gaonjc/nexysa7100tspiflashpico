#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Flash Configuration
#define SPI_PORT spi0
#define SPI_CS_PIN 5   // Chip Select
#define SPI_SCK_PIN 2  // Clock
#define SPI_MOSI_PIN 3 // MOSI
#define SPI_MISO_PIN 4 // MISO

#define FLASH_CMD_WRITE_ENABLE  0x06
#define FLASH_CMD_READ_STATUS   0x05
#define FLASH_CMD_PAGE_PROGRAM  0x02
#define FLASH_CMD_READ_DATA     0x03
#define FLASH_CMD_SECTOR_ERASE  0x20

#define FLASH_TEST_ADDRESS  0x000100  // Test address
#define PAGE_SIZE           256       // Flash page size

void cs_select() { gpio_put(SPI_CS_PIN, 0); }
void cs_deselect() { gpio_put(SPI_CS_PIN, 1); }

// Enable write operations on the flash
void spi_flash_write_enable() {
    cs_select();
    uint8_t cmd = FLASH_CMD_WRITE_ENABLE;
    spi_write_blocking(SPI_PORT, &cmd, 1);
    cs_deselect();

    // Check if write enable was successful
    cs_select();
    uint8_t status_cmd = FLASH_CMD_READ_STATUS;
    uint8_t status;
    spi_write_blocking(SPI_PORT, &status_cmd, 1);
    spi_read_blocking(SPI_PORT, 0xFF, &status, 1);
    cs_deselect();

    if (!(status & 0x02)) {
        printf("Write Enable FAILED! Status: 0x%02X\n", status);
    } else {
        printf("Write Enable SUCCESS! Status: 0x%02X\n", status);
    }
}

// Wait until the flash is ready
void spi_flash_wait_for_ready() {
    cs_select();
    uint8_t cmd = FLASH_CMD_READ_STATUS;
    uint8_t status;
    do {
        spi_write_blocking(SPI_PORT, &cmd, 1);
        spi_read_blocking(SPI_PORT, 0xFF, &status, 1);
    } while (status & 0x01);  // Wait while busy
    cs_deselect();
}

// Erase a sector of flash memory
void spi_flash_sector_erase(uint32_t address) {
    spi_flash_write_enable();
    cs_select();
    uint8_t cmd[4] = { FLASH_CMD_SECTOR_ERASE, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF };
    spi_write_blocking(SPI_PORT, cmd, 4);
    cs_deselect();
    
    sleep_ms(50);  // Allow time for erase

    // Check status register after erase
    cs_select();
    uint8_t status_cmd = FLASH_CMD_READ_STATUS;
    uint8_t status;
    spi_write_blocking(SPI_PORT, &status_cmd, 1);
    spi_read_blocking(SPI_PORT, 0xFF, &status, 1);
    cs_deselect();
    printf("Erase Status Register: 0x%02X\n", status);
    
    spi_flash_wait_for_ready();
}

// Write a page to flash memory
void spi_flash_write_page(uint32_t address, const uint8_t *data, size_t length) {
    spi_flash_write_enable();
    
    cs_select();
    uint8_t cmd[4] = { FLASH_CMD_PAGE_PROGRAM, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF };
    spi_write_blocking(SPI_PORT, cmd, 4);
    spi_write_blocking(SPI_PORT, data, length);
    cs_deselect();

    sleep_ms(10);  // Allow write time

    // Read status register
    cs_select();
    uint8_t status_cmd = FLASH_CMD_READ_STATUS;
    uint8_t status;
    spi_write_blocking(SPI_PORT, &status_cmd, 1);
    spi_read_blocking(SPI_PORT, 0xFF, &status, 1);
    cs_deselect();

    printf("Write Status Register: 0x%02X\n", status);

    spi_flash_wait_for_ready();
}

// Read data from flash memory
void spi_flash_read_data(uint32_t address, uint8_t *data, size_t length) {
    cs_select();
    uint8_t cmd[4] = { FLASH_CMD_READ_DATA, (address >> 16) & 0xFF, (address >> 8) & 0xFF, address & 0xFF };
    spi_write_blocking(SPI_PORT, cmd, 4);
    spi_read_blocking(SPI_PORT, 0xFF, data, length);
    cs_deselect();
}

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Allow time for serial monitor to connect
    printf("SPI Flash Test\n");

    // Initialize SPI at a lower speed
    spi_init(SPI_PORT, 50000);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO_PIN, GPIO_FUNC_SPI);
    
    gpio_init(SPI_CS_PIN);
    gpio_set_dir(SPI_CS_PIN, GPIO_OUT);
    gpio_put(SPI_CS_PIN, 1);

    // Enable internal pull-ups
    gpio_pull_up(SPI_MISO_PIN);
    gpio_pull_up(SPI_CS_PIN);

    printf("SPI Initialized!\n");

    // Check erased state
    uint32_t check_value = 0;
    spi_flash_read_data(FLASH_TEST_ADDRESS, (uint8_t *)&check_value, sizeof(check_value));
    printf("Erased State Value: 0x%08X\n", check_value);

    // Erase sector
    printf("Erasing sector at 0x%06X...\n", FLASH_TEST_ADDRESS);
    spi_flash_sector_erase(FLASH_TEST_ADDRESS);

    // Check again after erasing
    spi_flash_read_data(FLASH_TEST_ADDRESS, (uint8_t *)&check_value, sizeof(check_value));
    printf("Post-Erase Value: 0x%08X\n", check_value);

    // Test integer to write
    uint32_t write_value = 0x12345678;
    uint32_t read_value = 0;

    spi_flash_write_enable();
    printf("Write enabled!\n");

    // Write integer to flash
    printf("Writing integer 0x%08X to flash...\n", write_value);
    spi_flash_write_page(FLASH_TEST_ADDRESS, (uint8_t *)&write_value, sizeof(write_value));

    // Wait a few seconds
    sleep_ms(100);

    // Read integer from flash
    printf("Reading integer from flash...\n");
    spi_flash_read_data(FLASH_TEST_ADDRESS, (uint8_t *)&read_value, sizeof(read_value));

    // Read raw bytes
    uint8_t read_buffer[4] = {0};
    spi_flash_read_data(FLASH_TEST_ADDRESS, read_buffer, sizeof(read_buffer));
    printf("Read Bytes: 0x%02X 0x%02X 0x%02X 0x%02X\n", read_buffer[0], read_buffer[1], read_buffer[2], read_buffer[3]);

    printf("Read Integer: 0x%08X\n", read_value);
    sleep_ms(4*1000);
    return 0;
}
