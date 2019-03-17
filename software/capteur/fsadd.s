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

.privatefp	udata

r1x00	res	1
r1x01	res	1
r1x02	res	1
r1x03	res	1
r1x04	res	1
r1x05	res	1
r1x06	res	1
r1x07	res	1
r1x0b	res	1
r1x0c	res	1
r1x0d	res	1
r1x0e	res	1
r1x0f	res	1
___fsxxx_fl1	res	4
___fsxxx_fl2	res	4

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
	BANKSEL r1x00
;	.line	61; fsadd.c	fl1.f = a1;
	MOVLW	0x02
	MOVFF	PLUSW2, ___fsxxx_fl1
	MOVLW	0x03
	MOVFF	PLUSW2, (___fsxxx_fl1 + 1)
	MOVLW	0x04
	MOVFF	PLUSW2, (___fsxxx_fl1 + 2)
	MOVLW	0x05
	MOVFF	PLUSW2, (___fsxxx_fl1 + 3)
;	.line	62; fsadd.c	fl2.f = a2;
	MOVLW	0x06
	MOVFF	PLUSW2, ___fsxxx_fl2
	MOVLW	0x07
	MOVFF	PLUSW2, (___fsxxx_fl2 + 1)
	MOVLW	0x08
	MOVFF	PLUSW2, (___fsxxx_fl2 + 2)
	MOVLW	0x09
	MOVFF	PLUSW2, (___fsxxx_fl2 + 3)
;	.line	59; fsadd.c	unsigned long sign = 0;
	CLRF	r1x0b, B
;	.line	65; fsadd.c	if (!fl1.l)
	MOVF	___fsxxx_fl1, W, B
	IORWF	(___fsxxx_fl1 + 1), W, B
	IORWF	(___fsxxx_fl1 + 2), W, B
	IORWF	(___fsxxx_fl1 + 3), W, B
	BNZ	_00106_DS_
;	.line	66; fsadd.c	return fl2.f;
	MOVFF	(___fsxxx_fl2 + 3), FSR0L
	MOVFF	(___fsxxx_fl2 + 2), PRODH
	MOVFF	(___fsxxx_fl2 + 1), PRODL
	MOVF	___fsxxx_fl2, W, B
	BRA	_00133_DS_
_00106_DS_:
;	.line	67; fsadd.c	if (!fl2.l)
	MOVF	___fsxxx_fl2, W, B
	IORWF	(___fsxxx_fl2 + 1), W, B
	IORWF	(___fsxxx_fl2 + 2), W, B
	IORWF	(___fsxxx_fl2 + 3), W, B
	BNZ	_00108_DS_
;	.line	68; fsadd.c	return fl1.f;
	MOVFF	(___fsxxx_fl1 + 3), FSR0L
	MOVFF	(___fsxxx_fl1 + 2), PRODH
	MOVFF	(___fsxxx_fl1 + 1), PRODL
	MOVF	___fsxxx_fl1, W, B
	BRA	_00133_DS_
_00108_DS_:
;	.line	70; fsadd.c	exp1 = EXP (fl1.l);
	RLCF	(___fsxxx_fl1 + 2), W, B
	RLCF	(___fsxxx_fl1 + 3), W, B
	MOVWF	r1x00, B
	CLRF	r1x01, B
	CLRF	r1x02, B
	CLRF	r1x03, B
;	.line	71; fsadd.c	exp2 = EXP (fl2.l);
	RLCF	(___fsxxx_fl2 + 2), W, B
	RLCF	(___fsxxx_fl2 + 3), W, B
	MOVWF	r1x02, B
;	.line	73; fsadd.c	if (exp1 > exp2 + 25)
	ADDLW	0x19
	MOVWF	r1x04, B
	MOVLW	0x00
	ADDWFC	r1x03, W, B
; #	MOVWF	r1x05, B
; #	MOVF	r1x05, W
	ADDLW	0x80
	MOVWF	PRODL
	MOVF	r1x01, W
	ADDLW	0x80
	SUBWF	PRODL
	BNZ	_00181_DS_
	MOVF	r1x00, W, B
	SUBWF	r1x04, W, B
_00181_DS_:
	BC	_00110_DS_
;	.line	74; fsadd.c	return fl1.f;
	MOVFF	(___fsxxx_fl1 + 3), FSR0L
	MOVFF	(___fsxxx_fl1 + 2), PRODH
	MOVFF	(___fsxxx_fl1 + 1), PRODL
	MOVF	___fsxxx_fl1, W, B
	BRA	_00133_DS_
_00110_DS_:
;	.line	75; fsadd.c	if (exp2 > exp1 + 25)
	MOVF	r1x00, W, B
	ADDLW	0x19
	MOVWF	r1x04, B
	MOVLW	0x00
	ADDWFC	r1x01, W, B
; #	MOVWF	r1x05, B
; #	MOVF	r1x05, W
	ADDLW	0x80
	MOVWF	PRODL
	MOVF	r1x03, W, B
	ADDLW	0x80
	SUBWF	PRODL, W
	BNZ	_00182_DS_
	MOVF	r1x02, W, B
	SUBWF	r1x04, W, B
