;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.4.0 #8981 (Mar 22 2015) (NetBSD)
; This file was generated Wed Mar 25 19:23:00 2015
;--------------------------------------------------------
; PIC16 port for the Microchip 16-bit core micros
;--------------------------------------------------------
;
; Hand-optimised version of fssub
;
	list	p=18f452
	radix	dec


;--------------------------------------------------------
; public variables in this module
;--------------------------------------------------------
	global	___fssub

;--------------------------------------------------------
; extern variables in this module
;--------------------------------------------------------
	extern	___fsadd

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
r1x07   res     1
r1x0b   res     1
r1x0c   res     1
r1x0d   res     1
r1x0e   res     1
r1x0f   res     1
___fsxxx_fl1    res     4
___fsxxx_fl2    res     4

;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
; I code from now on!
; ; Starting pCode block
S_fssub____fssub	code
___fssub:
;	.line	56; fssub.c	__fssub (float a1, float a2) _FS_REENTRANT
	MOVFF	FSR2L, POSTDEC1
	MOVFF	FSR1L, FSR2L
	BANKSEL r1x00
;	.line	60; fssub.c	fl1.f = a1;
	MOVLW	0x02
	MOVFF	PLUSW2, ___fsxxx_fl1
	MOVLW	0x03
	MOVFF	PLUSW2, (___fsxxx_fl1 + 1)
	MOVLW	0x04
	MOVFF	PLUSW2, (___fsxxx_fl1 + 2)
	MOVLW	0x05
	MOVFF	PLUSW2, (___fsxxx_fl1 + 3)
;	.line	61; fssub.c	fl2.f = a2;
	MOVLW	0x06
	MOVFF	PLUSW2, ___fsxxx_fl2
	MOVLW	0x07
	MOVFF	PLUSW2, (___fsxxx_fl2 + 1)
	MOVLW	0x08
	MOVFF	PLUSW2, (___fsxxx_fl2 + 2)
	MOVLW	0x09
	MOVFF	PLUSW2, (___fsxxx_fl2 + 3)
;	.line	64; fssub.c	if (!fl2.l)
	MOVF	___fsxxx_fl2, W, B
	IORWF	(___fsxxx_fl2 + 1), W, B
	IORWF	(___fsxxx_fl2 + 2), W, B
	IORWF	(___fsxxx_fl2 + 3), W, B
	BNZ	_00106_DS_
;	.line	65; fssub.c	return fl1.f;
	MOVFF	(___fsxxx_fl1 + 3), FSR0L
	MOVFF	(___fsxxx_fl1 + 2), PRODH
	MOVFF	(___fsxxx_fl1 + 1), PRODL
	MOVF	___fsxxx_fl1, W, B
	BRA	_00109_DS_
_00106_DS_:
;	.line	66; fssub.c	if (!fl1.l)
	MOVF	___fsxxx_fl1, W, B
	IORWF	(___fsxxx_fl1 + 1), W, B
	IORWF	(___fsxxx_fl1 + 2), W, B
	IORWF	(___fsxxx_fl1 + 3), W, B
	BNZ	_00108_DS_
;	.line	67; fssub.c	return -fl2.f;
	BTG	(___fsxxx_fl2 + 3), 7, B
	MOVFF	(___fsxxx_fl2 + 3), FSR0L
	MOVFF	(___fsxxx_fl2 + 2), PRODH
	MOVFF	(___fsxxx_fl2 + 1), PRODL
	MOVF	___fsxxx_fl2, W
	BRA	_00109_DS_
_00108_DS_:
;	.line	70; fssub.c	fl2.l ^= SIGNBIT;
	BTG	(___fsxxx_fl2 + 3), 7, B
;	.line	71; fssub.c	return fl1.f + fl2.f; 
	MOVF	(___fsxxx_fl2 + 3), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(___fsxxx_fl2 + 2), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(___fsxxx_fl2 + 1), W, B
	MOVWF	POSTDEC1,   A
	MOVF	___fsxxx_fl2, W, B
	MOVWF	POSTDEC1,   A
	MOVF	(___fsxxx_fl1 + 3), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(___fsxxx_fl1 + 2), W, B
	MOVWF	POSTDEC1,   A
	MOVF	(___fsxxx_fl1 + 1), W, B
	MOVWF	POSTDEC1,   A
	MOVF	___fsxxx_fl1, W, B
	MOVWF	POSTDEC1,   A
	CALL	___fsadd
	MOVWF	r1x00, B
	MOVLW	0x08
	ADDWF	FSR1L, F
	MOVF	r1x00, W, B
_00109_DS_:
	MOVFF	PREINC1, FSR2L
	RETURN	



; Statistics:
; code size:	  340 (0x0154) bytes ( 0.26%)
;           	  170 (0x00aa) words
; udata size:	    8 (0x0008) bytes ( 0.62%)
; access size:	    8 (0x0008) bytes


	end
