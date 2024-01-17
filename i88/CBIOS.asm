	page 0
	CPU	8086

	title	'Customized Basic I/O System'

;*********************************************
;*                                           *
;* This Customized BIOS adapts CP/M-86 to    *
;* the following hardware configuration      *
;*     Processor:                            *
;*     Brand:                                *
;*     Controller:                           *
;*                                           *
;*                                           *
;*     Programmer: Akihito Honda             *
;*     Revisions : 1.0                       *
;*     Date : 2023.12.221                    *
;*                                           *
;*********************************************

cr		equ 0dh ;carriage return
lf		equ 0ah ;line feed

UNIMON = 1
UNI_SEG		equ	0ff50h
UNI_OFF		equ	0ff30h	; stask f300-f3ff, work f400-f4ff, code f500

bdos_int	equ 224 ;reserved BDOS interrupt

;---------------------------------------------
;|                                           |
bios_code	equ 2500h
ccp_offset	equ 0000h
bdos_ofst	equ 0B06h ;BDOS entry point
;|                                           |
;---------------------------------------------

	ASSUME	CS:CODE, DS:DATA, SS:DATA, ES:NOTHING

	SEGMENT	CODE
;	cseg
	org	ccp_offset
ccp:
	org	bios_code

;*********************************************
;*                                           *
;* BIOS Jump Vector for Individual Routines  *
;*                                           *
;*********************************************

	jmp	INIT		;Enter from BOOT ROM or LOADER
	jmp	WBOOT		;Arrive here from BDOS call 0  
	jmp	CONST		;return console keyboard status
	jmp	CONIN		;return console keyboard char
	jmp	CONOUT  	;write char to console device
	jmp	LISTOUT		;write character to list device
	jmp	PUNCH		;write character to punch device
	jmp	READER  	;return char from reader device 
	jmp	HOME		;move to trk 00 on cur sel drive
	jmp	SELDSK  	;select disk for next rd/write
	jmp	SETTRK  	;set track for next rd/write
	jmp	SETSEC  	;set sector for next rd/write
	jmp	SETDMA  	;set offset for user buff (DMA)
	jmp	READ		;read a 128 byte sector
	jmp	WRITE		;write a 128 byte sector
	jmp	LISTST  	;return list status 
	jmp	SECTRAN 	;xlate logical->physical sector 
	jmp	SETDMAB 	;set seg base for buff (DMA)
	jmp	GETSEGT 	;return offset of Mem Desc Table
	jmp	GETIOBF		;return I/O map byte (IOBYTE)
	jmp	SETIOBF		;set I/O map byte (IOBYTE) 

	; for make near jump table
n_jmp	equ	20
	db	128 - n_jmp*3 dup(90h)	; nop

;*********************************************
;*                                           *
;* INIT Entry Point, Differs for LDBIOS and  *
;* BIOS, according to "Loader_Bios" value    *
;*                                           *
;*********************************************

INIT:	;print signon message and initialize hardware
	mov	ax,cs		;we entered with a JMPF so use
	mov	ss,ax		;CS: as the initial value of SS:,
	mov	ds,ax		;DS:,
	mov	es,ax		;and ES:
	;use local stack during initialization
	mov	sp,stkbase
	cld			;set forward direction

;---------------------------------------------
;|                                           |
	; This is a BIOS for the CPM.SYS file.
	; Setup all interrupt vectors in low
	; memory to address trap

	push	ds		;save the DS register
	mov	[IOBYTE],0	;clear IOBYTE
	mov	ax,0
	mov	ds,ax
	mov	es,ax 		;set ES and DS to zero

	if UNIMON = 0
	;setup interrupt 0 to address trap routine
	mov	[int0_offset],int_trap
	mov	[int0_segment],CS
	mov	di,4
	mov	si,0		;then propagate
	mov	cx,510		;trap vector to

;	rep movs ax,ax	;all 256 interrupts
	rep movsw		;all 256 interrupts
	endif

	;BDOS offset to proper interrupt
	mov	[bdos_offset],bdos_ofst
	mov	[bdos_segment],CS
	pop	ds		;restore the DS register

;	(additional CP/M-86 initialization)
;|                                           |
;---------------------------------------------

	mov	bx,signon
	call	pmsg		;print signon message
	mov	cl,0		;default to dr A: on coldstart
	jmp	ccp		;jump to cold start entry of CCP

WBOOT:	jmp	ccp+6		;direct entry to CCP at command level

;---------------------------------------------
;|                                           |
int_trap:
	cli			;block interrupts
	mov	ax,cs
	mov	ds,ax		;get our data segment
	mov	bx,int_trp
	call	pmsg
	hlt			;hardstop