_00182_DS_:
	BC	_00112_DS_
;	.line	76; fsadd.c	return fl2.f;
	MOVFF	(___fsxxx_fl2 + 3), FSR0L
	MOVFF	(___fsxxx_fl2 + 2), PRODH
	MOVFF	(___fsxxx_fl2 + 1), PRODL
	MOVF	___fsxxx_fl2, W, B
	BRA	_00133_DS_
_00112_DS_:
;	.line	78; fsadd.c	mant1 = MANT (fl1.l);
	MOVF	___fsxxx_fl1, W, B
	MOVWF	r1x04, B
	MOVF	(___fsxxx_fl1 + 1), W, B
	MOVWF	r1x05, B
	MOVF	(___fsxxx_fl1 + 2), W, B
	MOVWF	r1x06, B
	CLRF	r1x07, B
	BSF	r1x06, 7, B
;	.line	79; fsadd.c	mant2 = MANT (fl2.l);
	MOVF	___fsxxx_fl2, W, B
	MOVWF	r1x0c, B
	MOVF	(___fsxxx_fl2 + 1), W, B
	MOVWF	r1x0d, B
	MOVF	(___fsxxx_fl2 + 2), W, B
	MOVWF	r1x0e, B
	CLRF	r1x0f, B
	BSF	r1x0e, 7, B
;	.line	81; fsadd.c	if (SIGN (fl1.l))
	MOVF	(___fsxxx_fl1 + 3), W, B
	ANDLW	0x80
	BZ	_00114_DS_
;	.line	82; fsadd.c	mant1 = -mant1;
	COMF	r1x07, F, B
	COMF	r1x06, F, B
	COMF	r1x05, F, B
	NEGF	r1x04, B
	BNZ	_00114_DS_
	INCF	r1x05, F, B
	BNZ	_00114_DS_
	INCF	r1x06, F, B
	BTFSC	STATUS, 2
	INCF	r1x07, F, B
_00114_DS_:
;	.line	83; fsadd.c	if (SIGN (fl2.l))
	MOVF	(___fsxxx_fl2 + 3), W, B
	ANDLW	0x80
	BZ	_00116_DS_
;	.line	84; fsadd.c	mant2 = -mant2;
	COMF	r1x0f, F, B
	COMF	r1x0e, F, B
	COMF	r1x0d, F, B
	NEGF	r1x0c, B
	BNZ	_00116_DS_
	INCF	r1x0d, F, B
	BNZ	_00116_DS_
	INCF	r1x0e, F, B
	BTFSC	STATUS, 2
	INCF	r1x0f, F, B
_00116_DS_:
;	.line	86; fsadd.c	if (exp1 > exp2)
	MOVF	r1x03, W, B
	ADDLW	0x80
	MOVWF	PRODL
	MOVF	r1x01, W, B
	ADDLW	0x80
	SUBWF	PRODL, W
	BNZ	_00189_DS_
	MOVF	r1x00, W, B
	SUBWF	r1x02, W, B
_00189_DS_:
	BC	_00118_DS_
;	.line	88; fsadd.c	mant2 >>= exp1 - exp2;
	MOVF	r1x02, W, B
	SUBWF	r1x00, W, B
	BZ	_00119_DS_
	BN	_00193_DS_
	NEGF	WREG
	BCF	STATUS, 0
_00191_DS_:
	BTFSC	r1x0f, 7, B
	BSF	STATUS, 0
	RRCF	r1x0f, F, B
	RRCF	r1x0e, F, B
	RRCF	r1x0d, F, B
	RRCF	r1x0c, F, B
	ADDLW	0x01
	BNC	_00191_DS_
	BRA	_00119_DS_
_00193_DS_:
	BCF	STATUS, 0
_00192_DS_:
	RLCF	r1x0c, F, B
	RLCF	r1x0d, F, B
	RLCF	r1x0e, F, B
	RLCF	r1x0f, F, B
	ADDLW	0x01
	BNC	_00192_DS_
_00190_DS_:
	BRA	_00119_DS_
_00118_DS_:
;	.line	92; fsadd.c	mant1 >>= exp2 - exp1;
	MOVF	r1x00, W, B
	SUBWF	r1x02, W, B
	BZ	_00194_DS_
	BN	_00197_DS_
	NEGF	WREG
	BCF	STATUS, 0
_00195_DS_:
	BTFSC	r1x07, 7, B
	BSF	STATUS, 0
	RRCF	r1x07, F, B
	RRCF	r1x06, F, B
	RRCF	r1x05, F, B
	RRCF	r1x04, F, B
	ADDLW	0x01
	BNC	_00195_DS_
	BRA	_00194_DS_
_00197_DS_:
	BCF	STATUS, 0
_00196_DS_:
	RLCF	r1x04, F, B
	RLCF	r1x05, F, B
	RLCF	r1x06, F, B
	RLCF	r1x07, F, B
	ADDLW	0x01
	BNC	_00196_DS_
