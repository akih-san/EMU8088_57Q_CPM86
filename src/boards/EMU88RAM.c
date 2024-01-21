/*
 * Copyright (c) 2023 @hanyazou
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#define BOARD_DEPENDENT_SOURCE

#include "../../src/emu88.h"
#include <stdio.h>
#include "../../drivers/SDCard.h"
#include "../../drivers/picregister.h"

#define SPI_PREFIX      SPI_SD
#define SPI_HW_INST     SPI1
#include "../../drivers/SPI.h"

#define I88_DATA        C
#define I88_ADDR_H      F
#define I88_ADDR_L      D

#define I88_A16		A0
#define I88_A17		A1
#define I88_A18		A2
#define I88_A19		A3

#define I88_IOM		B0
#define I88_ALE		B3

#define I88_RESET	E0
#define I88_HOLD	E1		// BUS request
#define I88_HOLDA	A5

// /WR
#define I88_WR		B1
// /RD
#define I88_RD		B2

// CLK
#define I88_CLK		B7

#define I88_READY	B5
#ifndef I88_READY
#define I88_TEST	B5
#endif

#define I88_NMI		B6
#define I88_INTA	A4
#define I88_INTR	B4

#define SRAM_OE		I88_RD
#define SRAM_WE		I88_WR

// RA6 is used as UART TXD
// RA7 is used as UART RXD

#define SPI_SS		E2
#define SPI_SD_POCI	C2

#define SPI_SD_PICO     C0
#define SPI_SD_CLK      C1
#define SPI_SD_SS       SPI_SS

#define IO_RW_event CLC5OUT
#define BUS_HOLD_ACK R(I88_HOLDA)

#include "emu88_common.c"

static void emu88_57q_sys_init()
{
    emu88_common_sys_init();

	WPU(I88_NMI) = 0;     // NMI Week pull down
	PPS(I88_NMI) = 0;     // set latch port
	LAT(I88_NMI) = 0;     // NMI=0
	TRIS(I88_NMI) = 0;    // Set as output

#ifdef I88_READY
    LAT(I88_READY) = 1;          // set READY
    TRIS(I88_READY) = 0;         // Set as output
#else
    LAT(I88_TEST) = 0;          // set /TEST
    TRIS(I88_TEST) = 0;         // Set as output
#endif

	// HOLD output pin
    LAT(I88_HOLD) = 1;
    TRIS(I88_HOLD) = 0;          // Set as output
	
	// IO/#M
	WPU(I88_IOM) = 1;     // I88_IOM Week pull up
	LAT(I88_IOM) = 1;     // init: I/O operation
	TRIS(I88_IOM) = 0;    // Set as onput

	// ALE
	WPU(I88_ALE) = 0;     // I88_ALE Week pull down
    TRIS(I88_ALE) = 1;    // Set as input

	// /WR output pin
	WPU(SRAM_WE) = 1;		// /WR Week pull up
    LAT(SRAM_WE) = 1;		// disactive
    TRIS(SRAM_WE) = 0;		// Set as output
    PPS(SRAM_WE) = 0x00;	//reset:output PPS is LATCH

	// /RD output pin
	WPU(SRAM_OE) = 1;		// /WR Week pull up
    LAT(SRAM_OE) = 1;		// disactive
    TRIS(SRAM_OE) = 0;		// Set as output
    PPS(SRAM_OE) = 0x00;	//reset:output PPS is LATCH

	// SPI_SS
	WPU(SPI_SS) = 1;     // SPI_SS Week pull up
	LAT(SPI_SS) = 1;     // set SPI disable
	TRIS(SPI_SS) = 0;    // Set as onput

	LAT(I88_CLK) = 1;	// 8088_CLK = 1
    TRIS(I88_CLK) = 0;	// set as output pin
	
#ifdef USE_READY
#include "clc_pwm3_ready.c"
#else
#include "clc_pwm3_t2wait.c"
#endif

	// SPI data and clock pins slew at maximum rate
   SLRCON(SPI_SD_PICO) = 0;
   SLRCON(SPI_SD_CLK) = 0;
   SLRCON(SPI_SD_POCI) = 0;

/*********** CLOCK TIMING ************************
 I88_CLK TIMING REQUIREMENTS(MUST)
 CLK Low Time  : minimum 118ns
 CLK High Time : minimum 69ns

 <NG>
 I88_CLK = 5000000UL
 1/5MHz  = 200ns
 CLK Low Time  : 100ns
 CLK High Time : 100ns

 <OK>
 I88_CLK = 4000000UL
 1/4MHz  = 250ns
 CLK Low Time  : 125ns
 CLK High Time : 125ns
*************************************************/

/**************** PWM3 ********************
// 8088 TIMING REQUIREMENTS(MUST)
// CLK Low Time  : minimum 118ns
// CLK High Time : minimum 69ns
// CLK duty 33%

// P64 = 1/64MHz = 15.625ns
// P5  = 1/5MHz  = 200ns = P64 * 12.8
//
// Set PWM Left Aligned mode
// PR = 12
// P1 = 5 : P64*5 = 78.125ns
// P2 = 8 : P64*8 = 125ns
// MODE = 0
//     high period time: 125ns
//     low period time: 78.125ns
//     125ns + 78.125 = 203.125ns f = 4923076.9230769230769230769230769 Hz
//     duty = 38.4%
******************************************/

