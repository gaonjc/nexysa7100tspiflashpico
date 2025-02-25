module spi_bridge (
    input logic clk,                 // FPGA System Clock
    input logic pico_mosi,           // RP2040 MOSI
    input logic pico_sck,            // RP2040 SCK
    input logic pico_cs,             // RP2040 CS
    output logic pico_miso,          // RP2040 MISO

    output logic flash_cs,           // SPI Flash CS
    output logic flash_mosi,         // SPI Flash MOSI
    input logic flash_miso           // SPI Flash MISO
);

    // Internal buffers
    logic sck_buffered;
    logic cs_buffered;
    logic mosi_buffered;
    logic miso_buffered;

    // ? STARTUPE2 for Flash SCK (E9 is a dedicated clock pin)
    STARTUPE2 #(
        .PROG_USR("FALSE"),
        .SIM_CCLK_FREQ(10.0)
    ) startup_inst (
        .USRCCLKO(sck_buffered),  // Connect to buffered SCK
        .USRCCLKTS(1'b0),         // Output enable (0 = enabled)
        .USRDONEO(1'b1),
        .USRDONETS(1'b1),
        .CLK(1'b0),
        .GSR(1'b0),
        .GTS(1'b0),
        .KEYCLEARB(1'b1),
        .PACK(1'b0)
    );

    // ? Buffer SPI signals
    always_ff @(posedge clk) begin
        sck_buffered   <= pico_sck;
        cs_buffered    <= pico_cs;
        mosi_buffered  <= pico_mosi;
        miso_buffered  <= flash_miso;
    end

    // ? Pass SPI data through
    assign flash_cs   = cs_buffered;
    assign flash_mosi = mosi_buffered;
    assign pico_miso  = miso_buffered;

endmodule