;|                                           |
;---------------------------------------------

;*********************************************
;*                                           *
;*   CP/M Character I/O Interface Routines   *
;*                                           *
;*********************************************

;;;
;;; PIC18F57Qxx
;;;

USARTD:	EQU	00H
USARTC:	EQU	02H

CONST:	;console status
	IN	AL,USARTC
	AND	AL,01h		; check U3RXIF
	jz	non_key
	or	al,255		;return non-zero if RDA
	RET
non_key:
	ret

CONIN: ;console input
	call	CONST
	jz	CONIN		;wait for RDA
	IN	AL,USARTD
	and	al,7fh 		;read data and remove parity bit
	ret

CONOUT:		;console output
	IN	AL,USARTC
	and	al, 02h		; check U3TXIF
	JZ	CONOUT
	mov	al,cl
	OUT	USARTD,AL
	RET

LISTOUT:		;list device output
;	rs	10	;(fill-in)
	xor	al, al
	ret

LISTST:			;poll list status
;	rs	10	;(fill-in)
	xor	al, al
	ret

PUNCH:		;write punch device
;	rs	10	;(fill-in)
	xor	al, al
	ret

READER:
;	rs	10	;(fill-in)
	xor	al, al
	ret

GETIOBF:
	mov	al,[IOBYTE]
	ret

SETIOBF:
	mov	[IOBYTE],cl	;set iobyte
return:
	ret			;iobyte not implemented

pmsg:
	mov	al,[BX] 	;get next char from message
	test	al,al
	jz	return		;if zero return
	mov	CL,AL
	call	CONOUT  	;print it
	inc	BX
	jmp	pmsg		;next character and loop

;*********************************************
;*                                           *
;*          Disk Input/Output Routines       *
;*                                           *
;*********************************************

UART_DREG		equ	00h	; 00h Data REG
UART_CREG		equ	02h	; 00h Control REG
DISK_REG_DRIVE		equ	10h	; 10h fdc-port: # of drive
DISK_REG_TRACK		equ	12h	; 12h fdc-port: # of track
DISK_REG_SECTOR		equ	14h	; 14h fdc-port: # of sector(0-7bit)
DISK_REG_SECTORH	equ	15h	; 15h fdc-port: # of sector high(8-15bit)

DISK_REG_FDCOP	equ	20h	; 20h fdc-port: command
DISK_REG_FDCST	equ	22h	; 22h fdc-port: status

DISK_REG_DMALL	equ	24h	; 24h dma-port: dma address 0-7bit
DISK_REG_DMALH	equ	25h	; 25h dma-port: dma address 8-15bit
DISK_REG_DMAHL	equ	26h	; 16h dma-port: dma address 16-23bit
DISK_REG_DMAHH	equ	27h	; 17h dma-port: dma address 24-31bit

DISK_OP_READ	equ	0
DISK_OP_WRITE	equ	1

; Status register
; bit 76543210
;     WR....EB
;
; bit 7 : 1=Write operation
; bit 6 : 1=Read operation
; bit 1 : 1=Error
; bit 0 : 1=Busy, 0=Ready

DISK_ST_READY	equ	00h
DISK_ST_BUSY	equ	01h
DISK_ST_ERROR	equ	02h
DISK_ST_READ	equ	40h
DISK_ST_WRITE	equ	80h

SELDSK:	;select disk given by register CL
;	mov	[disk],cl	;save disk number
	mov	al,cl		;save disk number

	cmp	al, 4
	jc	SELFD

	mov	bx,HDB1		;dph harddisk 1
	cmp	al, 8
	jz	SELHD

	mov	bx,HDB2		;dph harddisk 2
	cmp	al, 9
	jz	SELHD

	mov	bx,0000h	;ready for error return
	ret
SELFD:
	mov	ch,0		;double(n)
	mov	bx,cx		;bx = n
	mov	cl,4		;ready for *16
	shl	bx,cl		;n = n * 16
	mov	cx,dpbase
	add	bx,cx		;dpbase + n * 16
SELHD:
	out	DISK_REG_DRIVE, al	; set drive to PIC
	RET
	

HOME:	;move selected disk to home position (Track 0)
	xor	al, al
	jmp	set_home

SETTRK: ;set track address given by CX
	mov	al, cl
set_home:
	out	DISK_REG_TRACK, al
	ret

SETSEC: ;set sector number given by cx
	mov	ax, cx
	out	DISK_REG_SECTOR, ax
	ret

