/* $Id: autopilot_spi.h,v 1.3 2017/06/05 11:00:18 bouyer Exp $ */
/*
 * Copyright (c) 2017 Manuel Bouyer
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define SPI_INIT { \
	TRISCbits.TRISC3 = 0; /* SCK */ \
	TRISCbits.TRISC5 = 0; /* SD0 */ \
	TRISCbits.TRISC4 = 0; /* as output as nothing is connected here */ \
	TRISAbits.TRISA5 = 0; /* as output as nothing is connected here */ \
	/* SSPCON1 = 0x32; SPI master mode, Fosc/64 */ \
	/* SSPCON1 = 0x31; SPI master mode, Fosc/16 */ \
	SSPCON1 = 0x3a; /* SPI master mode, Fosc/16 */ \
	SSPSTAT = 0x00; /* data out on falling edge */ \
	PIR1bits.SSPIF=0; \
	PIR2bits.BCLIF=0; \
    }

char spi_write(const unsigned char data) __wparam ;
