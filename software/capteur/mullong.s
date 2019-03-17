;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.4.0 #8981 (Mar 22 2015) (NetBSD)
; This file was generated Wed Mar 25 19:48:46 2015
;--------------------------------------------------------
; PIC16 port for the Microchip 16-bit core micros
;--------------------------------------------------------
;
; hand-optimised version of mullong
;
	list	p=18f452
	radix	dec


;--------------------------------------------------------
; public variables in this module
;--------------------------------------------------------
	global	__mullong

;--------------------------------------------------------
;	Equates to used internal registers
;--------------------------------------------------------
WREG	equ	0xfe8
FSR0L	equ	0xfe9
FSR1L	equ	0xfe1
FSR2L	equ	0xfd9
POSTDEC1	equ	0xfe5
PREINC1	equ	0xfe4
PLUSW2	equ	0xfdb
PRODL	equ	0xff3
PRODH	equ	0xff4


.privatei	udata
__mullong_x_1_2	res	4

__mullong_y_1_2	res	4

__mullong_t_1_2	res	4

__mullong_t1_1_2	res	4

__mullong_t2_1_2	res	4

;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
; I code from now on!
; ; Starting pCode block
S_mullong___mullong	code
__mullong:
;	.line	61; mullong.c	long _mullong (long a, long b) _IL_REENTRANT
	MOVFF	FSR2L, POSTDEC1
	MOVFF	FSR1L, FSR2L
	BANKSEL __mullong_x_1_2
;	.line	97; mullong.c	x.l = a;
	MOVLW	0x02
	MOVFF	PLUSW2, __mullong_x_1_2
	MOVLW	0x03
	MOVFF	PLUSW2, (__mullong_x_1_2 + 1)
	MOVLW	0x04
	MOVFF	PLUSW2, (__mullong_x_1_2 + 2)
	MOVLW	0x05
	MOVFF	PLUSW2, (__mullong_x_1_2 + 3)
;	.line	98; mullong.c	y.l = b;
	MOVLW	0x06
	MOVFF	PLUSW2, __mullong_y_1_2
	MOVLW	0x07
	MOVFF	PLUSW2, (__mullong_y_1_2 + 1)
	MOVLW	0x08
	MOVFF	PLUSW2, (__mullong_y_1_2 + 2)
	MOVLW	0x09
	MOVFF	PLUSW2, (__mullong_y_1_2 + 3)
; ;multiply variable :(__mullong_y_1_2 + 2) by variable __mullong_x_1_2 and store in r0x00
;	.line	100; mullong.c	t.i.hi = x.b.b0 * y.b.b2;
	MOVF	__mullong_x_1_2, W, B
	MULWF	(__mullong_y_1_2 + 2), B
	MOVFF	PRODH, (__mullong_t_1_2 + 3)
	MOVFF	PRODL, (__mullong_t_1_2 + 2)
; ;multiply variable :__mullong_y_1_2 by variable __mullong_x_1_2 and store in r0x00
;	.line	101; mullong.c	t.i.lo = x.b.b0 * y.b.b0;
	MOVF	__mullong_x_1_2, W, B
	MULWF	__mullong_y_1_2, B
	MOVFF	PRODH, (__mullong_t_1_2 + 1)
	MOVFF	PRODL, __mullong_t_1_2
; ;multiply variable :__mullong_y_1_2 by variable (__mullong_x_1_2 + 3) and store in r0x00
;	.line	103; mullong.c	t.b.b3 += x.b.b3 * y.b.b0;
	MOVF	(__mullong_x_1_2 + 3), W, B
	MULWF	__mullong_y_1_2, B
	MOVF	PRODL, W
	ADDWF	(__mullong_t_1_2 + 3), F
; ;multiply variable :(__mullong_y_1_2 + 1) by variable (__mullong_x_1_2 + 2) and store in r0x00
;	.line	104; mullong.c	t.b.b3 += x.b.b2 * y.b.b1;
	MOVF	(__mullong_x_1_2 + 2), W, B
	MULWF	(__mullong_y_1_2 + 1), B
	MOVF	PRODL, W
	ADDWF	(__mullong_t_1_2 + 3), F
; ;multiply variable :__mullong_y_1_2 by variable (__mullong_x_1_2 + 2) and store in r0x00
;	.line	106; mullong.c	t.i.hi += x.b.b2 * y.b.b0;
	MOVF	(__mullong_x_1_2 + 2), W, B
	MULWF	__mullong_y_1_2, B
	MOVF	PRODL, W
	ADDWF	(__mullong_t_1_2 + 2), F
	MOVF	PRODH, W
	ADDWFC	(__mullong_t_1_2 + 3), F
