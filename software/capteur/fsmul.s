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


.privatefp      udata

r1x00   res     1
r1x01   res     1
r1x02   res     1
r1x03   res     1
r1x04   res     1
r1x05   res     1
r1x06   res     1
r1x10   res     1
r1x11   res     1
r1x12   res     1
r1x13   res     1
r1x14   res     1
r1x15   res     1
r1x16   res     1
r1x17   res     1




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
	BANKSEL r1x00
;	.line	63; fsmul.c	fl1.f = a1;
	MOVLW	0x02
	MOVFF	PLUSW2, r1x10
	MOVLW	0x03
	MOVFF	PLUSW2, r1x11
	MOVLW	0x04
	MOVFF	PLUSW2, r1x12
	MOVLW	0x05
	MOVFF	PLUSW2, r1x13
;	.line	64; fsmul.c	fl2.f = a2;
	MOVLW	0x06
	MOVFF	PLUSW2, r1x14
	MOVLW	0x07
	MOVFF	PLUSW2, r1x15
	MOVLW	0x08
	MOVFF	PLUSW2, r1x16
	MOVLW	0x09
	MOVFF	PLUSW2, r1x17
;	.line	66; fsmul.c	if (!fl1.l || !fl2.l)
	MOVF	r1x10, W, B
	IORWF	(r1x10 + 1), W, B
	IORWF	(r1x10 + 2), W, B
	IORWF	(r1x10 + 3), W, B
	BZ	_00105_DS_
	MOVF	r1x14, W, B
	IORWF	(r1x14 + 1), W, B
	IORWF	(r1x14 + 2), W, B
	IORWF	(r1x14 + 3), W, B
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
	MOVF	(r1x10 + 3), W, B
	ANDLW	0x80
	RLNCF	WREG, W
	MOVWF	r1x00,   B
	MOVF	(r1x14 + 3), W, B
	ANDLW	0x80
	RLNCF	WREG, W
	XORWF	r1x00, F, B
;	.line	71; fsmul.c	exp = EXP (fl1.l) - EXCESS;
	RLCF	(r1x10 + 2), W, B
	RLCF	(r1x10 + 3), W, B
	MOVWF	r1x01,   B
	CLRF	r1x02,   B
	MOVLW	0x82
	ADDWF	r1x01, F, B
	MOVLW	0xff
	ADDWFC	r1x02, F, B
;	.line	72; fsmul.c	exp += EXP (fl2.l);
	CLRF	r1x04,   B
	RLCF	(r1x14 + 2), W, B
	RLCF	(r1x14 + 3), W, B
	ADDWF	r1x01, F, B
	MOVF	r1x04, W, B
	ADDWFC	r1x02, F, B
;	.line	74; fsmul.c	fl1.l = MANT (fl1.l);
	BSF	(r1x10 + 2), 7, B
	CLRF	(r1x10 + 3),   B
;	.line	75; fsmul.c	fl2.l = MANT (fl2.l);
	BSF	(r1x14 + 2), 7, B
	CLRF	(r1x14 + 3),   B
;	.line	78; fsmul.c	result = (fl1.l >> 8) * (fl2.l >> 8);
	CLRF	POSTDEC1,   A
	MOVF	(r1x14 + 3), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x14 + 2), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x14 + 1), W, B
	MOVWF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	MOVF	(r1x10 + 3), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x10 + 2), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x10 + 1), W, B
	MOVWF	POSTDEC1,   A
	CALL	__mullong
	BANKSEL r1x00
	MOVWF	r1x03,   B
	MOVFF	PRODL, r1x04
	MOVFF	PRODH, r1x05
	MOVFF	FSR0L, r1x06
	MOVLW	0x08
	ADDWF	FSR1L, F
;	.line	79; fsmul.c	result += ((fl1.l & (unsigned long) 0xFF) * (fl2.l >> 8)) >> 8;
	CLRF	POSTDEC1,   A
	MOVF	(r1x14 + 3), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x14 + 2), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x14 + 1), W, B
	MOVWF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	MOVF	r1x10, W, B
	MOVWF	POSTDEC1,   A
	CALL	__mullong
	BANKSEL r1x00
	MOVLW	0x08
	ADDWF	FSR1L, F
	MOVF	PRODL, W
	ADDWF	r1x03, F, B
	MOVF	PRODH, W
	ADDWFC	r1x04, F, B
	MOVF	FSR0L, W
	ADDWFC	r1x05, F, B
	MOVLW	0x00
	ADDWFC	r1x06, F, B
