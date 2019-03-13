;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.4.0 #8981 (Mar 22 2015) (NetBSD)
; This file was generated Tue Mar 24 20:24:44 2015
;--------------------------------------------------------
; PIC16 port for the Microchip 16-bit core micros
;--------------------------------------------------------
;
; Hand-optimised version of fsmul
;
	list	p=18f452
	radix	dec


;--------------------------------------------------------
; public variables in this module
;--------------------------------------------------------
	global	___fsmul

;--------------------------------------------------------
; extern variables in this module
;--------------------------------------------------------
	extern	__mullong

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
r0x10	res	1
r0x11	res	1
r0x12	res	1
r0x13	res	1
r0x14	res	1
r0x15	res	1
r0x16	res	1
r0x17	res	1

;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
; I code from now on!
; ; Starting pCode block
S_fsmul____fsmul	code
___fsmul:
;	.line	56; fsmul.c	__fsmul (float a1, float a2) _FS_REENTRANT
	MOVFF	FSR2L, POSTDEC1
	MOVFF	FSR1L, FSR2L
	MOVFF	r0x00, POSTDEC1
	MOVFF	r0x01, POSTDEC1
	MOVFF	r0x02, POSTDEC1
	MOVFF	r0x03, POSTDEC1
	MOVFF	r0x04, POSTDEC1
	MOVFF	r0x05, POSTDEC1
	MOVFF	r0x06, POSTDEC1
	MOVFF	r0x10, POSTDEC1
	MOVFF	r0x11, POSTDEC1
	MOVFF	r0x12, POSTDEC1
	MOVFF	r0x13, POSTDEC1
	MOVFF	r0x14, POSTDEC1
	MOVFF	r0x15, POSTDEC1
	MOVFF	r0x16, POSTDEC1
	MOVFF	r0x17, POSTDEC1
;	.line	63; fsmul.c	fl1.f = a1;
	MOVLW	0x02
	MOVFF	PLUSW2, r0x10
	MOVLW	0x03
	MOVFF	PLUSW2, r0x11
	MOVLW	0x04
	MOVFF	PLUSW2, r0x12
	MOVLW	0x05
	MOVFF	PLUSW2, r0x13
;	.line	64; fsmul.c	fl2.f = a2;
	MOVLW	0x06
	MOVFF	PLUSW2, r0x14
	MOVLW	0x07
	MOVFF	PLUSW2, r0x15
	MOVLW	0x08
	MOVFF	PLUSW2, r0x16
	MOVLW	0x09
	MOVFF	PLUSW2, r0x17
;	.line	66; fsmul.c	if (!fl1.l || !fl2.l)
	MOVF	r0x10, W
	IORWF	(r0x10 + 1), W
	IORWF	(r0x10 + 2), W
	IORWF	(r0x10 + 3), W
	BZ	_00105_DS_
	MOVF	r0x14, W
	IORWF	(r0x14 + 1), W
	IORWF	(r0x14 + 2), W
	IORWF	(r0x14 + 3), W
	BNZ	_00106_DS_
_00105_DS_:
;	.line	67; fsmul.c	return 0;
	CLRF	FSR0L,   A
	CLRF	PRODH,   A
	CLRF	PRODL,   A
	CLRF	WREG,   A
	BRA	_00117_DS_
_00106_DS_:
;	.line	70; fsmul.c	sign = SIGN (fl1.l) ^ SIGN (fl2.l);
	MOVF	(r0x10 + 3), W
	ANDLW	0x80
	RLNCF	WREG, W
	MOVWF	r0x00,   A
	MOVF	(r0x14 + 3), W
	ANDLW	0x80
	RLNCF	WREG, W
	XORWF	r0x00, F
;	.line	71; fsmul.c	exp = EXP (fl1.l) - EXCESS;
	RLCF	(r0x10 + 2), W
	RLCF	(r0x10 + 3), W
	MOVWF	r0x01,   A
	CLRF	r0x02,   A
	MOVLW	0x82
	ADDWF	r0x01, F
	MOVLW	0xff
	ADDWFC	r0x02, F