//	PWM3CLK = 0x02;		// Fsoc
	PWM3CLK = 0x14;		// CLC8_OUT
	PWM3GIE = 0x00;		// interrupt disable
	PWM3PR = 0x000C;	// 13 periods ( 0 - 12 )
	PWM3S1P1 = 0x0005;	// P1 = 5
	PWM3S1P2 = 0x0008;	// P2 = 8
	PWM3S1CFG = 0x00;	// (POL1, POL2)= 0, PPEN = 0 MODE = 0 (Left Aligned mode)
	PWM3CON = 0x84;		// EN=1, LD=1
	RB7PPS = 0x1C;		// PWM3S1P1_OUT

    emu88_common_wait_for_programmer();

    //
    // Initialize SD Card
    //
    static int retry;
    for (retry = 0; 1; retry++) {
        if (20 <= retry) {
            printf("No SD Card?\n\r");
            while(1);
        }
//debug111
        if (SDCard_init(SPI_CLOCK_100KHZ, SPI_CLOCK_2MHZ, /* timeout */ 100) == SDCARD_SUCCESS)
//        if (SDCard_init(SPI_CLOCK_100KHZ, SPI_CLOCK_8MHZ, /* timeout */ 100) == SDCARD_SUCCESS)
            break;
        __delay_ms(200);
    }
}

static void emu88_57q_bus_master(int enable)
{
    if (enable) {
        // Set address bus as output
        TRIS(I88_ADDR_L) = 0x00;	// A7-A0
        TRIS(I88_ADDR_H) = 0x00;	// A8-A15
    	TRIS(I88_A16) = 0;			// Set as output
	    TRIS(I88_A17) = 0;			// Set as output
	    TRIS(I88_A18) = 0;			// Set as output
	    TRIS(I88_A19) = 0;			// Set as output
    	
    	LAT(I88_RD) = 1;
        LAT(I88_WR) = 1;

    	TRIS(I88_RD) = 0;           // output
    	TRIS(I88_WR) = 0;           // output
    	// SRAM U4, U5 are HiZ

    	LAT(I88_IOM) = 1;     // IOM# =1 set IO accsess 
    	TRIS(I88_IOM) = 0;    // Set as output

    } else {

    	// Set address bus as input
        TRIS(I88_ADDR_L) = 0xff;    // A7-A0
        TRIS(I88_ADDR_H) = 0xff;    // A8-A15
        TRIS(I88_DATA) = 0xff;      // D7-D0 pin
    	TRIS(I88_A16) = 1;    // Set as input
	    TRIS(I88_A17) = 1;    // Set as input
	    TRIS(I88_A18) = 1;    // Set as input
	    TRIS(I88_A19) = 1;    // Set as input

        // Set /RD and /WR as input
        TRIS(I88_RD) = 1;           // input
        TRIS(I88_WR) = 1;           // input
    	TRIS(I88_IOM) = 1;          // input

    }
}

static void emu88_57q_start_i88(void)
{

    emu88_common_start_i88();

    // Unlock IVT
    IVTLOCK = 0x55;
    IVTLOCK = 0xAA;
    IVTLOCKbits.IVTLOCKED = 0x00;

    // Default IVT base address
    IVTBASE = 0x000008;

    // Lock IVT
    IVTLOCK = 0x55;
    IVTLOCK = 0xAA;
    IVTLOCKbits.IVTLOCKED = 0x01;


//	printf("Start 8088/V20\r\n");

	TRIS(I88_HOLDA) = 1;    // HOLDA is set as input
	LAT(I88_HOLD) = 0;		// Release HOLD
	// I88 start
    LAT(I88_RESET) = 0;		// Release reset
}

#ifdef USE_READY
void reset_ready_pin(void)
{
	// Release wait (D-FF reset)
    CLCSELECT = 0;       // CLC1 select
	G3POL = 1;
	G3POL = 0;
}
#else
void reset_t2_mask(void)
{
	// Release wait (D-FF reset)
    CLCSELECT = 1;       // CLC2 select
	G3POL = 1;
	G3POL = 0;
}
#endif

void reset_io_read_mask(void)
{
	CLCSELECT = 2;       // CLC3 select
	G3POL = 1;
	G3POL = 0;
}

void end_io_read(void)
{
#ifdef USE_READY
//	reset_ready_pin();	// Reset READY signal
    CLCSELECT = 0;       // CLC1 select
	G3POL = 1;
	G3POL = 0;
#else
//	reset_t2_mask();			// Reset T2 mask flag
    CLCSELECT = 1;       // CLC2 select
	G3POL = 1;
	G3POL = 0;
#endif
	while(!R(I88_RD)) {};		// wait for /RD to be cleared
	TRIS(I88_DATA) = 0xff;		// Data bus is set as input
	reset_io_read_mask();		// Release 8088_CLK
}


void end_io_write(void)
{
#ifdef USE_READY
	reset_ready_pin();			// Reset READY signal
#else
	reset_t2_mask();			// Reset T2 mask flag
#endif

// if CLK > 5MHz, it may not be needed
//	while(!R(I88_WR)) {};		// wait for /WR to be cleared
}

void set_hold_pin(void)
{
	LAT(I88_HOLD) = 1;
}

void reset_hold_pin(void)
{
	LAT(I88_HOLD) = 0;
}
void emu88_57q_nmi_sig_off(void)
{
	LAT(I88_NMI) = 0;
}

void emu88_57q_nmi_sig_on(void)
{
	LAT(I88_NMI) = 1;
}

void board_init()
{
    emu88_common_init();

    board_sys_init_hook = emu88_57q_sys_init;
    board_start_i88_hook = emu88_57q_start_i88;
    bus_master_hook = emu88_57q_bus_master;
	board_nmi_sig_off_hook = emu88_57q_nmi_sig_off;
	board_nmi_sig_on_hook = emu88_57q_nmi_sig_on;
}

void board_event_loop(void) {
	while(1) {
    	if (!IO_RW_event) io_rd_wr();
		if (BUS_HOLD_ACK) bus_master_operation();
	}
}

#include "../../drivers/pic18f57q43_spi.c"
#include "../../drivers/SDCard.c"