SECTRAN: ;translate sector CX using table at [DX]
	or	dx, dx
	jz	no_skew
	mov	bx,cx
	add	bx,dx		;add sector to tran table address
	mov	bl,[bx]		;get logical sector
	mov	bh,0
	ret

no_skew:
	mov	bx, cx
	inc	bx		; ;sector no. start with 1
	ret

SETDMA: ;set DMA offset given by CX
	mov	ax, cx
	out	DISK_REG_DMALL, ax	; write 16bit data to PIC
	ret

SETDMAB: ;set DMA segment given by CX
	mov	ax, cx
	out	DISK_REG_DMAHL, ax	; write 16bit data to PIC
	ret
;
GETSEGT:  ;return address of physical memory table
	mov	bx,segtable
	ret

;*********************************************
;*                                           *
;*  All disk I/O parameters are setup:       *
;*     DISK     is disk number      (SELDSK) *
;*     TRK      is track number     (SETTRK) *
;*     SECT     is sector number    (SETSEC) *
;*     DMA_ADR  is the DMA offset   (SETDMA) *
;*     DMA_SEG  is the DMA segment  (SETDMAB)*
;*  READ reads the selected sector to the DMA*
;*  address, and WRITE writes the data from  *
;*  the DMA address to the selected sector   *
;*  (return 00 if successful,  01 if perm err)*
;*                                           *
;*********************************************

READ:
	in	al, DISK_REG_FDCST	; check status
	and	al, DISK_ST_BUSY
	jnz	read			; wait until READY

	mov	al, DISK_OP_READ	; read sector (DMA)
	out	DISK_REG_FDCOP, al	;
READ_W:
	in	al, DISK_REG_FDCST	; read status
	and	al, DISK_ST_BUSY
	jnz	READ_W

	and	al, DISK_ST_ERROR
	jz	rw_ok
;error
	mov	al, 1
	or	al, al
	ret
rw_ok:
	xor	al, al
	ret

WRITE:
	in	al, DISK_REG_FDCST	; check status
	and	al, DISK_ST_BUSY
	jnz	WRITE			; wait until READY

	mov	al, DISK_OP_WRITE
	out	DISK_REG_FDCOP, al	; set write command
	jmp	READ_W

;*********************************************
;*                                           *
;*               Data Areas                  *
;*                                           *
;*********************************************
data_offset	equ $

	SEGMENT	DATA
	org	data_offset	;contiguous with code segment

IOBYTE	db	0

;---------------------------------------------
;|                                           |
signon	db	cr,lf,cr,lf
	db	"CP/M-86 BIOS Generated!",cr,lf
	db	"EMU8088/V20_RAM edition. 2023.12"
	db	cr,lf,0
;|                                           |
;---------------------------------------------

int_trp	db	cr,lf
	db	'Interrupt Trap Halt'
	db	cr,lf

;	System Memory Segment Table

segtable	db	1	;
	dw tpa_seg	;1st seg starts after BIOS
	dw tpa_len	;and extends

;	include singles.lib ;read in disk definitions

;---------- 4 DISKS --------------------
dpbase	equ	$		;Base of Disk Parameter Blocks
dpe0	dw	xlt0,0000h	;Translate Table
	dw	0000h,0000h	;Scratch Area
	dw	dirbuf,dpb0	;Dir Buff, Parm Block
	dw	csv0,alv0	;Check, Alloc Vectors

dpe1	dw	xlt1,0000h	;Translate Table
	dw	0000h,0000h	;Scratch Area
	dw	dirbuf,dpb1	;Dir Buff, Parm Block
	dw	csv1,alv1	;Check, Alloc Vectors

dpe2	dw	xlt2,0000h	;Translate Table
	dw	0000h,0000h	;Scratch Area
	dw	dirbuf,dpb2	;Dir Buff, Parm Block
	dw	csv2,alv2	;Check, Alloc Vectors

dpe3	dw	xlt3,0000h	;Translate Table
	dw	0000h,0000h	;Scratch Area
	dw	dirbuf,dpb3	;Dir Buff, Parm Block
	dw	csv3,alv3	;Check, Alloc Vectors

;	        DISKDEF 0,1,26,6,1024,243,64,64,2
;
;	 1944:	128 Byte Record Capacity
;	  243:	Kilobyte Drive  Capacity
;	   64:	32 Byte Directory Entries
;	   64:	Checked Directory Entries
;	  128:	Records / Extent
;	    8:	Records / Block
;	   26:	Sectors / Track
;	    2:	Reserved  Tracks
;	    6:	Sector Skew Factor
;
dpb0	equ	$		;Disk Parameter Block
	dw	26		;Sectors Per Track
	db	3		;Block Shift
	db	7		;Block Mask
	db	0		;Extnt Mask
	dw	242		;Disk Size - 1
	dw	63		;Directory Max
	db	192		;Alloc0
	db	0		;Alloc1
	dw	16		;Check Size
	dw	2		;Offset