;	.line	72; fsmul.c	exp += EXP (fl2.l);
	CLRF	r0x04,   A
	RLCF	(r0x14 + 2), W
	RLCF	(r0x14 + 3), W
	ADDWF	r0x01, F
	MOVF	r0x04, W
	ADDWFC	r0x02, F
;	.line	74; fsmul.c	fl1.l = MANT (fl1.l);
	BSF	(r0x10 + 2), 7
	CLRF	(r0x10 + 3),   A
;	.line	75; fsmul.c	fl2.l = MANT (fl2.l);
	BSF	(r0x14 + 2), 7
	CLRF	(r0x14 + 3),   A
;	.line	78; fsmul.c	result = (fl1.l >> 8) * (fl2.l >> 8);
	CLRF	POSTDEC1,   A
	MOVF	(r0x14 + 3), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x14 + 2), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x14 + 1), W
	MOVWF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	MOVF	(r0x10 + 3), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x10 + 2), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x10 + 1), W
	MOVWF	POSTDEC1,   A
	CALL	__mullong
	MOVWF	r0x03,   A
	MOVFF	PRODL, r0x04
	MOVFF	PRODH, r0x05
	MOVFF	FSR0L, r0x06
	MOVLW	0x08
	ADDWF	FSR1L, F
;	.line	79; fsmul.c	result += ((fl1.l & (unsigned long) 0xFF) * (fl2.l >> 8)) >> 8;
	CLRF	POSTDEC1,   A
	MOVF	(r0x14 + 3), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x14 + 2), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x14 + 1), W
	MOVWF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	MOVF	r0x10, W
	MOVWF	POSTDEC1,   A
	CALL	__mullong
	MOVLW	0x08
	ADDWF	FSR1L, F
	MOVF	PRODL, W
	ADDWF	r0x03, F
	MOVF	PRODH, W
	ADDWFC	r0x04, F
	MOVF	FSR0L, W
	ADDWFC	r0x05, F
	MOVLW	0x00
	ADDWFC	r0x06, F
;	.line	80; fsmul.c	result += ((fl2.l & (unsigned long) 0xFF) * (fl1.l >> 8)) >> 8;
	CLRF	POSTDEC1,   A
	MOVF	(r0x10 + 3), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x10 + 2), W
	MOVWF	POSTDEC1,   A
	MOVF	(r0x10 + 1), W
	MOVWF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	MOVF	r0x14, W
	MOVWF	POSTDEC1,   A
	CALL	__mullong
	MOVLW	0x08
	ADDWF	FSR1L, F
	MOVF	PRODL, W
	ADDWF	r0x03, F
	MOVF	PRODH, W
	ADDWFC	r0x04, F
	MOVF	FSR0L, W
	ADDWFC	r0x05, F
	MOVLW	0x00
	ADDWFC	r0x06, F
;	.line	83; fsmul.c	result += 0x40;
	MOVLW	0x40
	ADDWF	r0x03, F
	MOVLW	0x00
	ADDWFC	r0x04, F
	ADDWFC	r0x05, F
	ADDWFC	r0x06, F
;	.line	85; fsmul.c	if (result & SIGNBIT)
	BTFSS	r0x06, 7
	BRA	_00109_DS_
;	.line	88; fsmul.c	result += 0x40;
	MOVLW	0x40
	ADDWF	r0x03, F
	MOVLW	0x00
	ADDWFC	r0x04, F
	ADDWFC	r0x05, F
	ADDWFC	r0x06, F
;	.line	89; fsmul.c	result >>= 8;
	MOVF	r0x04, W
	MOVWF	r0x03,   A
	MOVF	r0x05, W
	MOVWF	r0x04,   A
	MOVF	r0x06, W
	MOVWF	r0x05,   A
	CLRF	r0x06,   A
	BRA	_00110_DS_
_00109_DS_:
;	.line	93; fsmul.c	result >>= 7;
	RLCF	r0x03, W
	RLCF	r0x04, W
	MOVWF	r0x03,   A
	CLRF	r0x04,   A
	RLCF	r0x04, F
	MOVF	r0x05, W
	RLNCF	WREG, W
	ANDLW	0xfe
	IORWF	r0x04, F
	RLCF	r0x05, W
	RLCF	r0x06, W
	MOVWF	r0x05,   A
	CLRF	r0x06,   A
	RLCF	r0x06, F