;	.line	80; fsmul.c	result += ((fl2.l & (unsigned long) 0xFF) * (fl1.l >> 8)) >> 8;
	CLRF	POSTDEC1,   A
	MOVF	(r1x10 + 3), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x10 + 2), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(r1x10 + 1), W, B
	MOVWF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	CLRF	POSTDEC1,   A
	MOVF	r1x14, W, B
	MOVWF	POSTDEC1,   A
	CALL	__mullong
	BANKSEL r1x00
	MOVLW	0x08
	ADDWF	FSR1L, F
	MOVF	PRODL, W
	ADDWF	r1x03, F, B
	MOVF	PRODH, W
	ADDWFC	r1x04, F, B
	MOVF	FSR0L, W
	ADDWFC	r1x05, F, B
	MOVLW	0x00
	ADDWFC	r1x06, F, B
;	.line	83; fsmul.c	result += 0x40;
	MOVLW	0x40
	ADDWF	r1x03, F, B
	MOVLW	0x00
	ADDWFC	r1x04, F, B
	ADDWFC	r1x05, F, B
	ADDWFC	r1x06, F, B
;	.line	85; fsmul.c	if (result & SIGNBIT)
	BTFSS	r1x06, 7, B
	BRA	_00109_DS_
;	.line	88; fsmul.c	result += 0x40;
	MOVLW	0x40
	ADDWF	r1x03, F, B
	MOVLW	0x00
	ADDWFC	r1x04, F, B
	ADDWFC	r1x05, F, B
	ADDWFC	r1x06, F, B
;	.line	89; fsmul.c	result >>= 8;
	MOVF	r1x04, W, B
	MOVWF	r1x03,   B
	MOVF	r1x05, W, B
	MOVWF	r1x04,   B
	MOVF	r1x06, W, B
	MOVWF	r1x05,   B
	CLRF	r1x06,   B
	BRA	_00110_DS_
_00109_DS_:
;	.line	93; fsmul.c	result >>= 7;
	RLCF	r1x03, W, B
	RLCF	r1x04, W, B
	MOVWF	r1x03,   B
	CLRF	r1x04,   B
	RLCF	r1x04, F, B
	MOVF	r1x05, W, B
	RLNCF	WREG, W
	ANDLW	0xfe
	IORWF	r1x04, F, B
	RLCF	r1x05, W, B
	RLCF	r1x06, W, B
	MOVWF	r1x05,   B
	CLRF	r1x06,   B
	RLCF	r1x06, F, B
;	.line	94; fsmul.c	--exp;
	MOVLW	0xff
	ADDWF	r1x01, F, B
	ADDWFC	r1x02, F, B
_00110_DS_:
;	.line	97; fsmul.c	result &= ~HIDDEN;
	BCF	r1x05, 7, B
;	.line	100; fsmul.c	if (exp >= 0x100)
	MOVF	r1x02, W, B
	ADDLW	0x80
	ADDLW	0x7f
	BNZ	_00152_DS_
	MOVLW	0x00
	SUBWF	r1x01, W, B
_00152_DS_:
	BNC	_00115_DS_
;	.line	101; fsmul.c	fl1.l = (sign ? SIGNBIT : 0) | 0x7F800000;
	MOVF	r1x00, W, B
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
	BTFSS	r1x02, 7, B
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
	MOVF	r1x00, W, B
	BZ	_00121_DS_
;	CLRF	r1x00,   B
;	CLRF	r1x07,   B
;	CLRF	r1x08,   B
	MOVLW	0x80
	MOVWF	r1x10,   B
	BRA	_00122_DS_
_00121_DS_:
;	CLRF	r1x00,   B
;	CLRF	r1x07,   B
;	CLRF	r1x08,   B
	CLRF	r1x10,   B
_00122_DS_:
	CLRF	WREG,   A
	RRCF	r1x02, W, B
	RRCF	r1x01, W, B
	MOVWF	r1x12,   B
	CLRF	r1x11,   B
	RRCF	r1x11, F, B
;	MOVLW	0x00
;	IORWF	r1x00, F, B
;	IORWF	r1x07, F, B
;	MOVF	r1x11, W, B
;	IORWF	r1x08, F, B
	MOVF	r1x12, W, B
	IORWF	r1x10, F, B
;	MOVF	r1x00, W, B
;	IORWF	r1x03, F, B
;	MOVF	r1x07, W, B
;	IORWF	r1x04, F, B
;	MOVF	r1x08, W, B
	MOVF	r1x11, W, B
	IORWF	r1x05, F, B
	MOVF	r1x10, W, B
	IORWF	r1x06, F, B
;	.line	106; fsmul.c	return fl1.f;
	MOVF	r1x06, W, B
	MOVWF	FSR0L,   A
	MOVF	r1x05, W, B
	MOVWF	PRODH,   A
	MOVF	r1x04, W, B
	MOVWF	PRODL,   A
	MOVF	r1x03, W, B
_00117_DS_:
	MOVFF	PREINC1, FSR2L
	RETURN	



; Statistics:
; code size:	  916 (0x0394) bytes ( 0.70%)
;           	  458 (0x01ca) words
; udata size:	    8 (0x0008) bytes ( 0.62%)
; access size:	   16 (0x0010) bytes


	end
