/* $Id: main.c,v 1.32 2019/03/08 19:26:34 bouyer Exp $ */
/*
 * Copyright (c) 2019 Manuel Bouyer
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
#include <nmea2000.h>
#include <nmea2000_pgn.h>
#include <raddeg.h>

extern char stack;
extern char stack_end;

#pragma stack 0x100 256

void _reset (void) __naked __interrupt 0;
void _startup (void) __naked;

unsigned long nmea2000_user_id;

static unsigned char sid;

static struct nmea2000_msg msg;
static unsigned char nmea2000_data[NMEA2000_DATA_LENGTH];

unsigned int timer0_read(void);
#pragma callee_saves timer0_read

static char counter_1hz;	
static volatile char counter_10hz;	
static volatile unsigned char softintrs;
#define INT_10HZ	 (unsigned char)0x01
#define INT_RADIO0	 (unsigned char)0x02
#define INT_RADIO1	 (unsigned char)0x04

#define TIMER0_5MS 48

#define CLRWDT __asm__("clrwdt")

#define RADIO_ENABLE RCSTA1bits.SPEN
#define RADIO_FL 8
static unsigned char rrx_cnt;
static unsigned char rrx_abuf;
static unsigned char rrx_buf0[RADIO_FL];
static unsigned char rrx_buf1[RADIO_FL];

#define RADIO_SN0 0xe1
#define RADIO_SN1 0x1d
#define RADIO_SN2 0x00

static unsigned char last_buttons;
static unsigned char last_txv;
static unsigned char last_state;
static unsigned char last_rssi;
static unsigned char previous_buttons;

union inputs_status {
	struct {
		unsigned sw1            : 1;
		unsigned sw2            : 1;
		unsigned sw3            : 1;
		unsigned sw4            : 1;
		unsigned sw5            : 1;
		unsigned sw6            : 1;
		unsigned sw7            : 1;
		unsigned sw8            : 1;
	} s;
	unsigned char v;
};

static union inputs_status inputs_status;

union switch_events {
	struct {
		unsigned sw1    : 1; /* short press */
		unsigned sw2    : 1;
		unsigned sw3    : 1;
		unsigned sw4    : 1;
		unsigned sw5    : 1;
		unsigned sw6    : 1;
		unsigned sw7    : 1;
		unsigned sw8    : 1;
		unsigned sw1l   : 1; /* long press */
		unsigned sw2l   : 1;
		unsigned sw3l   : 1;
		unsigned sw4l   : 1;
		unsigned sw5l   : 1;
		unsigned sw6l   : 1;
		unsigned sw7l   : 1;
		unsigned sw8l   : 1;
	} s;
	unsigned short v;
};

static union switch_events switch_events;
static unsigned short sw1_count;
static unsigned short sw2_count;
static unsigned short sw3_count;
static unsigned short sw4_count;
static unsigned short sw5_count;
static unsigned short sw6_count;
static unsigned short sw7_count;
static unsigned short sw8_count;

#define BUTTON_1 ((unsigned char)(1 << 0))
#define BUTTON_2 ((unsigned char)(1 << 1))
#define BUTTON_3 ((unsigned char)(1 << 2))
#define BUTTON_4 ((unsigned char)(1 << 3))
#define BUTTON_5 ((unsigned char)(1 << 4))
#define BUTTON_6 ((unsigned char)(1 << 5))
#define BUTTON_7 ((unsigned char)(1 << 6))
#define BUTTON_8 ((unsigned char)(1 << 7))

static int command_received_heading;
static unsigned char received_auto_mode;
static int command_target_heading;
static unsigned char nmea2000_command_address;
static unsigned int last_command_data;
static char received_param_slot;

static void
send_command_engage(void)
{
	__data struct private_command_engage * d = (void *)&nmea2000_data[0];
	d->auto_mode = received_auto_mode;
	d->heading = command_target_heading;
	d->params_slot = received_param_slot;
	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_COMMAND_ENGAGE >> 8) & 0xff;
	msg.id.daddr = nmea2000_command_address;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct private_command_engage);
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMMAND_ENGAGE failed\n");
}

static void
send_control_mob(void)
{
	__data struct private_remote_control *d = (void *)&nmea2000_data[0];
	d->control_type = CONTROL_MOB;
	d->control_subtype = CONTROL_MOB_MARK;

	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_REMOTE_CONTROL >> 8) & 0xff;
	msg.id.daddr = NMEA2000_ADDR_GLOBAL;
	msg.id.priority = NMEA2000_PRIORITY_SECURITY;
	msg.dlc = CONTROL_MOB_SIZE;
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_REMOTE_CONTROL failed\n");
}

