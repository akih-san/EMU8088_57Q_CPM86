/*
 * UART, disk I/O and monitor firmware for EMU8088
 *
 * Based on main.c by Tetsuya Suzuki and emuz80_z80ram.c by Satoshi Okue
 * Base source code by @hanyazou https://twitter.com/hanyazou
 * Modified by Akihito Honda
 */
/*!
 * PIC18F47Q43/PIC18F47Q83/PIC18F47Q84 ROM image uploader and UART emulation firmware
 * This single source file contains all code
 *
 * Target: EMUZ80 with Z80+RAM
 * Compiler: MPLAB XC8 v2.40
 *
 * Modified by Satoshi Okue https://twitter.com/S_Okue
 * Version 0.1 2022/11/15
 */

/*
    PIC18F57Q43 ROM RAM and UART emulation firmware
    This single source file contains all code
    Original source code for PIC18F47Q43 ROM RAM and UART emulation firmware
    Designed by @hanyazou https://twitter.com/hanyazou

    Target: EMU8088 - The computer with only 8088/V20 and PIC18F57Q43
    Written by Akihito Honda
*/

#define BOARD_DEPENDENT_SOURCE

#include "../emu88.h"

static void emu88_common_sys_init()
{
    // System initialize
    OSCFRQ = 0x08;      // 64MHz internal OSC

    // Disable analog function
    ANSELA = 0x00;
    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;
    ANSELF = 0x00;
    ANSELE0 = 0;
    ANSELE1 = 0;
    ANSELE2 = 0;

    // RESET output pin
    PPS(I88_RESET) = 0;        // assign I88_RESET to LATCH
	LAT(I88_RESET) = 1;        // Reset
    TRIS(I88_RESET) = 0;       // Set as output

	// HOLDA
	WPU(I88_HOLDA) = 1;     // I88_HOLDA Week pull up
	LAT(I88_HOLDA) = 1;     // set HOLDA=1 (74LS373 is Hi-Z)
	TRIS(I88_HOLDA) = 0;    // Set as output during RESET period

    // UART3 initialize
    U3BRG = 416;        // 9600bps @ 64MHz
    U3RXEN = 1;         // Receiver enable
    U3TXEN = 1;         // Transmitter enable

    // UART3 Receiver
    TRISA7 = 1;         // RX set as input
    U3RXPPS = 0x07;     // RA7->UART3:RXD;

    // UART3 Transmitter
    LATA6 = 1;          // Default level
    TRISA6 = 0;         // TX set as output
    RA6PPS = 0x26;      // UART3:TXD -> RA6;

    U3ON = 1;           // Serial port enable

	// Init address LATCH to 0
	// Address bus A7-A0 pin
    WPU(I88_ADDR_L) = 0xff;     // Week pull up
	LAT(I88_ADDR_L) = 0xff;
    TRIS(I88_ADDR_L) = 0x00;    // Set as output

    // Address bus A15-A8
    WPU(I88_ADDR_H) = 0xff;     // Week pull up
    LAT(I88_ADDR_H) = 0xff;
    TRIS(I88_ADDR_H) = 0x00;    // Set as output


	WPU(I88_A16) = 1;     // A16 Week pull up
	LAT(I88_A16) = 1;     // init A16=0
    TRIS(I88_A16) = 0;    // Set as output

	WPU(I88_A17) = 1;     // A17 Week pull up
	LAT(I88_A17) = 1;     // init A17=0
    TRIS(I88_A17) = 0;    // Set as output

	WPU(I88_A18) = 1;     // A18 Week pull up
	LAT(I88_A18) = 1;     // init A18=0
    TRIS(I88_A18) = 0;    // Set as output

	WPU(I88_A19) = 1;     // A19 Week pull up
	LAT(I88_A19) = 1;     // init A19=0
    TRIS(I88_A19) = 0;    // Set as output

	// Data bus D7-D0 pin
    WPU(I88_DATA) = 0x00;       // Week pull down
    LAT(I88_DATA) = 0x00;
    TRIS(I88_DATA) = 0x00;      // Set as output

}

static void emu88_common_start_i88(void)
{
    // Address bus A15-A8
	TRIS(I88_ADDR_H) = 0xff;    // Set as input

    // Address bus A7-A0 pin
    TRIS(I88_ADDR_L) = 0xff;    // Set as input
    TRIS(I88_A16) = 1;    // Set as input
    TRIS(I88_A17) = 1;    // Set as input
    TRIS(I88_A18) = 1;    // Set as input
    TRIS(I88_A19) = 1;    // Set as input

	// Data bus D7-D0 input pin
    TRIS(I88_DATA) = 0xff;      // Set as input

    TRIS(I88_RD) = 1;           // Set as input
    TRIS(I88_WR) = 1;           // Set as input
	TRIS(I88_IOM) = 1;			// Set as input

	reset_io_read_mask();
#ifdef USE_READY
	reset_ready_pin();
#else
	reset_t2_mask();
#endif
}

