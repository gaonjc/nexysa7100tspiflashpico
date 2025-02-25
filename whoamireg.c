#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// SPI Flash Configuration
#define SPI_PORT spi0
#define SPI_CS_PIN 5   // Chip Select for SPI Flash
#define SPI_SCK_PIN 2  // Clock
#define SPI_MOSI_PIN 3 // MOSI
#define SPI_MISO_PIN 4 // MISO

#define FLASH_WHOAMI_CMD 0x9F // JEDEC ID command for SPI Flash

void spi_init_flash() {
    spi_init(SPI_PORT, 1000 * 1000); // Initialize SPI at 1MHz
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO_PIN, GPIO_FUNC_SPI);
    
    gpio_init(SPI_CS_PIN);
    gpio_set_dir(SPI_CS_PIN, GPIO_OUT);
    gpio_put(SPI_CS_PIN, 1); // Deselect chip
}

uint32_t read_flash_whoami() {
    uint8_t tx_buf[1] = {FLASH_WHOAMI_CMD};
    uint8_t rx_buf[3] = {0};
    
    gpio_put(SPI_CS_PIN, 0); // Select chip
    spi_write_blocking(SPI_PORT, tx_buf, 1); // Send WHOAMI command
    spi_read_blocking(SPI_PORT, 0x00, rx_buf, 3); // Read 3 bytes response
    gpio_put(SPI_CS_PIN, 1); // Deselect chip
    
    return (rx_buf[0] << 16) | (rx_buf[1] << 8) | rx_buf[2]; // Combine response bytes
}

int main() {
    stdio_init_all();
    spi_init_flash();
    

    
    while (1) {
        printf("Reading SPI Flash WHOAMI register...\n");
        uint32_t id = read_flash_whoami();
        printf("Flash WHOAMI ID: 0x%06X\n", id);
        sleep_ms(4000);
    }
    return 0;
}
