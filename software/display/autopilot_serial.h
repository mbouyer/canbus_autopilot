/* $Id: autopilot_serial.h,v 1.2 2017/06/05 11:00:18 bouyer Exp $ */
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

char getchar(void);

/* #define UART_BUFSIZE 16 */
/* #define UART_BUFSIZE_MASK 0x0f */
#define UART_BUFSIZE 128
#define UART_BUFSIZE_MASK 0x7f

extern char uart_txbuf[UART_BUFSIZE];
extern unsigned char uart_txbuf_prod;
extern volatile unsigned char uart_txbuf_cons;
void usart_putchar (char c) __wparam;

#define USART_INIT(p) { \
		IPR3bits.TX2IP=p; \
		IPR3bits.RC2IP=p; \
		uart_txbuf_prod = uart_txbuf_cons = 0; \
	}

#define USART_INTR {\
	if (PIE3bits.TX2IE && PIR3bits.TX2IF) { \
		if (uart_txbuf_prod == uart_txbuf_cons) { \
			PIE3bits.TX2IE = 0; /* buffer empty */ \
		} else { \
			/* Place char in TXREG - this starts transmition */ \
			TXREG2 = uart_txbuf[uart_txbuf_cons]; \
			uart_txbuf_cons = (uart_txbuf_cons + 1) & UART_BUFSIZE_MASK;\
		} \
	} \
    }
