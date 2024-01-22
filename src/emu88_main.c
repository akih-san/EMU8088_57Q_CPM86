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

#define INCLUDE_PIC_PRAGMA
#include "../src/emu88.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../drivers/utils.h"

static FATFS fs;
static DIR fsdir;
static FILINFO fileinfo;
static FIL files[NUM_FILES];
static int num_files = 0;
uint8_t tmp_buf[2][TMP_BUF_SIZE];
debug_t debug = {
    0,  // disk
    0,  // disk_read
    0,  // disk_write
    0,  // disk_verbose
    0,  // disk_mask
};

const unsigned char rom0[] = {
// Initial program loader at 0xFFFF0
#include "../i88/ccp_bdos.inc"
};

const unsigned char rom1[] = {
// Initial program loader at 0xFFFF0
#include "../i88/cbios.inc"
};

const unsigned char rom2[] = {
// Initial program loader at 0xFFFF0
//#include "../i88/start_86.inc"
#include "../i88/unimon_8086.inc"
};

void sys_init(void);
int disk_init(void);
int menu_select(void);
void start_i88(void);

static char *board_name = "EMU8088_RAM";

// main routine
void main(void)
{
    sys_init();
    printf("Board: %s\n\r", board_name);
    if (disk_init() < 0)
        while (1);
    io_init();
    mem_init();

    U3RXIE = 1;          // Receiver interrupt enable
    GIE = 1;             // Global interrupt enable

    //
    // Transfer ROM image to the SRAM
	// ROM image must fit 8088 startup address. reset vector = FFFF0h
	//
	/* Universal Monitor */
	write_sram(0x100000-sizeof(rom2), (uint8_t *)rom2, sizeof(rom2));
	printf("Load unimon = %06lx, Program size = %04x\r\n", 0x100000-sizeof(rom2), sizeof(rom2));
	
	/* CCP+BDOS */
	write_sram(CCP_OFF, (uint8_t *)rom0, sizeof(rom0));
	printf("Load CCP_BDOS = %06lx, System size = %04x\r\n", (uint32_t)CCP_OFF, sizeof(rom0));

	/* BIOS */
	write_sram(BIOS_OFF, (uint8_t *)rom1, sizeof(rom1));
	printf("Load CBIOS = %06lx, System size = %04x\r\n", (uint32_t)BIOS_OFF, sizeof(rom1));

    if (menu_select() < 0) while (1);

	//
    // Start i8088
    //
    printf("Use PWM3  %.2f MHz for 8088 clock\n\r", 1/((PWM3PR+1)*P64)*1000);
    printf("\n\r");

    start_i88();

	board_event_loop();
	
}

void sys_init()
{
    board_init();
    board_sys_init();
}

int disk_init(void)
{
    if (f_mount(&fs, "0://", 1) != FR_OK) {
        printf("Failed to mount SD Card.\n\r");
        return -2;
    }

    return 0;
}

int menu_select(void)
{
    int i;
    unsigned int drive;

    //
    // Select disk image folder
    //
    if (f_opendir(&fsdir, "/")  != FR_OK) {
        printf("Failed to open SD Card..\n\r");
        return -3;
    }
 restart:
    i = 0;
    int selection = -1;
    f_rewinddir(&fsdir);
    while (f_readdir(&fsdir, &fileinfo) == FR_OK && fileinfo.fname[0] != 0) {
        if (strncmp(fileinfo.fname, "CPMDISKS", 8) == 0 ||
            strncmp(fileinfo.fname, "CPMDIS~", 7) == 0) {
            printf("%d: %s\n\r", i, fileinfo.fname);
            if (strcmp(fileinfo.fname, "CPMDISKS") == 0)
                selection = i;
            i++;
        }
    }

	if (1 < i) {
        printf("Select: ");
        while (1) {
            uint8_t c = (uint8_t)getch();  // Wait for input char
            if ('0' <= c && c <= '9' && c - '0' < i) {
                selection = c - '0';
                break;
            }
        	if ((c == 0x0d || c == 0x0a) && 0 <= selection)
                break;
        }
        printf("%d\n\r", selection);
        f_rewinddir(&fsdir);
        i = 0;
        while (f_readdir(&fsdir, &fileinfo) == FR_OK && fileinfo.fname[0] != 0) {
            if (strncmp(fileinfo.fname, "CPMDISKS", 8) == 0 ||
                strncmp(fileinfo.fname, "CPMDIS~", 7) == 0) {
                if (selection == i)
                    break;
                i++;
            }
        }
        printf("%s is selected.\n\r", fileinfo.fname);
    } else {
        strcpy(fileinfo.fname, "CPMDISKS");
    }
    f_closedir(&fsdir);

    //
    // Open disk images
    //
    for (drive = 0; drive < num_drives && num_files < NUM_FILES; drive++) {
        char drive_letter = (char)('A' + drive);
        char * const buf = (char *)tmp_buf[0];
        sprintf(buf, "%s/DRIVE%c.DSK", fileinfo.fname, drive_letter);
        if (f_open(&files[num_files], buf, FA_READ|FA_WRITE) == FR_OK) {
            printf("Image file %s/DRIVE%c.DSK is assigned to drive %c\n\r",
                   fileinfo.fname, drive_letter, drive_letter);
            drives[drive].filep = &files[num_files];
            num_files++;
        }
    }
    if (drives[0].filep == NULL) {
        printf("No boot disk.\n\r");
        return -4;
    }

    return 0;
}

void start_i88(void)
{
    board_start_i88();
}