; ;multiply variable :(__mullong_y_1_2 + 1) by variable (__mullong_x_1_2 + 1) and store in r0x00
;	.line	107; mullong.c	t.i.hi += x.b.b1 * y.b.b1;
	MOVF	(__mullong_x_1_2 + 1), W, B
	MULWF	(__mullong_y_1_2 + 1), B
	MOVF	PRODL, W
	ADDWF	(__mullong_t_1_2 + 2), F
	MOVF	PRODH, W
	ADDWFC	(__mullong_t_1_2 + 3), F
; ;multiply variable :(__mullong_y_1_2 + 2) by variable (__mullong_x_1_2 + 1) and store in r0x00
;	.line	109; mullong.c	t1.bi.b3 = x.b.b1 * y.b.b2;
	MOVF	(__mullong_x_1_2 + 1), W, B
	MULWF	(__mullong_y_1_2 + 2), B
	MOVFF	PRODL, (__mullong_t1_1_2 + 3)
; ;multiply variable :__mullong_y_1_2 by variable (__mullong_x_1_2 + 1) and store in r0x00
;	.line	110; mullong.c	t1.bi.i12 = x.b.b1 * y.b.b0;
	MOVF	(__mullong_x_1_2 + 1), W, B
	MULWF	__mullong_y_1_2, B
	MOVFF	PRODH, (__mullong_t1_1_2 + 2)
	MOVFF	PRODL, (__mullong_t1_1_2 + 1)
; ;multiply variable :(__mullong_y_1_2 + 3) by variable __mullong_x_1_2 and store in r0x00
;	.line	112; mullong.c	t2.bi.b3 = x.b.b0 * y.b.b3;
	MOVF	__mullong_x_1_2, W, B
	MULWF	(__mullong_y_1_2 + 3), B
	MOVFF	PRODL, (__mullong_t2_1_2 + 3)
; ;multiply variable :(__mullong_y_1_2 + 1) by variable __mullong_x_1_2 and store in r0x00
;	.line	113; mullong.c	t2.bi.i12 = x.b.b0 * y.b.b1;
	MOVF	__mullong_x_1_2, W, B
	MULWF	(__mullong_y_1_2 + 1), B
	MOVFF	PRODH, (__mullong_t2_1_2 + 2)
	MOVFF	PRODL, (__mullong_t2_1_2 + 1)
;	.line	115; mullong.c	t1.bi.b0 = 0;
	CLRF	__mullong_t1_1_2, B
;	.line	117; mullong.c	t.l += t1.l;
	MOVF	__mullong_t1_1_2, W, B
	ADDWF	__mullong_t_1_2, W, B
	MOVWF	__mullong_t_1_2,   B
	MOVF	(__mullong_t1_1_2 + 1), W, B
	ADDWFC	(__mullong_t_1_2 + 1), F, B
	MOVF	(__mullong_t1_1_2 + 2), W, B
	ADDWFC	(__mullong_t_1_2 + 2), F, B
	MOVF	(__mullong_t1_1_2 + 3), W, B
	ADDWFC	(__mullong_t_1_2 + 3), F, B
;	.line	116; mullong.c	t2.bi.b0 = 0;
	CLRF	__mullong_t2_1_2, B
;	.line	119; mullong.c	return (t.l + t2.l);
	MOVF	__mullong_t2_1_2, W, B
	ADDWF	__mullong_t_1_2, F, B
	MOVF	(__mullong_t2_1_2 + 1), W, B
	ADDWFC	(__mullong_t_1_2 + 1), W, B
	MOVWF	PRODL,   A
	MOVF	(__mullong_t2_1_2 + 2), W, B
	ADDWFC	(__mullong_t_1_2 + 2), W, B
	MOVWF	PRODH,   A
	MOVF	(__mullong_t2_1_2 + 3), W, B
	ADDWFC	(__mullong_t_1_2 + 3), W, B
	MOVWF	FSR0L,   A
	MOVF	__mullong_t_1_2, W, B
	MOVFF	PREINC1, FSR2L
	RETURN	



; Statistics:
; code size:	  532 (0x0214) bytes ( 0.41%)
;           	  266 (0x010a) words
; udata size:	   20 (0x0014) bytes ( 1.56%)
; access size:	    8 (0x0008) bytes


	end
