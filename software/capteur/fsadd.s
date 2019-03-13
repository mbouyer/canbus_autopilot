;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.4.0 #8981 (Mar 22 2015) (NetBSD)
; This file was generated Tue Mar 24 19:38:42 2015
;--------------------------------------------------------
; PIC16 port for the Microchip 16-bit core micros
;--------------------------------------------------------
;
; Hand-optimised version of fsadd

	list	p=18f452
	radix	dec


;--------------------------------------------------------
; public variables in this module
;--------------------------------------------------------
	global	___fsadd

;--------------------------------------------------------
;	Equates to used internal registers
;--------------------------------------------------------
STATUS	equ	0xfd8
WREG	equ	0xfe8
FSR0L	equ	0xfe9
FSR1L	equ	0xfe1
FSR2L	equ	0xfd9
POSTDEC1	equ	0xfe5
PREINC1	equ	0xfe4
PLUSW2	equ	0xfdb
PRODL	equ	0xff3
PRODH	equ	0xff4


; Internal registers
.registers	udata_ovr	0x0000
r0x00	res	1
r0x01	res	1
r0x02	res	1
r0x03	res	1
r0x04	res	1
r0x05	res	1
r0x06	res	1
r0x07	res	1
r0x0b	res	1
r0x0c	res	1
r0x0d	res	1
r0x0e	res	1
r0x0f	res	1

udata_fsadd_0	udata
___fsadd_fl1_1_22	res	4

udata_fsadd_1	udata
___fsadd_fl2_1_22	res	4

;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
; I code from now on!
; ; Starting pCode block
S_fsadd____fsadd	code
___fsadd:
;	.line	54; fsadd.c	__fsadd (float a1, float a2) _FS_REENTRANT
	MOVFF	FSR2L, POSTDEC1
	MOVFF	FSR1L, FSR2L
	MOVFF	r0x00, POSTDEC1
	MOVFF	r0x01, POSTDEC1
	MOVFF	r0x02, POSTDEC1
	MOVFF	r0x03, POSTDEC1
	MOVFF	r0x04, POSTDEC1
	MOVFF	r0x05, POSTDEC1
	MOVFF	r0x06, POSTDEC1
	MOVFF	r0x07, POSTDEC1
	MOVFF	r0x0b, POSTDEC1
	MOVFF	r0x0c, POSTDEC1
	MOVFF	r0x0d, POSTDEC1
	MOVFF	r0x0e, POSTDEC1
	MOVFF	r0x0f, POSTDEC1
;	.line	61; fsadd.c	fl1.f = a1;
	MOVLW	0x02
	MOVFF	PLUSW2, ___fsadd_fl1_1_22
	MOVLW	0x03
	MOVFF	PLUSW2, (___fsadd_fl1_1_22 + 1)
	MOVLW	0x04
	MOVFF	PLUSW2, (___fsadd_fl1_1_22 + 2)
	MOVLW	0x05
	MOVFF	PLUSW2, (___fsadd_fl1_1_22 + 3)
;	.line	62; fsadd.c	fl2.f = a2;
	MOVLW	0x06
	MOVFF	PLUSW2, ___fsadd_fl2_1_22
	MOVLW	0x07
	MOVFF	PLUSW2, (___fsadd_fl2_1_22 + 1)
	MOVLW	0x08
	MOVFF	PLUSW2, (___fsadd_fl2_1_22 + 2)
	MOVLW	0x09
	MOVFF	PLUSW2, (___fsadd_fl2_1_22 + 3)
;	.line	59; fsadd.c	unsigned long sign = 0;
	CLRF	r0x0b,   A
	BANKSEL	___fsadd_fl1_1_22
;	.line	65; fsadd.c	if (!fl1.l)
	MOVF	___fsadd_fl1_1_22, W, B
	IORWF	(___fsadd_fl1_1_22 + 1), W, B
	IORWF	(___fsadd_fl1_1_22 + 2), W, B
	IORWF	(___fsadd_fl1_1_22 + 3), W, B
	BNZ	_00106_DS_
;	.line	66; fsadd.c	return fl2.f;
	MOVFF	(___fsadd_fl2_1_22 + 3), FSR0L
	MOVFF	(___fsadd_fl2_1_22 + 2), PRODH
	MOVFF	(___fsadd_fl2_1_22 + 1), PRODL
	BANKSEL	___fsadd_fl2_1_22
	MOVF	___fsadd_fl2_1_22, W, B
	BRA	_00133_DS_
_00106_DS_:
	BANKSEL	___fsadd_fl2_1_22
