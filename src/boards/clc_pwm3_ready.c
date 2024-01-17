//
// Setup CLC
//
// PWM3 version
// use READY

// Setup CLC1,2,3,4,5,6,8
//
	//========== CLC pin assign ===========
    CLCIN0PPS = 0x03;			// assign RA3(A19)
    CLCIN1PPS = 0x05;			// assign RA5(HOLDA)
    CLCIN2PPS = 0x0b;			// assign RB3(ALE)
    CLCIN3PPS = 0x08;			// assign RB0(IO/M#)
    CLCIN6PPS = 0x09;			// assign RB1(WR#)
    CLCIN7PPS = 0x0a;			// assign RB2(RD#)
	
	//========== CLC1 :  reset signal for CLC4  ==========
	// normaly signal is [0] (G3POL=0)
	// [H] signal is maked by software (G3POL=1)
    CLCSELECT = 0;		// CLC1 select

    CLCnSEL0 = 127;		// NC
	CLCnSEL1 = 127;		// NC
	CLCnSEL2 = 127;		// NC
    CLCnSEL3 = 127;		// NC

	CLCnGLS0 = 0x00;	// input 0
	CLCnGLS1 = 0x00;	// input 0
	CLCnGLS2 = 0x00;	// input 0
    CLCnGLS3 = 0x00;	// input 0

    CLCnPOL = 0x0b;		// (POL, G3POL) = 0, (G1POL, G2POL, G4POL) = 1
    CLCnCON = 0x82;		// EN=1, 4 input AND
    
	//========== CLC2 :  sync CLC4 reset signal  ==========
    CLCSELECT = 1;		// CLC2 select

    CLCnSEL0 = 0x33;	// CLC1OUT
	CLCnSEL1 = 0x36;	// CLC4OUT
	CLCnSEL2 = 127;		// NC
    CLCnSEL3 = 127;		// NC

    CLCnGLS0 = 0x02;	// CLC1OUT (CLK)
	CLCnGLS1 = 0x00;	// NC: output=1(G2POL=1) : 1 -> D
    CLCnGLS2 = 0x08;	// CLC4OUT
    CLCnGLS3 = 0x00;	// NC

    CLCnPOL = 0x02;		// (G2POL) = 1, (POL, G4POL,G3POL,G1POL)=0
    CLCnCON = 0x84;		// DFF, no interrupt occurs

	G3POL = 1;
	G3POL = 0;			// reset DFF Q=0;

	//========== CLC3 :  CK_MSK  ==========
	// reset DFF with software(reset_io_read_mask();)
	// if HOLDA=1 then CLC3OUT=1
    CLCSELECT = 2;		// CLC3 select

    CLCnSEL0 = 7;		// CLCIN7PPS (RD#)
	CLCnSEL1 = 3;		// CLCIN3PPS (IO/#M)
	CLCnSEL2 = 1;		// CLCIN1PPS (HOLDA)
    CLCnSEL3 = 127;		// NC

	CLCnGLS0 = 0x02;	// #RD -> DFF(CLK)
	CLCnGLS1 = 0x24;	// not (not IO/#M) or (HOLDA)) ->DFF(D)
	CLCnGLS2 = 0x00;	// input 0
    CLCnGLS3 = 0x00;	// input 0

    CLCnPOL = 0x82;		// (POL, G2POL) = 1, (G4POL,G3POL,G1POL)=0
    CLCnCON = 0x84;		// EN=1, select DFF
    
	//========== CLC4 : READY ==========
	// reset DFF with software(reset_ready_pin();)
	// if HOLDA=1 then CLC4OUT=1
    CLCSELECT = 3;		// CLC4 select

    CLCnSEL0 = 2;		// CLCIN2PPS : RB3(ALE)
    CLCnSEL1 = 3;		// CLCIN3PPS : RB0(IO/M#)
	CLCnSEL2 = 1;		// CLCIN1PPS (HOLDA)
    CLCnSEL3 = 0x38;	// CLC6OUT

//    CLCnGLS0 = 0x01;	// ALE signal is inverted
    CLCnGLS0 = 0x02;	// ALE signal is not inverted
	CLCnGLS1 = 0x24;	// not (not IO/#M) or (HOLDA)) ->DFF(D)
    CLCnGLS2 = 0x80;	// CLC6OUT
    CLCnGLS3 = 0x00;	// not gated

    CLCnPOL = 0x82;		// (POL, G2POL) = 1, (G4POL,G3POL,G1POL)=0
    CLCnCON = 0x84;		// DFF, no interrupt occurs

	PPS(I88_READY) = 0x04;	// RB5(READY): CLC4_OUT

	//========== CLC5 : I/O Read/Write cycle ==========
	// CLCOUT5 = !((!RD# || !WR#) && IO/M && !HOLDA)
	CLCSELECT = 4;		// CLC5 select

	CLCnSEL0 = 3;		// CLCIN3PPS (IO/#M)
    CLCnSEL1 = 7;		// CLCIN7PPS (RD#)
    CLCnSEL2 = 6;		// CLCIN6PPS (WR#)
    CLCnSEL3 = 1;		// CLCIN1PPS (HOLDA)

    CLCnGLS0 = 0x02;	// IO/#M
	CLCnGLS1 = 0x14;	// (not RD#) or (not WR#)
    CLCnGLS2 = 0x40;	// not HOLDA
    CLCnGLS3 = 0x00;	// output = 0

    CLCnPOL = 0x88;		// (POL, G4POL)=1, (G3POL, G2POL, G1POL)=0
    CLCnCON = 0x82;		// 4 input AND

	//========== CLC6 : make reset signal for READY  ==========
    CLCSELECT = 5;		// CL6 select

	CLCnSEL0 = 0x26;	// PWM3S1P1_OUT (CLK)
    CLCnSEL1 = 0x34;	// CLC2OUT
	CLCnGLS2 = 0x00;	// input 0
    CLCnGLS3 = 0x00;	// input 0

    CLCnGLS0 = 0x01;	// CLK is inverted
	CLCnGLS1 = 0x08;	// CLC2OUT (not inverted)
    CLCnGLS2 = 0x00;	// not gated
    CLCnGLS3 = 0x00;	// not gated

    CLCnPOL = 0x00;		// All POL = 0
    CLCnCON = 0x84;		// DFF, no interrupt occurs

	//========== CLC8 8088 CLK  ==========
	CLCSELECT = 7;		// select CLC8  

	CLCnSEL0 = 0x08;	// Fosc
	CLCnSEL1 = 127;		// NC
	CLCnSEL2 = 0x35;	// CLC3OUT
	CLCnSEL3 = 127;		// NC

	CLCnGLS0 = 0x02;	// Fosc no invert
	CLCnGLS1 = 0x00;	// not gated
	CLCnGLS2 = 0x20;	// CLC3 no invert
	CLCnGLS3 = 0x00;	// not gated

	CLCnPOL = 0x0a;		// (POL, G1POL, G3POL)=0, (G2POL, G4POL)=1
	CLCnCON = 0x82;		// 4 input AND, no interrupt

