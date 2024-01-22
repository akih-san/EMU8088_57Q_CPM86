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

#include "../src/emu88.h"
#include <stdio.h>
#include <assert.h>

#include "../fatfs/ff.h"
#include "../drivers/utils.h"

drive_t drives[] = {
    { 26 },
    { 26 },
    { 26 },
    { 26 },
    { 0 },
    { 0 },
    { 0 },
    { 0 },
    { 128 },
    { 128 },
    { 128 },
    { 128 },
    { 0 },
    { 0 },
    { 0 },
    { 16484 },
};
const int num_drives = (sizeof(drives)/sizeof(*drives));
int do_bus_master;

// console input buffers
#define U3B_SIZE 256
static unsigned char rx_buf[U3B_SIZE];	//UART Rx ring buffer
static unsigned int rx_wp, rx_rp, rx_cnt;
static uint8_t disk_drive;
static uint8_t disk_track;
static uint16_t disk_sector;
static uint8_t disk_op;
static uint16_t disk_dmal;
static uint16_t disk_dmah;
static uint8_t *disk_datap;
static uint8_t disk_stat;
static uint8_t disk_buf[SECTOR_SIZE];


void io_init(void) {
	rx_wp = 0;
	rx_rp = 0;
	rx_cnt = 0;
	disk_stat = DISK_ST_READY;
	do_bus_master = 0;
    disk_drive = 0;
    disk_track = 0;
    disk_sector = 0;
//    disk_op = DISK_OP_RESET;
    disk_dmal = 0;
    disk_dmah = 0;
    disk_datap = NULL;
}

//
// define interrupt
//
// Never called, logically
void __interrupt(irq(default),base(8)) Default_ISR(){}

////////////// UART3 Receive interrupt ////////////////////////////
//UART3 Rx interrupt
/////////////////////////////////////////////////////////////////
void __interrupt(irq(U3RX),base(8)) URT3Rx_ISR(){

	unsigned char rx_data;

	rx_data = U3RXB;			// get rx data

	if (rx_data == CTL_Q) {
		nmi_sig_on();			// NMI interrupt occurs
	}
	else if (rx_cnt < U3B_SIZE) {
		rx_buf[rx_wp] = rx_data;
		rx_wp = (rx_wp + 1) & (U3B_SIZE - 1);
		rx_cnt++;
	}
//	else {
//		// over fllow. rx data is thrown away
//	}
}

// UART3 Transmit
void putch(char c) {
    while(!U3TXIF);             // Wait or Tx interrupt flag set
    U3TXB = c;                  // Write data
}

// UART3 Recive
int getch(void) {
	char c;

	while(!rx_cnt);             // Wait for Rx interrupt flag set
	GIE = 0;                // Disable interrupt
	c = rx_buf[rx_rp];
	rx_rp = (rx_rp + 1) & ( U3B_SIZE - 1);
	rx_cnt--;
	GIE = 1;                // enable interrupt
    return c;               // Read data
}

uint32_t get_physical_addr(uint16_t ah, uint16_t al)
{
// real 32 bit address
//	return (uint32_t)((disk_dmah << 8) | disk_dmal);

// 8086 : segment:offset
	return (uint32_t)((disk_dmah << 4) + disk_dmal);
}

/////////////////////////////////////////////////////////////////
// IO READ/WRITE EXIST
/////////////////////////////////////////////////////////////////
void io_rd_wr(void) {

	uint8_t io_data;
	uint8_t io_addr;

	// get IO port (0 - 255) 16bit IO address does not support
	io_addr = addr_l_pins();

	// check 8088 IO cycle
	if (rd_pin()) goto io_write;

//-------------------------
// 8088 IO read cycle
//-------------------------
	set_data_dir(0x00);           // Set as output
	switch (io_addr) {
		case DISK_REG_FDCST:
			set_data_pins(disk_stat);
			break;

		case UART_CREG:
			set_data_pins(((PIR9 & 0x02) | (rx_cnt !=0))); 		// Out PIR9 (bit0:U3RXIF bit1:U3TXIF)
			break;

		case UART_DREG:
			set_data_pins(rx_buf[rx_rp]);
			rx_rp = (rx_rp + 1) & ( U3B_SIZE - 1);
			rx_cnt--;
			break;

    	default:
        	printf("WARNING: unknown I/O read %d (%02XH)\n\r", io_addr, io_addr);
        	set_data_pins(0xff);    // Invalid data
    }
	end_io_read();
	return;

//-------------------------
// 8088 IO write cycle
//-------------------------
io_write:

	io_data = data_pins();
	switch (io_addr) {
		case DISK_REG_FDCOP:
			// if status BUSY then command does not opetate.
			if(!(disk_stat & DISK_ST_BUSY)) {
				if (io_data == DISK_OP_WRITE) {
					disk_stat = DISK_ST_WRITE | DISK_ST_BUSY;
					disk_datap = disk_buf;
					do_bus_master = 1;
				}
				else if (io_data == DISK_OP_READ) {
					disk_stat = DISK_ST_READ | DISK_ST_BUSY;
					do_bus_master = 1;
//					if (disk_drive == 8 ) {
//						printf( "drive(%d) track(%d) sector(%d)\n\r",disk_drive,disk_track,disk_sector);
//					}
				}
				else disk_stat = DISK_ST_ERROR;
			}
			break;

		case DISK_REG_DRIVE:
			disk_drive = io_data;
			break;
		case DISK_REG_TRACK:
			disk_track = io_data;
			break;
		case DISK_REG_SECTOR:
			disk_sector = (disk_sector & 0xff00) | io_data;
			break;
		case DISK_REG_SECTORH:
			disk_sector = (disk_sector & 0x00ff) | ((uint16_t)io_data << 8);
//			printf("io_addr(%04x) disk_sector(%04x)\n\r", io_addr, disk_sector);
			break;

		case DISK_REG_DMALL:
			disk_dmal = (disk_dmal & 0xff00) | io_data;
//			printf("io_addr(%04x) disk_dmal(%04x)\n\r", io_addr, disk_dmal);
        	break;
		case DISK_REG_DMALH:
			disk_dmal = (disk_dmal & 0x00ff) | ((uint16_t)io_data << 8);
//			printf("io_addr(%04x) disk_dmal(%04x)\n\r", io_addr, disk_dmal);
			break;

		case DISK_REG_DMAHL:
			disk_dmah = (disk_dmah & 0xff00) | io_data;
//			printf("io_addr(%04x) disk_dmah(%04x)\n\r", io_addr, disk_dmah);
			break;
		case DISK_REG_DMAHH:
			disk_dmah = (disk_dmah & 0x00ff) | ((uint16_t)io_data << 8);
//			printf("io_addr(%04x) disk_dmah(%04x)\n\r", io_addr, disk_dmah);
			break;

		case UART_DREG:
		    U3TXB = io_data;        // Write data
			break;
		
		case NMI_SIG_OFF:
			nmi_sig_off();
			break;

		default:
			printf("WARNING: unknown I/O write %d, %d (%02XH, %02XH)\n\r", io_addr, io_data, io_addr,io_data);
	}
	end_io_write();
	if ( do_bus_master ) set_hold_pin();			// set HOLD signal
}