;	.line	67; fsadd.c	if (!fl2.l)
	MOVF	___fsadd_fl2_1_22, W, B
	IORWF	(___fsadd_fl2_1_22 + 1), W, B
	IORWF	(___fsadd_fl2_1_22 + 2), W, B
	IORWF	(___fsadd_fl2_1_22 + 3), W, B
	BNZ	_00108_DS_
;	.line	68; fsadd.c	return fl1.f;
	MOVFF	(___fsadd_fl1_1_22 + 3), FSR0L
	MOVFF	(___fsadd_fl1_1_22 + 2), PRODH
	MOVFF	(___fsadd_fl1_1_22 + 1), PRODL
	BANKSEL	___fsadd_fl1_1_22
	MOVF	___fsadd_fl1_1_22, W, B
	BRA	_00133_DS_
_00108_DS_:
	BANKSEL	(___fsadd_fl1_1_22 + 2)
;	.line	70; fsadd.c	exp1 = EXP (fl1.l);
	RLCF	(___fsadd_fl1_1_22 + 2), W, B
	RLCF	(___fsadd_fl1_1_22 + 3), W, B
	MOVWF	r0x00,   A
	CLRF	r0x01,   A
	CLRF	r0x02,   A
	CLRF	r0x03,   A
	BANKSEL	(___fsadd_fl2_1_22 + 2)
;	.line	71; fsadd.c	exp2 = EXP (fl2.l);
	RLCF	(___fsadd_fl2_1_22 + 2), W, B
	RLCF	(___fsadd_fl2_1_22 + 3), W, B
	MOVWF	r0x02,   A
;	.line	73; fsadd.c	if (exp1 > exp2 + 25)
	ADDLW	0x19
	MOVWF	r0x04,   A
	MOVLW	0x00
	ADDWFC	r0x03, W
; #	MOVWF	r0x05,   A
; #	MOVF	r0x05, W
	ADDLW	0x80
	MOVWF	PRODL,   A
	MOVF	r0x01, W
	ADDLW	0x80
	SUBWF	PRODL, W
	BNZ	_00181_DS_
	MOVF	r0x00, W
	SUBWF	r0x04, W
_00181_DS_:
	BC	_00110_DS_
;	.line	74; fsadd.c	return fl1.f;
	MOVFF	(___fsadd_fl1_1_22 + 3), FSR0L
	MOVFF	(___fsadd_fl1_1_22 + 2), PRODH
	MOVFF	(___fsadd_fl1_1_22 + 1), PRODL
	BANKSEL	___fsadd_fl1_1_22
	MOVF	___fsadd_fl1_1_22, W, B
	BRA	_00133_DS_
_00110_DS_:
;	.line	75; fsadd.c	if (exp2 > exp1 + 25)
	MOVF	r0x00, W
	ADDLW	0x19
	MOVWF	r0x04,   A
	MOVLW	0x00
	ADDWFC	r0x01, W
; #	MOVWF	r0x05,   A
; #	MOVF	r0x05, W
	ADDLW	0x80
	MOVWF	PRODL,   A
	MOVF	r0x03, W
	ADDLW	0x80
	SUBWF	PRODL, W
	BNZ	_00182_DS_
	MOVF	r0x02, W
	SUBWF	r0x04, W
_00182_DS_:
	BC	_00112_DS_
;	.line	76; fsadd.c	return fl2.f;
	MOVFF	(___fsadd_fl2_1_22 + 3), FSR0L
	MOVFF	(___fsadd_fl2_1_22 + 2), PRODH
	MOVFF	(___fsadd_fl2_1_22 + 1), PRODL
	BANKSEL	___fsadd_fl2_1_22
	MOVF	___fsadd_fl2_1_22, W, B
	BRA	_00133_DS_
_00112_DS_:
	BANKSEL	___fsadd_fl1_1_22
;	.line	78; fsadd.c	mant1 = MANT (fl1.l);
	MOVF	___fsadd_fl1_1_22, W, B
	MOVWF	r0x04,   A
	MOVF	(___fsadd_fl1_1_22 + 1), W, B
	MOVWF	r0x05,   A
	MOVF	(___fsadd_fl1_1_22 + 2), W, B
	MOVWF	r0x06,   A
	CLRF	r0x07,   A
	BSF	r0x06, 7
	BANKSEL	___fsadd_fl2_1_22
;	.line	79; fsadd.c	mant2 = MANT (fl2.l);
	MOVF	___fsadd_fl2_1_22, W, B
	MOVWF	r0x0c,   A
	MOVF	(___fsadd_fl2_1_22 + 1), W, B
	MOVWF	r0x0d,   A
	MOVF	(___fsadd_fl2_1_22 + 2), W, B
	MOVWF	r0x0e,   A
	CLRF	r0x0f,   A
	BSF	r0x0e, 7
	BANKSEL	(___fsadd_fl1_1_22 + 3)
