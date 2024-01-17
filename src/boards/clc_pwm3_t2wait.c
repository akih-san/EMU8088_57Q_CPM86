//
// Setup CLC
//
// PWM3 version
// don't use READY
//
// Setup CLC2,3,5,7,8
//
	//========== CLC pin assign ===========
    CLCIN0PPS = 0x03;			// assign RA3(A19)
    CLCIN1PPS = 0x05;			// assign RA5(HOLDA)
    CLCIN2PPS = 0x0b;			// assign RB3(ALE)
    CLCIN3PPS = 0x08;			// assign RB0(IO/M#)
    CLCIN6PPS = 0x09;			// assign RB1(WR#)
    CLCIN7PPS = 0x0a;			// assign RB2(RD#)
	

	//========== CLC2 : T2 mask ==========
	// reset DFF with software(reset_t2_mask();)
	// if HOLDA=1 then CLC2OUT=1
	CLCSELECT = 1;		// CLC2 select

	CLCnSEL0 = 0x26;	// PWM3S1P1_OUT
	CLCnSEL2 = 3;		// CLCIN3PPS (IO/#M)
    CLCnSEL1 = 0x39;	// CLC7OUT (ALE1)
    CLCnSEL3 = 1;		// CLCIN1PPS (HOLDA)

    CLCnGLS0 = 0x01;	// not NCO2/PWM3 ->DFF(CLK)
	CLCnGLS1 = 0x94;	// not((not ALE1) or (not IO/#M) or HOLDA) ->DFF(D)
    CLCnGLS2 = 0x00;	// not gated
    CLCnGLS3 = 0x00;	// not gated

    CLCnPOL = 0x82;		// (POL, G2POL) = 1, (G4POL,G3POL,G1POL)=0
    CLCnCON = 0x84;		// DFF, no interrupt occurs

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

	//========== CLC7 : delayed ALE : ALE1 ==========
    CLCSELECT = 6;		// CLC7 select
	// if HOLDA=1 then CLC7OUT=0

	CLCnSEL0 = 0x26;	// PWM3
	CLCnSEL1 = 2;		// CLCIN2PPS : RB3(ALE)
	CLCnSEL2 = 1;		// CLCIN1PPS (HOLDA)
    CLCnSEL3 = 127;		// NC

	CLCnGLS0 = 0x02;	// NCO2
	CLCnGLS1 = 0x08;	// ALE (no invert)
	CLCnGLS2 = 0x20;	// HOLDA
    CLCnGLS3 = 0x00;	// not gated

    CLCnPOL = 0x00;		// all POL = 0
    CLCnCON = 0x84;		// EN=1, select DFF
	
//========== CLC8 8088 CLK  ==========
	CLCSELECT = 7;		// select CLC8  

	CLCnSEL0 = 0x08;	// Fosc
	CLCnSEL1 = 0x34;	// CLC2OUT
	CLCnSEL2 = 0x35;	// CLC3OUT
	CLCnSEL3 = 127;		// NC

	CLCnGLS0 = 0x02;	// Fosc no invert
	CLCnGLS1 = 0x08;	// CLC2 no invert
	CLCnGLS2 = 0x20;	// CLC3 no invert
	CLCnGLS3 = 0x00;	// not gated

	CLCnPOL = 0x08;		// (POL, G1POL, G2POL, G3POL)=0, G4POL=1
	CLCnCON = 0x82;		// 4 input AND, no interrupt