void write_sram(uint32_t addr, uint8_t *buf, unsigned int len)
{
    union address_bus_u ab;
    unsigned int i;

    ab.w = addr;

	// set SRAM read address
	LAT(I88_IOM) = 0;		// active RAM CE
	LAT(SRAM_OE) = 1;		// deactivate /OE(/RD)
	TRIS(I88_DATA) = 0x00;	// Set as output

	// set SRAM read address
	LAT(I88_ADDR_H) = ab.lh;
    LAT(I88_ADDR_L) = ab.ll;
	LAT(I88_A16) = (ab.hl & 0x01) ? 1 : 0;
	LAT(I88_A17) = (ab.hl & 0x02) ? 1 : 0;
	LAT(I88_A18) = (ab.hl & 0x04) ? 1 : 0;
	LAT(I88_A19) = (ab.hl & 0x08) ? 1 : 0;
    for(i = 0; i < len; i++) {
        LAT(SRAM_WE) = 0;					// activate /WE
        LAT(I88_DATA) = ((uint8_t*)buf)[i];
        LAT(SRAM_WE) = 0;					// activate /WE
        LAT(SRAM_WE) = 1;					// deactivate /WE

    	LAT(I88_ADDR_L) = ++ab.ll;
        if (ab.ll == 0) {
		    LAT(I88_ADDR_H) = ++ab.lh;
        	if (ab.lh == 0) {
        		ab.hl++;
        		LAT(I88_A16) = (ab.hl & 0x01) ? 1 : 0;
				LAT(I88_A17) = (ab.hl & 0x02) ? 1 : 0;
				LAT(I88_A18) = (ab.hl & 0x04) ? 1 : 0;
				LAT(I88_A19) = (ab.hl & 0x08) ? 1 : 0;
        	}
        }
    }
	LAT(I88_IOM) = 1;		// deactive RAM CE
}

void read_sram(uint32_t addr, uint8_t *buf, unsigned int len)
{
    union address_bus_u ab;
    unsigned int i;

	LAT(I88_IOM) = 0;				// active RAM CE
	LAT(SRAM_WE) = 1;				// deactivate /WE
	TRIS(I88_DATA) = 0xFF;			// Set as input

	ab.w = addr;

	// set SRAM read address
	LAT(I88_ADDR_H) = ab.lh;
    LAT(I88_ADDR_L) = ab.ll;
	LAT(I88_A16) = (ab.hl & 0x01) ? 1 : 0;
	LAT(I88_A17) = (ab.hl & 0x02) ? 1 : 0;
	LAT(I88_A18) = (ab.hl & 0x04) ? 1 : 0;
	LAT(I88_A19) = (ab.hl & 0x08) ? 1 : 0;

	for(i = 0; i < len; i++) {
        LAT(SRAM_OE) = 0;      // activate /OE
        ((uint8_t*)buf)[i] = PORT(I88_DATA);
        LAT(SRAM_OE) = 0;      // activate /OE
        LAT(SRAM_OE) = 1;      // deactivate /OE

		LAT(I88_ADDR_L) = ++ab.ll;
        if (ab.ll == 0) {
		    LAT(I88_ADDR_H) = ++ab.lh;
        	if (ab.lh == 0) {
        		ab.hl++;
				LAT(I88_A16) = (ab.hl & 0x01) ? 1 : 0;
				LAT(I88_A17) = (ab.hl & 0x02) ? 1 : 0;
				LAT(I88_A18) = (ab.hl & 0x04) ? 1 : 0;
				LAT(I88_A19) = (ab.hl & 0x08) ? 1 : 0;
        	}
        }
    }
	LAT(I88_IOM) = 1;		// deactive RAM CE
}

static uint8_t emu88_common_addr_l_pins(void) { return PORT(I88_ADDR_L); }
static void emu88_common_set_addr_l_pins(uint8_t v) { LAT(I88_ADDR_L) = v; }
static uint8_t emu88_common_data_pins(void) { return PORT(I88_DATA); }
static void emu88_common_set_data_pins(uint8_t v) { LAT(I88_DATA) = v; }
static void emu88_common_set_data_dir(uint8_t v) { TRIS(I88_DATA) = v; }
static __bit emu88_common_rd_pin(void) { return R(I88_RD); }
static __bit emu88_common_wr_pin(void) { return R(I88_WR); }

static void emu88_common_init()
{
    board_addr_l_pins_hook      = emu88_common_addr_l_pins;
    board_set_addr_l_pins_hook  = emu88_common_set_addr_l_pins;
    board_data_pins_hook        = emu88_common_data_pins;
    board_set_data_pins_hook    = emu88_common_set_data_pins;
    board_set_data_dir_hook     = emu88_common_set_data_dir;
    board_rd_pin_hook           = emu88_common_rd_pin;
    board_wr_pin_hook           = emu88_common_wr_pin;
}

static void emu88_common_wait_for_programmer()
{
    //
    // Give a chance to use PRC (RB6) and PRD (RB7) to PIC programer.
    //
    printf("\n\r");
    printf("wait for programmer ...\r");
    __delay_ms(200);
    printf("                       \r");

    printf("\n\r");
}