;	.line	94; fsmul.c	--exp;
	MOVLW	0xff
	ADDWF	r0x01, F
	ADDWFC	r0x02, F
_00110_DS_:
;	.line	97; fsmul.c	result &= ~HIDDEN;
	BCF	r0x05, 7
;	.line	100; fsmul.c	if (exp >= 0x100)
	MOVF	r0x02, W
	ADDLW	0x80
	ADDLW	0x7f
	BNZ	_00152_DS_
	MOVLW	0x00
	SUBWF	r0x01, W
_00152_DS_:
	BNC	_00115_DS_
;	.line	101; fsmul.c	fl1.l = (sign ? SIGNBIT : 0) | 0x7F800000;
	MOVF	r0x00, W
	BZ	_00119_DS_
;	.line	106; fsmul.c	return fl1.f;
	MOVLW	0xFF
	MOVWF	FSR0L,   A
	MOVLW	0x80
	MOVWF	PRODH,   A
	CLRF	PRODL,   A
	CLRF	WREG,   A
	BRA	_00117_DS_
_00119_DS_:
;	.line	106; fsmul.c	return fl1.f;
	MOVLW	0x7F
	MOVWF	FSR0L,   A
	MOVLW	0x80
	MOVWF	PRODH,   A
	CLRF	PRODL,   A
	CLRF	WREG,   A
	BRA	_00117_DS_
_00115_DS_:
;	.line	102; fsmul.c	else if (exp < 0)
	BSF	STATUS, 0
	BTFSS	r0x02, 7
	BCF	STATUS, 0
	BNC	_00112_DS_
;	.line	103; fsmul.c	fl1.l = 0;
;	.line	106; fsmul.c	return fl1.f;
	CLRF	FSR0L,   A
	CLRF	PRODH,   A
	CLRF	PRODL,   A
	CLRF	WREG,   A
	BRA	_00117_DS_
_00112_DS_:
;	.line	105; fsmul.c	fl1.l = PACK (sign ? SIGNBIT : 0 , exp, result);
	MOVF	r0x00, W
	BZ	_00121_DS_
;	CLRF	r0x00,   A
;	CLRF	r0x07,   A
;	CLRF	r0x08,   A
	MOVLW	0x80
	MOVWF	r0x10,   A
	BRA	_00122_DS_
_00121_DS_:
;	CLRF	r0x00,   A
;	CLRF	r0x07,   A
;	CLRF	r0x08,   A
	CLRF	r0x10,   A
_00122_DS_:
	CLRF	WREG,   A
	RRCF	r0x02, W
	RRCF	r0x01, W
	MOVWF	r0x12,   A
	CLRF	r0x11,   A
	RRCF	r0x11, F
;	MOVLW	0x00
;	IORWF	r0x00, F
;	IORWF	r0x07, F
;	MOVF	r0x11, W
;	IORWF	r0x08, F
	MOVF	r0x12, W
	IORWF	r0x10, F
;	MOVF	r0x00, W
;	IORWF	r0x03, F
;	MOVF	r0x07, W
;	IORWF	r0x04, F
;	MOVF	r0x08, W
	MOVF	r0x11, W
	IORWF	r0x05, F
	MOVF	r0x10, W
	IORWF	r0x06, F
;	.line	106; fsmul.c	return fl1.f;
	MOVF	r0x06, W
	MOVWF	FSR0L,   A
	MOVF	r0x05, W
	MOVWF	PRODH,   A
	MOVF	r0x04, W
	MOVWF	PRODL,   A
	MOVF	r0x03, W
_00117_DS_:
	MOVFF	PREINC1, r0x17
	MOVFF	PREINC1, r0x16
	MOVFF	PREINC1, r0x15
	MOVFF	PREINC1, r0x14
	MOVFF	PREINC1, r0x13
	MOVFF	PREINC1, r0x12
	MOVFF	PREINC1, r0x11
	MOVFF	PREINC1, r0x10
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
; code size:	  916 (0x0394) bytes ( 0.70%)
;           	  458 (0x01ca) words
; udata size:	    8 (0x0008) bytes ( 0.62%)
; access size:	   16 (0x0010) bytes


	end
