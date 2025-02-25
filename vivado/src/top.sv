`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/24/2025 10:04:46 PM
// Design Name: 
// Module Name: top
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module top (
    input logic clk,                // System Clock (e.g., 100 MHz)
    input logic pico_mosi,          // RP2040 MOSI
    input logic pico_sck,           // RP2040 SCK
    input logic pico_cs,            // RP2040 Chip Select
    output logic pico_miso,         // RP2040 MISO

    output logic flash_cs,          // SPI Flash Chip Select
    output logic flash_mosi,        // SPI Flash MOSI
    input logic flash_miso          // SPI Flash MISO
);

    // Instantiate SPI Bridge
    spi_bridge bridge (
        .clk(clk),
        .pico_mosi(pico_mosi),
        .pico_sck(pico_sck),
        .pico_cs(pico_cs),
        .pico_miso(pico_miso),
        .flash_cs(flash_cs),
        .flash_mosi(flash_mosi),
        .flash_miso(flash_miso)
    );

endmodule