static void
send_control_radio(void)
{
	__data struct private_remote_control *d = (void *)&nmea2000_data[0];
	d->control_type = CONTROL_REMOTE_RADIO;
	d->control_subtype = 0;
	d->control_data[0] = last_txv;
	d->control_data[1] = last_state;
	d->control_data[2] = last_rssi;

	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_REMOTE_CONTROL >> 8) & 0xff;
	msg.id.daddr = NMEA2000_ADDR_GLOBAL;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = CONTROL_REMOTE_RADIO_SIZE;
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_REMOTE_CONTROL failed\n");
}

static void
send_control_beep(char type) __wparam
{
	__data struct private_remote_control *d = (void *)&nmea2000_data[0];
	d->control_type = CONTROL_BEEP;
	d->control_subtype = type;

	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_REMOTE_CONTROL >> 8) & 0xff;
	msg.id.daddr = NMEA2000_ADDR_GLOBAL;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = CONTROL_BEEP_SIZE;
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_REMOTE_CONTROL failed\n");
}

void
user_handle_iso_request(unsigned long pgn)
{
	printf("ISO_REQUEST for %ld from %d\n", pgn, rid.saddr);
#if 0
	switch(pgn) {
	case PRIVATE_COMPASS_OFFSET:
		send_private_compass_offset(rid.saddr);
		break;
	case NMEA2000_RATEOFTURN:
		send_nmea2000_rateofturn();
		break;
	case NMEA2000_ATTITUDE:
		send_nmea2000_attitude();
		break;
	}
#endif
}

void
user_receive()
{
	unsigned long pgn;

	pgn = ((unsigned long)rid.page << 16) | ((unsigned long)rid.iso_pg << 8);
	if (rid.iso_pg > 239)
		pgn |= rid.daddr;

	switch(pgn) {
	case PRIVATE_COMMAND_STATUS:
	{
		struct private_command_status *command_status = (void *)rdata;
		last_command_data = timer0_read();
		nmea2000_command_address = rid.saddr;
		command_received_heading = command_status->heading;
		received_auto_mode = command_status->auto_mode;
		received_param_slot = command_status->params_slot;
		break;
	}
	}
}