;	.line	81; fsadd.c	if (SIGN (fl1.l))
	MOVF	(___fsadd_fl1_1_22 + 3), W, B
	ANDLW	0x80
	BZ	_00114_DS_
;	.line	82; fsadd.c	mant1 = -mant1;
	COMF	r0x07, F
	COMF	r0x06, F
	COMF	r0x05, F
	NEGF	r0x04,   A
	BNZ	_00114_DS_
	INCF	r0x05, F
	BNZ	_00114_DS_
	INCF	r0x06, F
	BTFSC	STATUS, 2
	INCF	r0x07, F
_00114_DS_:
	BANKSEL	(___fsadd_fl2_1_22 + 3)
;	.line	83; fsadd.c	if (SIGN (fl2.l))
	MOVF	(___fsadd_fl2_1_22 + 3), W, B
	ANDLW	0x80
	BZ	_00116_DS_
;	.line	84; fsadd.c	mant2 = -mant2;
	COMF	r0x0f, F
	COMF	r0x0e, F
	COMF	r0x0d, F
	NEGF	r0x0c,   A
	BNZ	_00116_DS_
	INCF	r0x0d, F
	BNZ	_00116_DS_
	INCF	r0x0e, F
	BTFSC	STATUS, 2
	INCF	r0x0f, F
_00116_DS_:
;	.line	86; fsadd.c	if (exp1 > exp2)
	MOVF	r0x03, W
	ADDLW	0x80
	MOVWF	PRODL,   A
	MOVF	r0x01, W
	ADDLW	0x80
	SUBWF	PRODL, W
	BNZ	_00189_DS_
	MOVF	r0x00, W
	SUBWF	r0x02, W
_00189_DS_:
	BC	_00118_DS_
;	.line	88; fsadd.c	mant2 >>= exp1 - exp2;
	MOVF	r0x02, W
	SUBWF	r0x00, W
	BZ	_00119_DS_
	BN	_00193_DS_
	NEGF	WREG,   A
	BCF	STATUS, 0
_00191_DS_:
	BTFSC	r0x0f, 7
	BSF	STATUS, 0
	RRCF	r0x0f, F
	RRCF	r0x0e, F
	RRCF	r0x0d, F
	RRCF	r0x0c, F
	ADDLW	0x01
	BNC	_00191_DS_
	BRA	_00119_DS_
_00193_DS_:
	BCF	STATUS, 0
_00192_DS_:
	RLCF	r0x0c, F
	RLCF	r0x0d, F
	RLCF	r0x0e, F
	RLCF	r0x0f, F
	ADDLW	0x01
	BNC	_00192_DS_
_00190_DS_:
	BRA	_00119_DS_
_00118_DS_:
;	.line	92; fsadd.c	mant1 >>= exp2 - exp1;
	MOVF	r0x00, W
	SUBWF	r0x02, W
	BZ	_00194_DS_
	BN	_00197_DS_
	NEGF	WREG,   A
	BCF	STATUS, 0
_00195_DS_:
	BTFSC	r0x07, 7
	BSF	STATUS, 0
	RRCF	r0x07, F
	RRCF	r0x06, F
	RRCF	r0x05, F
	RRCF	r0x04, F
	ADDLW	0x01
	BNC	_00195_DS_
	BRA	_00194_DS_
_00197_DS_:
	BCF	STATUS, 0
_00196_DS_:
	RLCF	r0x04, F
	RLCF	r0x05, F
	RLCF	r0x06, F
	RLCF	r0x07, F
	ADDLW	0x01
	BNC	_00196_DS_
_00194_DS_:
;	.line	93; fsadd.c	exp1 = exp2;
	MOVFF	r0x02, r0x00
	MOVFF	r0x03, r0x01
_00119_DS_:
;	.line	95; fsadd.c	mant1 += mant2;
	MOVF	r0x0c, W
	ADDWF	r0x04, F
	MOVF	r0x0d, W
	ADDWFC	r0x05, F
	MOVF	r0x0e, W
	ADDWFC	r0x06, F
	MOVF	r0x0f, W
	ADDWFC	r0x07, F
;	.line	97; fsadd.c	if (mant1 < 0)
	BTFSS	r0x07, 7
	BRA	_00123_DS_
;	.line	99; fsadd.c	mant1 = -mant1;
	COMF	r0x07, F
	COMF	r0x06, F
	COMF	r0x05, F
	NEGF	r0x04,   A
	BNZ	_00198_DS_
	INCF	r0x05, F
	BNZ	_00198_DS_
	INCF	r0x06, F
	BTFSC	STATUS, 2
	INCF	r0x07, F
