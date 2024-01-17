/*
 * UART, disk I/O and monitor firmware for SuperMEZ80-SPI
 *
 * Based on main.c by Tetsuya Suzuki and emuz80_z80ram.c by Satoshi Okue
 * Modified by @hanyazou https://twitter.com/hanyazou
 */
#ifndef __SUPERMEZ80_H__
#define __SUPERMEZ80_H__

#include "../src/picconfig.h"
#include <xc.h>
#include <stdint.h>
#include "../fatfs/ff.h"

//
// Configlations
//

//#define TEST_DEBUG
#define USE_READY
#define USE_PWM3

#ifndef USE_READY
#define USE_T2
#endif

#ifndef USE_PWM3
#define USE_NCO2
#endif


#define P64 15.625

#define ENABLE_DISK_DEBUG

#define NUM_FILES        6
#define SECTOR_SIZE      128
#define TMP_BUF_SIZE     256

#define MEM_CHECK_UNIT   TMP_BUF_SIZE * 16 // 4 KB
#define MAX_MEM_SIZE     0x00100000        // 1 MB

//
// Constant value definitions
//

#define UART_DREG			0x00	// 00h Data REG
#define UART_CREG			0x02	// 00h Control REG
#define DISK_REG_DRIVE		0x10	// 10h fdc-port: # of drive
#define DISK_REG_TRACK		0x12	// 12h fdc-port: # of track
#define DISK_REG_SECTOR		0x14	// 14h fdc-port: # of sector(0-7bit)
#define DISK_REG_SECTORH	0x15	// 15h fdc-port: # of sector high(8-15bit)

#define DISK_REG_FDCOP	0x20	// 20h fdc-port: command
#define DISK_REG_FDCST	0x22	// 22h fdc-port: status

#define DISK_REG_DMALL	0x24	// 24h dma-port: dma address 0-7bit
#define DISK_REG_DMALH	0x25	// 25h dma-port: dma address 8-15bit
#define DISK_REG_DMAHL	0x26	// 26h dma-port: dma address 16-23bit
#define DISK_REG_DMAHH	0x27	// 27h dma-port: dma address 24-31bit

#define DISK_OP_READ	0
#define DISK_OP_WRITE	1

#define NMI_SIG_OFF		0x30	// 30h NMI signal off (LED OFF)

#define CTL_Q 0x11

// Status register
// bit 76543210
//     WR....EB
//
// bit 7 : 1=Write operation
// bit 6 : 1=Read operation
// bit 1 : 1=Error
// bit 0 : 1=Busy, 0=Ready

#define DISK_ST_READY		0x00
#define DISK_ST_BUSY		0x01
#define DISK_ST_ERROR		0x02
#define DISK_ST_READ		0x40
#define DISK_ST_WRITE		0x80

#define CCP_OFF        0x400
#define BIOS_OFF       CCP_OFF + 0x2500
#define START_VEC      0xffff0

//
// Type definitions
//

// Address Bus
union address_bus_u {
    uint32_t w;             // 32 bits Address
    struct {
        uint8_t ll;        // Address L low
        uint8_t lh;        // Address L high
        uint8_t hl;        // Address H low
        uint8_t hh;        // Address H high
    };
};

typedef struct {
    unsigned int sectors;
    FIL *filep;
} drive_t;

typedef struct {
    uint8_t disk;
    uint8_t disk_read;
    uint8_t disk_write;
    uint8_t disk_verbose;
    uint16_t disk_mask;
} debug_t;

typedef struct {
    uint8_t *addr;
    uint16_t offs;
    unsigned int len;
} mem_region_t;

//
// Global variables and function prototypes
//

extern uint8_t tmp_buf[2][TMP_BUF_SIZE];
extern debug_t debug;
extern int do_bus_master;

// io

/*
enum {
    IO_STAT_INVALID       = 0,
    IO_STAT_NOT_STARTED   = 10,
    IO_STAT_RUNNING       = 20,
    IO_STAT_READ_WAITING  = 30,
    IO_STAT_INTR_WAITING  = 35,
    IO_STAT_WRITE_WAITING = 40,
    IO_STAT_STOPPED       = 50,
    IO_STAT_RESUMING      = 60,
    IO_STAT_INTERRUPTED   = 70,
    IO_STAT_PREPINVOKE    = 80,
    IO_STAT_MONITOR       = 90
};
*/

extern void reset_ready_pin(void);
extern void io_init(void);
extern int io_stat(void);
extern int getch(void);
extern drive_t drives[];
extern const int num_drives;

extern void mem_init(void);

// board

extern void board_init(void);
extern void (*board_sys_init_hook)(void);
#define board_sys_init() (*board_sys_init_hook)()
extern void (*bus_master_hook)(int enable);
#define bus_master(enable) (*bus_master_hook)(enable)
extern void (*board_start_i88_hook)(void);
#define board_start_i88() (*board_start_i88_hook)()

extern void write_sram(uint32_t addr, uint8_t *buf, unsigned int len);
extern void read_sram(uint32_t addr, uint8_t *buf, unsigned int len);
extern void reset_io_read_mask(void);
extern void reset_t2_mask(void);
extern void end_io_read(void);
extern void end_io_write(void);
extern void set_hold_pin(void);
extern void reset_hold_pin(void);
extern void board_event_loop(void);
extern void io_rd_wr(void);
extern void bus_master_operation(void);

// Address read and write
extern uint8_t (*board_addr_l_pins_hook)(void);
#define addr_l_pins() (*board_addr_l_pins_hook)()
extern void (*board_set_addr_l_pins_hook)(uint8_t);
#define set_addr_l_pins(v) (*board_set_addr_l_pins_hook)(v)

// Data read and write
extern uint8_t (*board_data_pins_hook)(void);
#define data_pins() (*board_data_pins_hook)()
extern void (*board_set_data_pins_hook)(uint8_t);
#define set_data_pins(v) (*board_set_data_pins_hook)(v)
extern void (*board_set_data_dir_hook)(uint8_t);
#define set_data_dir(v) (*board_set_data_dir_hook)(v)

// RD    read only
extern __bit (*board_rd_pin_hook)(void);
#define rd_pin() (board_rd_pin_hook?(*board_rd_pin_hook)():1)
extern __bit (*board_wr_pin_hook)(void);
#define wr_pin() (board_wr_pin_hook?(*board_wr_pin_hook)():1)

//NMI
extern void (*board_nmi_sig_off_hook)(void);
#define nmi_sig_off() (*board_nmi_sig_off_hook)()

extern void (*board_nmi_sig_on_hook)(void);
#define nmi_sig_on() (*board_nmi_sig_on_hook)()

//
// debug macros
//
#ifdef ENABLE_DISK_DEBUG
#define DEBUG_DISK (debug.disk || debug.disk_read || debug.disk_write || debug.disk_verbose)
#define DEBUG_DISK_READ (debug.disk_read)
#define DEBUG_DISK_WRITE (debug.disk_write)
#define DEBUG_DISK_VERBOSE (debug.disk_verbose)
#else
#define DEBUG_DISK 0
#define DEBUG_READ 0
#define DEBUG_WRITE 0
#define DEBUG_DISK_VERBOSE 0
#endif

#endif  // __SUPERMEZ80_H__