static void
buttons_change(void)
{
	printf("btn 0x%x txv %d stat 0x%x rssi %d\n",
		    last_buttons, last_txv, last_state, last_rssi);
	if (last_buttons == previous_buttons)
		return;

	send_control_radio();
#define check_bx(x) \
	if ((last_buttons & BUTTON_##x) && (previous_buttons & BUTTON_##x) == 0) { \
		if (inputs_status.s.sw##x == 0) { \
			inputs_status.s.sw##x = 1; \
			sw##x##_count = timer0_read(); \
		} \
	} else if ((previous_buttons & BUTTON_##x) && (last_buttons & BUTTON_##x) == 0) { \
		if (inputs_status.s.sw##x) { \
			switch_events.s.sw##x = 1; \
			inputs_status.s.sw##x = 0; \
		} \
	}

	check_bx(1)
	check_bx(2)
	check_bx(3)
	check_bx(4)
	check_bx(5)
	check_bx(6)
	check_bx(7)
	check_bx(8)
#undef check_bx
	previous_buttons = last_buttons;
}

static void
update_heading(signed char change, char btype)
{
	if ((timer0_read() - last_command_data) > 15000)
		return;
	if (received_auto_mode == AUTO_HEAD) {
		int heading_d = rad2deg(command_received_heading);
		heading_d += change;
		if (heading_d >= 360)
			heading_d -= 360;
		else if (heading_d < 0)
			heading_d += 360;
		command_target_heading = deg2rad(heading_d);
		send_command_engage();
		send_control_beep(btype);
	}
}

void
main(void) __naked
{
	unsigned char c;
	static unsigned int poll_count;

	sid = 0;
	softintrs = 0;
	counter_10hz = 100;
	counter_1hz = 10;

	inputs_status.v = 0;
	switch_events.v = 0;
	previous_buttons = 0;

	received_auto_mode = AUTO_OFF;

	/* configure sleep mode: PRI_IDLE */
	OSCCONbits.SCS = 0;
	OSCCONbits.IDLEN = 1;

	/* everything is low priority by default */
	IPR1 = 0;
	IPR2 = 0;
	IPR3 = 0;
	IPR4 = 0;
	IPR5 = 0;
	INTCON = 0;
	INTCON2 = 0;
	INTCON3 = 0;

	RCONbits.IPEN=1; /* enable interrupt priority */

	/* configure timer0 as free-running counter at 9.765625Khz * 4 */
	T0CON = 0x07; /* b00000111: internal clock, 1/256 prescaler */
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 0; /* no interrupt */
	T0CONbits.TMR0ON = 1;

	/* configure timer2 for 1Khz interrupt */
	PMD1bits.TMR2MD=0;
	T2CON = 0x22; /* b00100001: postscaller 1/5, prescaler 1/16 */
	PR2 = 125; /* 1khz output */
	T2CONbits.TMR2ON = 1;
	PIR1bits.RC1IF = 0;
	PIR1bits.TMR2IF = 0;
	IPR1bits.TMR2IP = 1; /* high priority interrupt */
	PIE1bits.TMR2IE = 1;

	USART_INIT(0);

	/*
	 * setup UART1 for 19200bps receive
	 * radio enable is connected to TX1, so we also have to enable TX1
	 */
	RCSTA1 = 0x10; /* enable RX */
	TXSTA1 = 0x20; /* enable TX */
	BAUDCON1 = 0x10; /* TX1 low-level idle state */
	SPBRG1 = 7;
	PIE1bits.RC1IE = 1;

	INTCONbits.GIE_GIEH=1;  /* enable high-priority interrupts */
	INTCONbits.PEIE_GIEL=1; /* enable low-priority interrrupts */
	

	/* read user-id from config bits */
	nmea2000_user_id = 0;
	__asm
	movlw UPPER __IDLOC0
	movwf _TBLPTRU, a
	movlw HIGH __IDLOC0
	movwf _TBLPTRH, a
	movlw LOW __IDLOC0
	movwf _TBLPTRL, a
	tblrd*+;
	movff _TABLAT, _nmea2000_user_id;
	tblrd*+;
	movff _TABLAT, _nmea2000_user_id + 1;
	__endasm;

	nmea2000_init();

	stdout = STREAM_USER; /* Use the macro PUTCHAR with printf */

	/* enable watch dog timer */
	WDTCON = 0x01;

	printf("\nready");
	poll_count = timer0_read();
	while (nmea2000_addr_status != ADDR_STATUS_OK) {
		nmea2000_poll(5);
		while ((timer0_read() - poll_count) < TIMER0_5MS) {
			nmea2000_receive();
		}
		poll_count = timer0_read();
		CLRWDT;
	}

	printf(", addr %d, id %ld\n", nmea2000_addr, nmea2000_user_id);

	rrx_cnt = rrx_abuf = 0;
	RADIO_ENABLE = 1;
again:
	while (1) {
		CLRWDT;
		if (PIR5bits.RXBnIF)
			nmea2000_receive();
		if (nmea2000_addr_status == ADDR_STATUS_CLAIMING) {
			if ((timer0_read() - poll_count) > TIMER0_5MS) {
				nmea2000_poll(5);
				poll_count = timer0_read();
			}
			if (nmea2000_addr_status == ADDR_STATUS_OK) {
				printf("new addr %d\n", nmea2000_addr);
			}
		};

		if (softintrs & INT_10HZ) {
			softintrs &= ~INT_10HZ;
			counter_1hz--;
			if (counter_1hz == 0) {
				counter_1hz = 10;
			}
		}
		if (softintrs & INT_RADIO0) {
			if (rrx_buf0[0] == RADIO_SN0 &&
			    rrx_buf0[1] == RADIO_SN1 &&
			    rrx_buf0[2] == RADIO_SN2) {
				last_buttons = rrx_buf0[3];
				last_txv = rrx_buf0[5];
				last_state = rrx_buf0[6];
				last_rssi = rrx_buf0[7];
				printf("rx0 sn 0x%2x%2x%2x ",
				    rrx_buf0[2], rrx_buf0[1], rrx_buf0[0]);
				softintrs &= ~INT_RADIO0;
				buttons_change();
			}
		}
		if (softintrs & INT_RADIO1) {
			if (rrx_buf1[0] == RADIO_SN0 &&
			    rrx_buf1[1] == RADIO_SN1 &&
			    rrx_buf1[2] == RADIO_SN2) {
				last_buttons = rrx_buf1[3];
				last_txv = rrx_buf1[5];
				last_state = rrx_buf1[6];
				last_rssi = rrx_buf1[7];
				printf("rx1 sn 0x%2x%2x%2x ",
				    rrx_buf1[2], rrx_buf1[1], rrx_buf1[0]);
				softintrs &= ~INT_RADIO1;
				buttons_change();
			}
		}
#define check_bxl(x) \
	if (inputs_status.s.sw##x) { \
		if (timer0_read() - sw##x##_count > 10000) { \
			switch_events.s.sw##x##l = 1; \
			inputs_status.s.sw##x = 0; \
		} \
	}
		check_bxl(1)
		check_bxl(2)
		check_bxl(3)
		check_bxl(4)
		check_bxl(5)
		check_bxl(6)
		check_bxl(7)
		check_bxl(8)
#undef check_bxl

		if (switch_events.s.sw1) {
			update_heading(-1, CONTROL_BEEP_SHORT);
			switch_events.s.sw1 = 0;
		}
		if (switch_events.s.sw2) {
			update_heading(+1, CONTROL_BEEP_SHORT);
			switch_events.s.sw2 = 0;
		}
		if (switch_events.s.sw4) {
			update_heading(-10, CONTROL_BEEP_SHORT);
			switch_events.s.sw4 = 0;
		}
		if (switch_events.s.sw5) {
			update_heading(+10, CONTROL_BEEP_SHORT);
			switch_events.s.sw5 = 0;
		}
		if (switch_events.s.sw4l) {
			update_heading(-90, CONTROL_BEEP_LONG);
			switch_events.s.sw4l = 0;
		}
		if (switch_events.s.sw5l) {
			update_heading(+90, CONTROL_BEEP_LONG);
			switch_events.s.sw5l = 0;
		}
		if (switch_events.s.sw3l) {
			send_control_mob();
			switch_events.s.sw3l = 0;
		}

		if (RCREG2 == 'r')
			break;

		if (nmea2000_addr_status == ADDR_STATUS_OK) {
			__asm
			SLEEP
			__endasm;
		}
	}

end:
	while ((c = getchar()) != 'r') {
		printf("got %c\n", c);
		goto again;
	}
	printf("returning\n");
	while (PIE3bits.TX2IE)
		; /* wait for transmit to complete */
	INTCONbits.PEIE=0; /* disable peripheral interrupts */
	INTCONbits.GIE=0;  /* disable interrrupts */
}

unsigned int
timer0_read() __naked
{
	/* return TMR0L | (TMR0H << 8), reading TMR0L first */
	__asm
	movf    _TMR0L, w
	movff   _TMR0H, _PRODL
	return
	__endasm;
}

/* Vectors */
void _reset (void) __naked __interrupt 0
{
	__asm__("goto __startup");
}


void _startup (void) __naked
{

  __asm
    // Initialize the stack pointer
    lfsr 1, _stack_end
    lfsr 2, _stack_end
    clrf _TBLPTRU, 0    // 1st silicon doesn't do this on POR

    // initialize the flash memory access configuration. this is harmless
    // for non-flash devices, so we do it on all parts.
    bsf _EECON1, 7, 0
    bcf _EECON1, 6, 0
    __endasm ;

  /* Call the user's main routine */
  main();
  __asm__("reset");
}

/*
 * high priority interrupt. Split in 2 parts; one for the entry point
 * where we'll deal with timer0, then jump to another address
 * as we don't have enough space before the low priority vector
 */
void _irqh (void) __naked __shadowregs __interrupt 1
{
	__asm
	bcf   _PIR1, 1
	goto _irqh_timer2
	__endasm ;

}

void irqh_timer2(void) __naked
{
	/*
	 * no sdcc registers are automatically saved,
	 * so we have to be carefull with C code !
	 */
	counter_10hz--;
	if (counter_10hz == 0) {
		counter_10hz = 100;
		softintrs |= INT_10HZ;
	}			
	__asm
	retfie 1
	nop
	__endasm;
}

void _irq (void) __interrupt 2 /* low priority */
{
	unsigned char c;
	USART_INTR;
	if (PIR5 != 0) {
		nmea2000_intr();
	}
	if (PIR5bits.RXBnIF)
		PIE5bits.RXBnIE = 0; /* main loop will check */

	if (PIR5 != 0)
		PIE5 &= _PIE5_TXBnIE;
	while (PIR1bits.RC1IF) {
		c = RCREG1;
		if (c == 0x0a) {
			/* ignore */
			;
		} else if (c == 0x0d) {
			if (rrx_cnt == 8) {
				/* all good, switch rx bufs and signal */
				if (rrx_abuf == 0) {
					softintrs |= INT_RADIO0;
					rrx_abuf = 1;
				} else {
					softintrs |= INT_RADIO1;
					rrx_abuf = 0;
				}
				rrx_cnt = 0;
			} else {
				/* protocol error, just reset */
				rrx_cnt = 0;
			}
		} else {
			if (rrx_cnt >= 8) {
				/* protocol error, just reset */
				rrx_cnt = 0;
			} else {
				if (rrx_abuf == 0) {
					rrx_buf0[rrx_cnt] = c;
				} else {
					rrx_buf1[rrx_cnt] = c;
				}
				rrx_cnt++;
			}
		}
		if (RCSTA1bits.OERR) {
			RCSTA1bits.CREN = 0;
			RCSTA1bits.CREN = 1;
		}

	}
}