//
// bus master handling
// this fanction is invoked at main() after HOLDA = 1
//

void bus_master_operation(void) {

	uint32_t addr;

    bus_master(1);

	//
	// Do disk I/O
	//

	uint32_t sector = 0;
	if (num_drives <= disk_drive || drives[disk_drive].filep == NULL) {
		disk_stat = DISK_ST_ERROR;
	}
	else {
		sector = disk_track * drives[disk_drive].sectors + disk_sector - 1;
		FIL *filep = drives[disk_drive].filep;
		unsigned int n;
		FRESULT fres;
		if ((fres = f_lseek(filep, sector * SECTOR_SIZE)) != FR_OK) {
			printf("f_lseek(): ERROR %d\n\r", fres);
			disk_stat = DISK_ST_ERROR;
		}
		else if (disk_stat & DISK_ST_READ) {
			//
			// DISK read
			 //

			// read from the DISK
			if ((fres = f_read(filep, disk_buf, SECTOR_SIZE, &n)) != FR_OK || n != SECTOR_SIZE) {
				printf("f_read(): ERROR res=%d, n=%d\n\r", fres, n);
				disk_stat = DISK_ST_ERROR;
			}
			else if (DEBUG_DISK_READ && DEBUG_DISK_VERBOSE && !(debug.disk_mask & (1 << disk_drive))) {
				util_hexdump_sum("buf: ", disk_buf, SECTOR_SIZE);
			}
			else {
				//
				// DMA operation
				//
				// transfer read data to SRAM
				addr = get_physical_addr( disk_dmah, disk_dmal );
				write_sram(addr, disk_buf, SECTOR_SIZE);
				disk_stat = DISK_ST_READY;
///debug
//printf("f_read(): SRAM address(%08lx),disk_dmah(%04x),disk_dmal(%04x)\n\r", addr, disk_dmah, disk_dmal);
///debug
				#ifdef CPM_MEM_DEBUG
				printf("f_read(): SRAM address(%08lx),disk_dmah(%04x),disk_dmal(%04x)\n\r", addr, disk_dmah, disk_dmal);
				read_sram(addr, disk_buf, SECTOR_SIZE);
				util_hexdump_sum("RAM: ", disk_buf, SECTOR_SIZE);
				#endif  // CPM_MEM_DEBUG
			}
		}
		else if (disk_stat & DISK_ST_WRITE) {
			//
			// DISK write
			//
			// transfer write data from SRAM to the buffer
			addr = get_physical_addr( disk_dmah, disk_dmal );
			read_sram(addr, disk_buf, SECTOR_SIZE);

			if (DEBUG_DISK_WRITE && DEBUG_DISK_VERBOSE && !(debug.disk_mask & (1 << disk_drive))) {
				util_hexdump_sum("buf: ", disk_buf, SECTOR_SIZE);
			}

			// write buffer to the DISK
			if ((fres = f_write(filep, disk_buf, SECTOR_SIZE, &n)) != FR_OK || n != SECTOR_SIZE) {
				printf("f_write(): ERROR res=%d, n=%d\n\r", fres, n);
				disk_stat = DISK_ST_ERROR;
			}
			else if ((fres = f_sync(filep)) != FR_OK) {
				printf("f_sync(): ERROR %d\n\r", fres);
				disk_stat = DISK_ST_ERROR;
			}
			else disk_stat = DISK_ST_READY;
		}
		else disk_stat = DISK_ST_ERROR;
	}

// exit_bus_master:

	do_bus_master = 0;
    bus_master(0);
	reset_hold_pin();			// reset HOLD signal
}

