/* $Id: serial.c,v 1.2 2017/06/05 11:00:19 bouyer Exp $ */
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
#include <stdio.h>
#include <autopilot_serial.h>

char uart_txbuf[UART_BUFSIZE];
unsigned char uart_txbuf_prod;
volatile unsigned char uart_txbuf_cons;

PUTCHAR(c) /* Macro */
{
	unsigned char new_uart_txbuf_prod;
#if 1
	if  (!PORTBbits.RB7)
		return;
#endif
	new_uart_txbuf_prod = (uart_txbuf_prod + 1) & UART_BUFSIZE_MASK;
#if 1

again:
        while (new_uart_txbuf_prod == uart_txbuf_cons) {
		PIE3bits.TX2IE = 1; /* ensure we'll make progress */
	}
	uart_txbuf[uart_txbuf_prod] = c;
	uart_txbuf_prod = new_uart_txbuf_prod;
	PIE3bits.TX2IE = 1;
	if (c == '\n') {
		c = '\r';
		new_uart_txbuf_prod = (uart_txbuf_prod + 1) & UART_BUFSIZE_MASK;
		goto again;
	}
#else
again:
	while (!PIR3bits.TX2IF)
		; /* wait */
	TXREG2 = c;
	if (c == '\n') {
		c = '\r';
		goto again;
	}
#endif
}

char
getchar(void)
{
	char c;
	while (!PIR3bits.RC2IF); /* wait for a char */
	c = RCREG2;
	if (RCSTA2bits.OERR) {
		RCSTA2bits.CREN = 0;
		RCSTA2bits.CREN = 1;
	}
	return c;
}
