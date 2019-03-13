/* $Id: eeprom.c,v 1.3 2017/06/05 11:00:19 bouyer Exp $ */
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

#include <pic18fregs.h>

char
eeprom_read(unsigned int addr)
{
	EEADRH = (addr & 0xff00) >> 8;
	EEADR = (addr & 0xff);

	EECON1 = 0x00; /* point to eeprom */
	EECON1bits.RD = 1;
	__asm__("nop"); /* wait one cycle */
	return EEDATA;

}
void
eeprom_write(unsigned int addr, char data)
{
	EEADRH = (addr & 0xff00) >> 8;
	EEADR = (addr & 0xff);

	EECON1 = 0x04; /* point to eeprom, enable write */
	EEDATA = data;

	INTCONbits.GIE_GIEH=0; /* disable interrupts */
	INTCONbits.PEIE_GIEL=0;

	EECON2 = 0x55;
	EECON2 = 0xaa;
	EECON1bits.WR = 1;
	INTCONbits.GIE_GIEH=1; /* enable interrupts */
	INTCONbits.PEIE_GIEL=1;
	while (EECON1bits.WR)
		; /* wait */
}

