/*
Author: Ross Johnson
File: final_ethernet_controller.v
Version 1.6

The Verilog for the final Ethernet controller. It instantiates 
the data processor, PLL, DDIO and the Nios II
*/
module final_ethernet_controller
(
    //clock and reset
    input CLOCK_50,
    input KEY,
    // Ethernet 1
    output       ETHERNET_1_GTX_CLK,
    output       ETHERNET_1_MDC,
    inout        ETHERNET_1_MDIO,
    output       ETHERNET_1_RESET_N,
    input        ETHERNET_1_RX_CLK,
    input  [3:0] ETHERNET_1_RX_DATA,
    input        ETHERNET_1_RX_DV,
    output [3:0] ETHERNET_1_TX_DATA,
    output       ETHERNET_1_TX_EN,
    //SDRAM
    output [12:0] DRAM_ADDR,
    output [1:0]  DRAM_BA,
    output        DRAM_CAS_N,
    output        DRAM_CKE,
    output        DRAM_CLK,
    output        DRAM_CS_N,
    inout  [31:0] DRAM_DQ,
    output [3:0]  DRAM_DQM,
    output        DRAM_RAS_N,
    output        DRAM_WE_N,
    //LEDs
    output [17:0] LEDR,
	output [8:0]  LEDG
);

    wire sys_clk, tx_clk;
    wire reset;
    wire mdc, mdio_in, mdio_oen, mdio_out;
    wire [31:0] vita_49_data;
	wire vita_49_data_ready;
    //MDIO lines
    assign mdio_in   = ETHERNET_1_MDIO;
    assign ETHERNET_1_MDC  = mdc;
    //tri state bus
    assign ETHERNET_1_MDIO = mdio_oen ? 1'bz : mdio_out;

    assign ETHERNET_1_RESET_N = reset;
    //PLL
    my_pll pll_inst
    (
        .areset (~KEY),
        .inclk0 (CLOCK_50),
        .c0     (sys_clk),
        .c1     (tx_clk),
        .c2     (DRAM_CLK),
        .locked (reset)
    );
    //DDIO
    my_ddio_out ddio_out_inst
    (
        .datain_h (1'b1),
        .datain_l (1'b0),
        .outclock (tx_clk),
        .dataout  (ETHERNET_1_GTX_CLK)
    );
    //Nios II
    nios_system system_inst
    (
        .clk_clk                                 (sys_clk),
        .reset_reset_n                           (reset),
        .tse_mac_mac_rgmii_connection_rx_control (ETHERNET_1_RX_DV),
        .tse_mac_pcs_mac_rx_clock_connection_clk (ETHERNET_1_RX_CLK),
        .tse_mac_mac_rgmii_connection_tx_control (ETHERNET_1_TX_EN),
        .tse_mac_pcs_mac_tx_clock_connection_clk (tx_clk),
        .tse_mac_mac_rgmii_connection_rgmii_out  (ETHERNET_1_TX_DATA),
        .tse_mac_mac_rgmii_connection_rgmii_in   (ETHERNET_1_RX_DATA),
        .tse_mac_mac_mdio_connection_mdio_in     (mdio_in),
        .tse_mac_mac_mdio_connection_mdio_out    (mdio_out),
        .tse_mac_mac_mdio_connection_mdc         (mdc),
        .tse_mac_mac_mdio_connection_mdio_oen    (mdio_oen),
        .sdram_wire_addr                         (DRAM_ADDR),
        .sdram_wire_ba                           (DRAM_BA),
        .sdram_wire_cas_n                        (DRAM_CAS_N),
        .sdram_wire_cke                          (DRAM_CKE),
        .sdram_wire_cs_n                         (DRAM_CS_N),
        .sdram_wire_dq                           (DRAM_DQ),
        .sdram_wire_dqm                          (DRAM_DQM),
        .sdram_wire_ras_n                        (DRAM_RAS_N),
        .sdram_wire_we_n                         (DRAM_WE_N),
        .data_lines_external_connection_export   (vita_49_data),
		.data_ready_external_connection_export   (vita_49_data_ready)
    );
    //data processor
    data_processing data
    (
        .sys_clk       (sys_clk),
        .reset         (reset),
        .data          (vita_49_data),
        .data_ready    (vita_49_data_ready),
        .display_lines (LEDR)
    );
    
endmodule