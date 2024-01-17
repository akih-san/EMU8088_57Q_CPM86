/*
 * UART, disk I/O and monitor firmware for SuperMEZ80-SPI
 *
 * Based on main.c by Tetsuya Suzuki and emuz80_z80ram.c by Satoshi Okue
 * Modified by @hanyazou https://twitter.com/hanyazou
 */

#include "../src/emu88.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../drivers/utils.h"

uint32_t mem_size = 0;

void mem_init()
{
    unsigned int i;
    uint32_t addr;

    // RAM check
    for (i = 0; i < TMP_BUF_SIZE; i += 2) {
        tmp_buf[0][i + 0] = 0xa5;
        tmp_buf[0][i + 1] = 0x5a;
    }
    for (addr = 0; addr < MAX_MEM_SIZE; addr += MEM_CHECK_UNIT) {
        printf("Memory 000000 - %06lXH\r", addr);
        tmp_buf[0][0] = (addr >>  0) & 0xff;
        tmp_buf[0][1] = (addr >>  8) & 0xff;
        tmp_buf[0][2] = (addr >> 16) & 0xff;

    	write_sram(addr, tmp_buf[0], TMP_BUF_SIZE);
        read_sram(addr, tmp_buf[1], TMP_BUF_SIZE);

    	if (memcmp(tmp_buf[0], tmp_buf[1], TMP_BUF_SIZE) != 0) {
            printf("\nMemory error at %06lXH\n\r", addr);
//            #ifdef TEST_DEBUG
            util_addrdump("WR: ", addr, tmp_buf[0], TMP_BUF_SIZE);
            util_addrdump("RD: ", addr, tmp_buf[1], TMP_BUF_SIZE);
//            #endif
			while(1){};		// stop
            break;
        }
        if (addr == 0) continue;

    	read_sram(0, tmp_buf[1], TMP_BUF_SIZE);
        if (memcmp(tmp_buf[0], tmp_buf[1], TMP_BUF_SIZE) == 0) {
            // if the page at addr is the same as the first page,
			// then addr reachs end of memory
            break;
        }
    }
	mem_size = addr;
	printf("Memory 000000 - %06lXH %d KB OK\r\n", addr, (int)(mem_size / 1024));
}