_00194_DS_:
;	.line	93; fsadd.c	exp1 = exp2;
	MOVFF	r1x02, r1x00
	MOVFF	r1x03, r1x01
_00119_DS_:
;	.line	95; fsadd.c	mant1 += mant2;
	MOVF	r1x0c, W, B
	ADDWF	r1x04, F, B
	MOVF	r1x0d, W, B
	ADDWFC	r1x05, F, B
	MOVF	r1x0e, W, B
	ADDWFC	r1x06, F, B
	MOVF	r1x0f, W, B
	ADDWFC	r1x07, F, B
;	.line	97; fsadd.c	if (mant1 < 0)
	BTFSS	r1x07, 7, B
	BRA	_00123_DS_
;	.line	99; fsadd.c	mant1 = -mant1;
	COMF	r1x07, F, B
	COMF	r1x06, F, B
	COMF	r1x05, F, B
	NEGF	r1x04, B
	BNZ	_00198_DS_
	INCF	r1x05, F, B
	BNZ	_00198_DS_
	INCF	r1x06, F, B
	BTFSC	STATUS, 2
	INCF	r1x07, F, B
_00198_DS_:
;	.line	100; fsadd.c	sign = SIGNBIT;
	MOVLW	0x80
	MOVWF	r1x0b, B
	BRA	_00125_DS_
_00123_DS_:
;	.line	102; fsadd.c	else if (!mant1)
	MOVF	r1x04, W, B
	IORWF	r1x05, W, B
	IORWF	r1x06, W, B
	IORWF	r1x07, W, B
	BNZ	_00125_DS_
;	.line	103; fsadd.c	return (0);
	CLRF	FSR0L
	CLRF	PRODH
	CLRF	PRODL
	CLRF	WREG
	BRA	_00133_DS_
_00125_DS_:
;	.line	106; fsadd.c	while (mant1 < HIDDEN)
	MOVLW	0x00
	SUBWF	r1x07, W, B
	BNZ	_00199_DS_
	MOVLW	0x80
	SUBWF	r1x06, W, B
	BNZ	_00199_DS_
	MOVLW	0x00
	SUBWF	r1x05, W, B
	BNZ	_00199_DS_
	MOVLW	0x00
	SUBWF	r1x04, W, B
_00199_DS_:
	BC	_00130_DS_
;	.line	108; fsadd.c	mant1 <<= 1;
	MOVF	r1x04, W, B
	ADDWF	r1x04, F, B
	RLCF	r1x05, F, B
	RLCF	r1x06, F, B
	RLCF	r1x07, F, B
;	.line	109; fsadd.c	--exp1;
	MOVLW	0xff
	ADDWF	r1x00, F, B
	ADDWFC	r1x01, F, B
	BRA	_00125_DS_
_00130_DS_:
;	.line	113; fsadd.c	while (mant1 & 0xff000000)
	MOVF	r1x07, W, B
	BZ	_00132_DS_
;	.line	115; fsadd.c	if (mant1&1)
	BTFSS	r1x04, 0, B
	BRA	_00129_DS_
;	.line	116; fsadd.c	mant1 += 2;
	MOVLW	0x02
	ADDWF	r1x04, F, B
	MOVLW	0x00
	ADDWFC	r1x05, F, B
	ADDWFC	r1x06, F, B
	ADDWFC	r1x07, F, B
_00129_DS_:
;	.line	117; fsadd.c	mant1 >>= 1 ;
	RLCF	r1x07, W, B
	RRCF	r1x07, F, B
	RRCF	r1x06, F, B
	RRCF	r1x05, F, B
	RRCF	r1x04, F, B
;	.line	118; fsadd.c	exp1++;
	INFSNZ	r1x00, F, B
	INCF	r1x01, F, B
	BRA	_00130_DS_
_00132_DS_:
;	.line	122; fsadd.c	mant1 &= ~HIDDEN;
	MOVLW	0x7f
	ANDWF	r1x06, F, B
;	.line	125; fsadd.c	fl1.l = PACK (sign, (unsigned long) exp1, mant1);
	CLRF	WREG
	RRCF	r1x01, W, B
	RRCF	r1x00, W, B
	MOVWF	r1x0f, B
	CLRF	r1x0e, B
	RRCF	r1x0e, F, B
	MOVF	r1x0b, W, B
	IORWF	r1x0f, F, B
	MOVF	r1x06, W, B
	IORWF	r1x0e, F, B
	MOVF	r1x07, W, B
	IORWF	r1x0f, F, B
;	.line	127; fsadd.c	return fl1.f;
	MOVF	r1x0f, W, B
	MOVWF	FSR0L
	MOVF	r1x0e, W, B
	MOVWF	PRODH
	MOVF	r1x05, W, B
	MOVWF	PRODL
	MOVF	r1x04, W, B
_00133_DS_:
	MOVFF	PREINC1, FSR2L
	RETURN	



; Statistics:
; code size:	  942 (0x03ae) bytes ( 0.72%)
;           	  471 (0x01d7) words
; udata size:	    8 (0x0008) bytes ( 0.62%)
; access size:	   17 (0x0011) bytes


	end