xlt0	equ	$		;Translate Table
	db	1,7,13,19
	db	25,5,11,17
	db	23,3,9,15
	db	21,2,8,14
	db	20,26,6,12
	db	18,24,4,10
	db	16,22
als0	equ	31		;Allocation Vector Size
css0	equ	16		;Check Vector Size

;	        DISKDEF 1,0
;
;	Disk 1 - 3  are the same as Disk 0
;
dpb1	equ	dpb0		;Equivalent Parameters
dpb2	equ	dpb0		;Equivalent Parameters
dpb3	equ	dpb0		;Equivalent Parameters
als1	equ	als0		;Same Allocation Vector Size
als2	equ	als0		;Same Allocation Vector Size
als3	equ	als0		;Same Allocation Vector Size
css1	equ	css0		;Same Checksum Vector Size
css2	equ	css0		;Same Checksum Vector Size
css3	equ	css0		;Same Checksum Vector Size
xlt1	equ	xlt0		;Same Translate Table
xlt2	equ	xlt0		;Same Translate Table
xlt3	equ	xlt0		;Same Translate Table
;	        ENDEF
;
;	fixed data tables for 4MB harddisks
;
;	disk parameter header
HDB1:	DW	0000H,0000H
	DW	0000H,0000H
	DW	dirbuf,HDBLK
	DW	CHKHD1,ALLHD1
HDB2:	DW	0000H,0000H
	DW	0000H,0000H
	DW	dirbuf,HDBLK
	DW	CHKHD2,ALLHD2
;
;       disk parameter block for harddisk
;
;HDBLK:	DW	32		;SEC PER TRACK
;	DB	4		;BLOCK SHIFT
;	DB	15		;BLOCK MASK
;	DB	0		;EXTNT MASK
;	DW	2047		;DISK SIZE-1
;	DW	255		;DIRECTORY MAX
;	DB	240		;ALLOC0
;	DB	0		;ALLOC1
;	DW	0		;CHECK SIZE
;	DW	0		;OFFSET

HDBLK:  DW    128		;sectors per track
	DB    4			;block shift factor
	DB    15		;block mask
	DB    0			;extent mask
	DW    2039		;disk size-1
	DW    1023		;directory max
	DB    255		;alloc 0
	DB    255		;alloc 1
	DW    0			;check size
	DW    0			;track offset

alshd1	equ	255		;Allocation Vector Size
;alshd1	equ	32		;Allocation Vector Size
csshd1	equ	0		;Check Vector Size
alshd2	equ	alshd1		;Allocation Vector Size
csshd2	equ	csshd1		;Check Vector Size

;
;	Uninitialized Scratch Memory Follows:
;
begdat	equ	$		;Start of Scratch Area
dirbuf	ds	128		;Directory Buffer
alv0	ds	als0		;Alloc Vector
csv0	ds	css0		;Check Vector
alv1	ds	als1		;Alloc Vector
csv1	ds	css1		;Check Vector
alv2	ds	als2		;Alloc Vector
csv2	ds	css2		;Check Vector
alv3	ds	als3		;Alloc Vector
csv3	ds	css3		;Check Vector
ALLHD1:	DS	alshd1		;allocation vector harddisk 1
ALLHD2:	DS	alshd2		;allocation vector harddisk 2
CHKHD1:	equ	$		;check vector harddisk 1 (0)
CHKHD2:	equ	$		;check vector harddisk 2 (0)
enddat	equ	$		;End of Scratch Area

datsiz	equ	enddat - begdat	;Size of Scratch Area

	db	0		;Marks End of Module

loc_stk	dw  32 dup(?)		;local stack for initialization

stkbase	equ	$
lastoff	equ	$
	db 0	;fill last address for GENCMD

tpa_seg	equ (lastoff+0400h+15) / 16

	if UNIMON = 1
tpa_len	equ UNI_OFF - tpa_seg
	else
tpa_len	equ 10000h - tpa_seg
	endif

;*********************************************
;*                                           *
;*          Dummy Data Section               *
;*                                           *
;*********************************************
	SEGMENT	DATA
	org 	0	;(interrupt vectors)

int0_offset	dw	?
int0_segment	dw	?
;	pad to system call vector
	ds	4*(bdos_int-1)

bdos_offset	dw	?
bdos_segment	dw	?
	END