_00198_DS_:
;	.line	100; fsadd.c	sign = SIGNBIT;
	MOVLW	0x80
	MOVWF	r0x0b,   A
	BRA	_00125_DS_
_00123_DS_:
;	.line	102; fsadd.c	else if (!mant1)
	MOVF	r0x04, W
	IORWF	r0x05, W
	IORWF	r0x06, W
	IORWF	r0x07, W
	BNZ	_00125_DS_
;	.line	103; fsadd.c	return (0);
	CLRF	FSR0L,   A
	CLRF	PRODH,   A
	CLRF	PRODL,   A
	CLRF	WREG,   A
	BRA	_00133_DS_
_00125_DS_:
;	.line	106; fsadd.c	while (mant1 < HIDDEN)
	MOVLW	0x00
	SUBWF	r0x07, W
	BNZ	_00199_DS_
	MOVLW	0x80
	SUBWF	r0x06, W
	BNZ	_00199_DS_
	MOVLW	0x00
	SUBWF	r0x05, W
	BNZ	_00199_DS_
	MOVLW	0x00
	SUBWF	r0x04, W
_00199_DS_:
	BC	_00130_DS_
;	.line	108; fsadd.c	mant1 <<= 1;
	MOVF	r0x04, W
	ADDWF	r0x04, F
	RLCF	r0x05, F
	RLCF	r0x06, F
	RLCF	r0x07, F
;	.line	109; fsadd.c	--exp1;
	MOVLW	0xff
	ADDWF	r0x00, F
	ADDWFC	r0x01, F
	BRA	_00125_DS_
_00130_DS_:
;	.line	113; fsadd.c	while (mant1 & 0xff000000)
	MOVF	r0x07, W
	BZ	_00132_DS_
;	.line	115; fsadd.c	if (mant1&1)
	BTFSS	r0x04, 0
	BRA	_00129_DS_
;	.line	116; fsadd.c	mant1 += 2;
	MOVLW	0x02
	ADDWF	r0x04, F
	MOVLW	0x00
	ADDWFC	r0x05, F
	ADDWFC	r0x06, F
	ADDWFC	r0x07, F
_00129_DS_:
;	.line	117; fsadd.c	mant1 >>= 1 ;
	RLCF	r0x07, W
	RRCF	r0x07, F
	RRCF	r0x06, F
	RRCF	r0x05, F
	RRCF	r0x04, F
;	.line	118; fsadd.c	exp1++;
	INFSNZ	r0x00, F
	INCF	r0x01, F
	BRA	_00130_DS_
_00132_DS_:
;	.line	122; fsadd.c	mant1 &= ~HIDDEN;
	MOVLW	0x7f
	ANDWF	r0x06, F
;	.line	125; fsadd.c	fl1.l = PACK (sign, (unsigned long) exp1, mant1);
	CLRF	WREG,   A
	RRCF	r0x01, W
	RRCF	r0x00, W
	MOVWF	r0x0f,   A
	CLRF	r0x0e,   A
	RRCF	r0x0e, F
	MOVF	r0x0b, W
	IORWF	r0x0f, F
	MOVF	r0x06, W
	IORWF	r0x0e, F
	MOVF	r0x07, W
	IORWF	r0x0f, F
;	.line	127; fsadd.c	return fl1.f;
	MOVF	r0x0f, W
	MOVWF	FSR0L,   A
	MOVF	r0x0e, W
	MOVWF	PRODH,   A
	MOVF	r0x05, W
	MOVWF	PRODL,   A
	MOVF	r0x04, W
_00133_DS_:
	MOVFF	PREINC1, r0x0f
	MOVFF	PREINC1, r0x0e
	MOVFF	PREINC1, r0x0d
	MOVFF	PREINC1, r0x0c
	MOVFF	PREINC1, r0x0b
	MOVFF	PREINC1, r0x07
	MOVFF	PREINC1, r0x06
	MOVFF	PREINC1, r0x05
	MOVFF	PREINC1, r0x04
	MOVFF	PREINC1, r0x03
	MOVFF	PREINC1, r0x02
	MOVFF	PREINC1, r0x01
	MOVFF	PREINC1, r0x00
	MOVFF	PREINC1, FSR2L
	RETURN	



; Statistics:
; code size:	  942 (0x03ae) bytes ( 0.72%)
;           	  471 (0x01d7) words
; udata size:	    8 (0x0008) bytes ( 0.62%)
; access size:	   17 (0x0011) bytes


	end
