/* $Id: main.c,v 1.20 2019/03/11 21:24:53 bouyer Exp $ */
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
#include <math.h>
#include <autopilot_serial.h>
#include <nmea2000.h>
#include <nmea2000_pgn.h>
#include <eeprom.h>
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

static int received_heading;
#define HEADING_INVALID 32767
static int received_pitch;
static int received_roll;
static int received_rot_rate;
static int previous_rot_rate;

unsigned int acuator_time;
unsigned int last_capteur_data;

unsigned int timer0_read(void);
#pragma callee_saves timer0_read

static char counter_10hz;
static char counter_1hz;
static volatile union softintrs {
	struct softintrs_bits {
		char int_10hz : 1;	/* 0.1s timer */
		char int_att  : 1;	/* got new attitude message */
		char int_rot  : 1;	/* got new rate of turn message */
		char int_ledo : 1;	/* toggle LED in timer intr handler */
	} bits;
	char byte;
} softintrs;

static int target_heading; /* heading to follow */
static unsigned char auto_mode;

#define CLRWDT __asm__("clrwdt")

#define EN_ALL LATCbits.LATC1
#define EN_A   PORTAbits.RA6
#define EN_B   PORTAbits.RA1
#define IN_A   LATCbits.LATC0
#define IN_B   LATAbits.LATA0
#define PWM    LATCbits.LATC2

#define LEDR   LATBbits.LATB3
#define LEDG   LATBbits.LATB2

static unsigned char led_expire;
static unsigned char led_wait;

#define SPKR_PORT	LATAbits.LATA3
/* beep freq: 19.53125Khz / x */
#define SPKR_440        44 /* 440Hz */
#define SPKR_1000       19 /* 1Khz */
static unsigned char beep_duration;
#define SPKR_440_D100   88 /* 0.1s */
#define SPKR_1000_D100  200 /* 0.1s */

static union command_errors err_list, err_ack;

int a2d_motorcurrent; /* AN2 */
int a2d_rudder; /* AN9 */
int a2d_acc;
unsigned char a2d_idx;
unsigned char motor_overcurrent_count;
#define MOTOR_OVERCURRENT_MAX 3 /* 0.3s */

#define RUDDER_MIN (3072 >> 4)
#define RUDDER_MAX (29736 >> 4)

/*
 * rudder PID parameters. half move (13330 / 16) = 833 in about 4s,
 * sample every 0.1s
 * so max speed is about +/-20 for an output of +/-125
 */
#define RUDDER_ERR_FACTOR   -100L
#define RUDDER_DIFF_FACTOR  -350L
#define RUDDER_INT_FACTOR  -1L
#define RUDDER_DIVIDE_FACTOR 64L

int rudder_cons;
int previous_rudder;
int previous_acuator_cmd;

/*
 * steering PID parameters. 
 * For output in R (inputs are in R, R/s and R.s * 10) (with R = rad * 10000)
 */
#define NPARAMS 6
struct steering_factors {
	int  factors[3];
} steering_factors[NPARAMS];

static char steering_factors_slot;

static float target_rudder;

static void
read_factors_from_eeprom(void)
{
	char *d = (void *)&steering_factors[0];
	char i, j;
	for (i = 0; i < sizeof(struct steering_factors) * NPARAMS; i++)
		d[i] = eeprom_read(i);
	for (i = 0; i < NPARAMS; i++) {
		for (j = 0; j < 3; j++) {
			if (steering_factors[i].factors[j] < 0 ||
			    steering_factors[i].factors[j] > 9999) {
				steering_factors[i].factors[j] = 0;
			}
		}
	}
}

static void
write_factors_to_eeprom(char slot) __wparam
{
	char *d = (void *)&steering_factors[slot];
	char i;
	for (i = 0; i < sizeof(struct steering_factors); i++)
		eeprom_write(i + slot * sizeof(struct steering_factors), d[i]);
}

static void
send_command_status(void)
{
	long rudder_report;
	struct private_command_status *data = (void *)&nmea2000_data[0];

	rudder_report = a2d_rudder - (RUDDER_MIN + RUDDER_MAX) / 2;
	rudder_report = rudder_report * 200 / (RUDDER_MAX - RUDDER_MIN);
	PGN2ID(PRIVATE_COMMAND_STATUS, msg.id);
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct private_command_status);
	msg.data = &nmea2000_data[0];
	data->heading = target_heading;
	data->auto_mode = auto_mode;
	data->command_errors.byte = err_list.byte;
	data->rudder = rudder_report;
	data->params_slot = steering_factors_slot;
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMMAND_STATUS failed\n");
}

static void
send_command_factors(char addr, char slot)
{
	struct private_command_factors *data = (void *)&nmea2000_data[0];

	msg.id.id = 0;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.id.iso_pg = (PRIVATE_COMMAND_FACTORS >> 8) & 0xff;
	msg.id.daddr = addr;
	msg.dlc = sizeof(struct private_command_factors);
	msg.data = &nmea2000_data[0];
	data->slot = slot;
	data->factors[0] = steering_factors[slot].factors[0];
	data->factors[1] = steering_factors[slot].factors[1];
	data->factors[2] = steering_factors[slot].factors[2];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMMAND_FACTORS failed\n");
}

void
user_handle_iso_request(unsigned long pgn)
{
	printf("ISO_REQUEST for %ld from %d\n", pgn, rid.saddr);
	switch(pgn) {
	case PRIVATE_COMMAND_STATUS:
		send_command_status();
		break;
	}
}

void
user_receive()
{
	unsigned long pgn;
	unsigned char decoded=0;

	pgn = ((unsigned long)rid.page << 16) | ((unsigned long)rid.iso_pg << 8);
	if (rid.iso_pg > 239)
		pgn |= rid.daddr;

	switch(pgn) {
	case NMEA2000_ATTITUDE:
	{
		struct nmea2000_attitude_data *d = (void *)rdata;
		decoded = 1;
		received_heading = d->yaw;
		received_pitch = d->pitch;
		received_roll = d->roll;
		softintrs.bits.int_att = 1;
		break;
	}
	case NMEA2000_RATEOFTURN:
	{
		struct nmea2000_rateofturn_data *d = (void *)rdata;
		decoded = 1;
		received_rot_rate = d->rate / 1000;
		softintrs.bits.int_rot = 1;
		break;
	}
	case PRIVATE_COMMAND_ACUATOR:
		if (auto_mode & AUTO_STANDBY) {
			struct private_command_acuator *d = (void *)rdata;
			char move = d->move;
			decoded = 1;
#ifdef FAKE
			a2d_rudder = a2d_rudder + (move * 20);
			if (a2d_rudder < RUDDER_MIN)
				a2d_rudder = RUDDER_MIN;
			if (a2d_rudder > RUDDER_MAX)
				a2d_rudder = RUDDER_MAX;
#else
			if (move < 0) {
				IN_A = 1;
				IN_B = 0;
				move = -move;
			} else {
				IN_A = 0;
				IN_B = 1;
			}
			if (CCPR2L == 0)
				CCPR2L = 10;
			CCPR2L += 10 * move;
			CCP2CON = 0x0c; /* PWM on */
#endif
			acuator_time = timer0_read();

		}
		break;
	case PRIVATE_COMMAND_ENGAGE:
	{
		struct private_command_engage *d = (void *)rdata;
		char need_msg_update;
		if (target_heading != d->heading ||
		    auto_mode != d->auto_mode)
			need_msg_update = 1;
		else
			need_msg_update = 0;
		/* chech value */
		if (d->auto_mode == AUTO_OFF) {
			LEDR = LEDG = 0;
			softintrs.bits.int_ledo = 0;
			EN_ALL = 0;
			IN_A = 0;
			IN_B = 0;
			PWM = 0;
			CCPR2L = 0;
			CCP2CON = 0; /* PWM off */
			target_heading = HEADING_INVALID;
		} else if (d->auto_mode & AUTO_STANDBY) {
			LEDR = LEDG = 0;
			softintrs.bits.int_ledo = 0;
			EN_ALL = 1;
			IN_A = 0;
			IN_B = 0;
			PWM = 0;
			CCPR2L = 0;
			CCP2CON = 0; /* PWM off */
			target_heading = HEADING_INVALID;
		} else if (d->auto_mode & AUTO_HEAD) {
			if ((auto_mode & AUTO_HEAD) == 0) {
				LEDR = LEDG = 0;
				softintrs.bits.int_ledo = 0;
				auto_mode = AUTO_HEAD;
				/* init rudder pos */
				rudder_cons = previous_rudder =
				    a2d_rudder;
				/* init PID */
				previous_rot_rate = received_rot_rate;
				/* power on but idle */
				EN_ALL = 1;
				IN_A = 0;
				IN_B = 0;
				PWM = 0;
				CCPR2L = 0;
				CCP2CON = 0; /* PWM off */
			}
			target_heading = d->heading;
			steering_factors_slot = d->params_slot;
		}
		auto_mode = d->auto_mode;
		if (need_msg_update)
			send_command_status();
		decoded = 1;
		break;
	}
	case PRIVATE_COMMAND_ERRACK:
	{
		struct private_command_errack *d = (void *)rdata;
		decoded = 1;
		err_ack.byte = d->ack_errors.byte;
		err_list.byte &= ~d->ack_errors.byte;
		send_command_status();
		break;
	}
	case PRIVATE_COMMAND_FACTORS_REQUEST:
	{
		struct private_command_factors_request *d = (void *)rdata;
		decoded = 1;
		if (d->slot >= 0 && d->slot < NPARAMS) {
			send_command_factors(rid.saddr, d->slot);
		}
		break;
	}
	case PRIVATE_COMMAND_FACTORS:
	{
		struct private_command_factors *d = (void *)rdata;
		decoded = 1;
		printf("factors[%d]=%d %d %d\n", d->slot,
		    d->factors[0],
		    d->factors[1],
		    d->factors[2]);
		if (d->slot >= 0 && d->slot < NPARAMS) {
			steering_factors[d->slot].factors[0] =
			    d->factors[0];
			steering_factors[d->slot].factors[1] =
			    d->factors[1];
			steering_factors[d->slot].factors[2] =
			    d->factors[2];
			write_factors_to_eeprom(d->slot);
		}
		break;
	}
	}
#if 0
	if (decoded == 0) {
		unsigned char i;
		printf("got msg %6d %3d\n",
		   pgn, rid.saddr);
		printf("data:");
		for (i =0; i < 8; i++) {
			printf("%02x", rdata[i]);
		}
		putchar('\n');
	}
	else {
		printf("head %d pitch %d roll %d rate %d\n",
			received_heading,
			received_pitch,
			received_roll,
			received_rot_rate);
	}
#endif
}

/* compute needed acuator move */
static unsigned char
move_acuator(void)
{
#ifdef FAKE
	long rudder_error;

	rudder_error = rudder_cons - a2d_rudder;

	/* 5s for full to full, slow down for small moves */
	if (rudder_error < 10 && rudder_error >= -10)
		return 0;
	if (rudder_error > (RUDDER_MAX - RUDDER_MIN) / 50)
		rudder_error = (RUDDER_MAX - RUDDER_MIN) / 50;
	else
		rudder_error = rudder_error / 2;
	a2d_rudder += rudder_error;
	return 1;
#else
	long rudder_error;
	long rudder_speed;
	long acuator_cmd;
	int acuator_cmd_soft;

	rudder_error = rudder_cons - a2d_rudder;
	rudder_speed = previous_rudder - a2d_rudder;
	previous_rudder = a2d_rudder;

	if (rudder_error < 10 && rudder_error >= -10)
		return 0;
	acuator_cmd = (rudder_error * RUDDER_ERR_FACTOR) +
		      (rudder_speed * RUDDER_DIFF_FACTOR);
	acuator_cmd = acuator_cmd / RUDDER_DIVIDE_FACTOR;
	if ((acuator_cmd > 0 && previous_acuator_cmd < 0) ||
	    (acuator_cmd < 0 && previous_acuator_cmd > 0)) {
		/* fast invert; stop first */
		acuator_cmd = acuator_cmd_soft = 0;
	} else if ((acuator_cmd > 20 && acuator_cmd > previous_acuator_cmd) ||
		   (acuator_cmd < -20 && acuator_cmd < previous_acuator_cmd)) {
		/* we want to go faster */
		acuator_cmd_soft = (acuator_cmd + previous_acuator_cmd) / 2;
	} else {
		/* going slower; no need to be progressive */
		acuator_cmd_soft = acuator_cmd;
	}

	if (acuator_cmd_soft > 20) {
		IN_A = 0;
		IN_B = 1;
		LEDG = 1;
		LEDR = 0;
		softintrs.bits.int_ledo = 0;
		led_wait = 5;
		if (acuator_cmd_soft >= 125) {
			PWM = 1;
			CCP2CON = 0;
		} else {
			PWM = 0;
			CCPR2L = acuator_cmd_soft;
			CCP2CON = 0x0c;
		}
	} else if (acuator_cmd_soft < -20) {
		IN_A = 1;
		IN_B = 0;
		LEDG = 0;
		LEDR = 1;
		softintrs.bits.int_ledo = 0;
		led_wait = 5;
		if (acuator_cmd_soft <= -125) {
			PWM = 1;
			CCP2CON = 0;
		} else {
			PWM = 0;
			CCPR2L = -acuator_cmd_soft;
			CCP2CON = 0x0c;
		}
	} else {
		if (previous_acuator_cmd != 0) {
			if (led_expire == 0) {
				LEDG = 0;
				LEDR = 0;
			}
			IN_A = 0;
			IN_B = 0;
			PWM = 0;
			CCPR2L = 0;
			CCP2CON = 0;
		}
		acuator_cmd = 0;
	}
	previous_acuator_cmd = acuator_cmd;
	return 1;
#endif
}

static void
compute_rudder_cons(void)
{
	long heading_error;
	float rudder_correct;
	int new_rudder_int;
	int d_rot;

	if (received_heading == HEADING_INVALID)
		return;

	heading_error = (long)received_heading - (long)target_heading;

	if (heading_error >= 31416L)
		heading_error = heading_error - 62832L;
	else if (heading_error <= -31416L)
		heading_error = 62832L + heading_error;

	/* compute rotational acceleration */
	d_rot = (received_rot_rate - previous_rot_rate) * 10;
	previous_rot_rate = received_rot_rate;

	rudder_correct =
	  (float)heading_error *
	    (float)steering_factors[steering_factors_slot].factors[FACTOR_ERR] +
	  (float)received_rot_rate *
	    (float)steering_factors[steering_factors_slot].factors[FACTOR_DIF] +
	  (float)d_rot *
	    (float)steering_factors[steering_factors_slot].factors[FACTOR_DIF2];
	/*
	 * we have to divide rudder_correct by 100000 because the
	 * factors are * 100000. 
	 * then convert to rudder unit. RUDDER_MAX - RUDDER_MIN is
	 * about 60 deg, or 1047 millirads */
	/* convert back to deg */
#if 0
	rudder_correct = (rudder_correct / 100000 *
			(float)((RUDDER_MAX - RUDDER_MIN) / 1047 ));
#endif
	rudder_correct = (rudder_correct * 0.000000009551098 *
	    (float)((RUDDER_MAX - RUDDER_MIN)));

	target_rudder += rudder_correct;
	if (target_rudder < RUDDER_MIN)
		target_rudder = RUDDER_MIN;
	else if (target_rudder > RUDDER_MAX)
		target_rudder = RUDDER_MAX;
	new_rudder_int = target_rudder;
#if 0
	if (new_rudder_int < RUDDER_MIN + (RUDDER_MAX - RUDDER_MIN) / 3)
		new_rudder_int = RUDDER_MIN + (RUDDER_MAX - RUDDER_MIN) / 3;
	else if (new_rudder_int > RUDDER_MAX - (RUDDER_MAX - RUDDER_MIN) / 3)
		new_rudder_int = RUDDER_MAX - (RUDDER_MAX - RUDDER_MIN) / 3;
#endif

	/* try to avoid unneeded small moves */
	if (new_rudder_int - rudder_cons > 0 || 
	    new_rudder_int - rudder_cons < -0) {
		rudder_cons = new_rudder_int;
	}
}

void
main(void) __naked
{
	unsigned char c;
	static unsigned int poll_count;
	static unsigned char acuator_temp_off;
	unsigned int count;
	static unsigned int conv_count;
	static unsigned char motor_overcurrent_count;

	sid = 0;

	softintrs.byte = 0;
	counter_10hz = 100;
	counter_1hz = 10;

	auto_mode = AUTO_OFF;
	target_heading = HEADING_INVALID;
	led_wait = 5;
	received_heading = HEADING_INVALID;

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

	/* configure timer0 as free-running counter at 9.765625Khz */
	T0CON = 0x07; /* b00000111: internal clock, 1/256 prescaler */
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 0; /* no interrupt */
	T0CONbits.TMR0ON = 1;

	/* configure timer2 for 1Khz interrupt */
	PMD1bits.TMR2MD=0;
	T2CON = 0x21; /* b00100001: postscaller 1/5, prescaler 1/4 */
	PR2 = 125; /* 1khz output */
	T2CONbits.TMR2ON = 1;
	PIR1bits.TMR2IF = 0;
	IPR1bits.TMR2IP = 1; /* high priority interrupt */
	PIE1bits.TMR2IE = 1;

	/* prepare timer4 for speaker drive */
	PMD1bits.TMR4MD=0;
	T4CON = 0x2f; /* b00101111: postscaller 1/16, prescaler 1/4 */
	PR4 = 255; /* 150Hz by default */
	beep_duration = 0;
	T4CONbits.TMR4ON = 0; /* keep off for now */
	PIR4bits.TMR4IF = 0;
	IPR4bits.TMR4IP = 1; /* high priority interrupt */
	PIE4bits.TMR4IE = 1;

	USART_INIT(0);

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

	/* configure inputs/outputs */
	ANCON0 = 0xfd;
	ANCON1 = 0xff;
	/* shutdown power module */
	EN_ALL = 0;
	TRISCbits.TRISC1 = 0;
	/* shutdown PWM output */
	PWM = 0;
	TRISCbits.TRISC2 = 0;
	/* direction inputs to 0 */
	IN_A = 0;
	IN_B = 0;
	TRISCbits.TRISC0 = 0;
	TRISAbits.TRISA0 = 0;

	/* prepare pwm2: use timer2 */
	CCPTMRSbits.C2TSEL = 0;

	/* spkr off */
	SPKR_PORT = 0;
	TRISAbits.TRISA3 = 0;

	/* LED on, red */
	LEDR = 1;
	LEDG = 0;
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 0;

	stdout = STREAM_USER; /* Use the macro PUTCHAR with printf */

	printf("Hello world\n");

#ifndef FAKE
	/* set up ADC */
	PIR1bits.ADIF = 0;
	ADCON0 = (2 << 2); /* select channel 2 */
	ADCON1 = 0x00;
	ADCON2 = 0xbd; /* Right justified, 20Tad Aq, Fosc/16 */
	ADCON0bits.ADON = 1;
	ADCON0bits.GO = 1; /* start */
#endif
	a2d_acc = 0;
	a2d_idx = 8;

	/* enable watch dog timer */
	WDTCON = 0x01;

	printf("ready");
	poll_count = timer0_read();
	while (nmea2000_addr_status != ADDR_STATUS_OK) {
		nmea2000_poll(5);
		while ((timer0_read() - poll_count) < 48) {
			nmea2000_receive();
		}
		poll_count = timer0_read();
		CLRWDT;
	}

	LEDR = 0;
	LEDG = 1;
	printf(", addr %d, id %ld\n", nmea2000_addr, nmea2000_user_id);
	PR4 = SPKR_440;
	T4CONbits.TMR4ON = 1;
	count = timer0_read();
	while (timer0_read() - count < 10000)
		; // wait
	T4CONbits.TMR4ON = 0;
	SPKR_PORT = 0;
	LEDG = 0;
	conv_count = 0;
	motor_overcurrent_count = 0;
	a2d_rudder = (RUDDER_MIN + RUDDER_MAX) / 2;
	a2d_motorcurrent = 0;
	err_list.byte = err_ack.byte = 0;

	previous_acuator_cmd = 0;
	read_factors_from_eeprom();

#if 0
	steering_factors[0].factors[0] = 1000;
	steering_factors[0].factors[1] = 2000;
	steering_factors[0].factors[2] = 10;
#endif
	steering_factors_slot = 0;

again:
	while (1) {
		CLRWDT;
		if (PIR1bits.ADIF) {
			PIR1bits.ADIF = 0;
			a2d_acc += ((unsigned int)ADRESH << 8) | ADRESL;
			a2d_idx--;
			if (a2d_idx == 0) {
				ADCON0bits.ADON = 0;
				if ((ADCON0 & 0x7c) == (2 << 2)) {
					/* we got motor current */
					a2d_motorcurrent = a2d_acc;
					/* switch to rudder */
					ADCON0 = (9 << 2);
					ADCON1 = 0x00;
				} else {
					/* we got rudder */
					a2d_rudder = a2d_acc >> 4;
					/* switch to motor */
					ADCON0 = (2 << 2);
					ADCON1 = 0x20;
					conv_count++;
				}
				ADCON0bits.ADON = 1;
				a2d_acc = 0;
				a2d_idx = 8;
			}
			ADCON0bits.GO = 1; /* start */
		}

		if ((auto_mode & AUTO_STANDBY) != 0 &&
		    (timer0_read() - acuator_time) > 2000) {
			IN_A = 0;
			IN_B = 0;
			CCPR2L = 0;
			CCP2CON = 0; /* PWM off */
		}

		if (softintrs.bits.int_10hz) {
			unsigned char needs_status_update = 0;
			softintrs.bits.int_10hz = 0;
			if (led_expire > 0) {
				led_expire--;
				if (led_expire == 0) {
					LEDR = LEDG = 0;
					softintrs.bits.int_ledo = 0;
				}
			}
#ifndef FAKE
			if (acuator_temp_off > 0) {
				acuator_temp_off--;
				if (acuator_temp_off == 0) {
					EN_ALL = 1;
					err_list.bits.output_overload = 0;
					err_ack.bits.output_overload = 0;
				}
			}
			if (a2d_motorcurrent > 12000) {
				motor_overcurrent_count++;
				if (motor_overcurrent_count >=
				    MOTOR_OVERCURRENT_MAX) {
					printf("E %d", a2d_motorcurrent);
					EN_ALL = 0;
					acuator_temp_off = 10;
					if (!err_list.bits.output_overload &&
					    !err_ack.bits.output_overload) {
						err_list.bits.output_overload = 1;
					}
				}
			} else {
				motor_overcurrent_count = 0;
			}
			if (EN_ALL && (EN_A == 0 || EN_B == 0)) {
				if (!err_list.bits.output_error &&
				    !err_ack.bits.output_error) {
					err_list.bits.output_error = 1;
				}
			} else {
				err_list.bits.output_error = 0;
				err_ack.bits.output_error = 0;
			}
			if (a2d_rudder < RUDDER_MIN - 30 ||
			    a2d_rudder > RUDDER_MAX + 30) {
				if (!err_list.bits.no_rudder_data &&
				    !err_ack.bits.no_rudder_data) {
					err_list.bits.no_rudder_data = 1;
				}
			} else {
				err_list.bits.no_rudder_data = 0;
				err_ack.bits.no_rudder_data = 0;
			}
#endif /* !FAKE*/

			counter_1hz--;
			if (counter_1hz == 0) {
				counter_1hz = 10;
				if (--led_wait == 0) {
					if (LEDR == 0 && LEDG == 0) {
						LEDR = 1;
						led_expire = 1;
					}
					if (auto_mode == AUTO_OFF) {
						led_wait = 5;
					} else if (auto_mode & AUTO_STANDBY) {
						softintrs.bits.int_ledo = 1;
						led_wait = 1;
					} else if (auto_mode & AUTO_HEAD) {
						softintrs.bits.int_ledo = 1;
						led_wait = 5;
					}
				}
				printf("A/D c/s %3d I %5d r %4d rc %4d m %d t %6d h %6d R %6d\n",
				    conv_count,
				    a2d_motorcurrent, a2d_rudder, rudder_cons,
				    auto_mode, target_heading,
				    received_heading, received_rot_rate);
				conv_count = 0;
				needs_status_update = 1;
			}
			if ((auto_mode & AUTO_HEAD) != 0) {
				needs_status_update |= move_acuator();
			}
			if (needs_status_update) {
				send_command_status();
			}
		}
		if (PIR5bits.RXBnIF)
			nmea2000_receive();

		if (nmea2000_addr_status == ADDR_STATUS_CLAIMING) {
			if ((timer0_read() - poll_count) > 48) {
				nmea2000_poll(5);
				poll_count = timer0_read();
			}
		}
		if (softintrs.bits.int_rot && softintrs.bits.int_att) {
			/* new data available */
			softintrs.bits.int_rot = 0;
			softintrs.bits.int_att = 0;
			last_capteur_data = timer0_read();
			err_list.bits.no_capteur_data = 0;
			err_ack.bits.no_capteur_data = 0;
			if (auto_mode & AUTO_HEAD) {
				compute_rudder_cons();
			}
		}
                if ((timer0_read() - last_capteur_data) > 50000) {
			if (!err_list.bits.no_capteur_data &&
			    !err_ack.bits.no_capteur_data) {
				err_list.bits.no_capteur_data = 1;
			}
		}
		if (err_list.byte != 0) {
			if (T4CONbits.TMR4ON == 0) {
				PR4 = SPKR_1000;
				T4CONbits.TMR4ON = 1;
				beep_duration = 0;
				send_command_status();
			}
		} else {
			T4CONbits.TMR4ON = 0;
			SPKR_PORT = 0;
		}

		if (PIR3bits.RC2IF) {
		switch (c = RCREG2) {
#if 0
		case '0':
			/* shut down power module */
			EN_ALL = 0;
			PWM = 0;
			IN_A = 0;
			IN_B = 0;
			break;
		case '1':
			/* enable power module, with outputs to ground */
			EN_ALL = 1;
			IN_A = 0;
			IN_B = 0;
			PWM = 0;
			CCP2CON = 0x00; /* PWM off */
			break;
		case 't':
			if (EN_ALL) {
				/* rotate clockwise, slow */
				if (CCP2CON == 0) {
					PWM = 0;
					IN_A = 1;
					IN_B = 0;
					CCPR2L = 20;
					CCP2CON = 0x0c; /* PWM on */
				} else {
					CCPR2L += 10;
				}
				printf("pwm %d\n", CCPR2L);
			}
			break;
		case 'b':
			if (EN_ALL) {
				/* rotate counterclockwise, slow */
				if (CCP2CON == 0) {
					PWM = 0;
					IN_A = 0;
					IN_B = 1;
					CCPR2L = 20;
					CCP2CON = 0x0c; /* PWM on */
				} else {
					CCPR2L += 10;
				}
				printf("pwm %d\n", CCPR2L);
			}
			break;
		case 'T':
			if (EN_ALL) {
				/* rotate clockwise, fast */
				IN_A = 1;
				IN_B = 0;
				PWM = 1;
				CCP2CON = 0x00; /* PWM off */
			}
			break;
		case 'B':
			if (EN_ALL) {
				/* rotate counterclockwise, fast */
				IN_A = 0;
				IN_B = 1;
				PWM = 1;
				CCP2CON = 0x00; /* PWM off */
			}
			break;
		case ' ':
			/* stop motor */
			IN_A = 0;
			IN_B = 0;
			PWM = 0;
			CCP2CON = 0x00; /* PWM off */
			break;
#endif
		default:
			break;
		}
		} else
			c = 0;
		if (c == 'r')
			break;
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
	btfss _PIR1, 1
	bra timer4
	bcf   _PIR1, 1
	goto _irqh_timer2
timer4:
	goto _irqh_timer4
	__endasm ;

}

void irqh_timer2(void) __naked
{
	/*
	 * no sdcc registers are automatically saved,
	 * so we have to be carefull with C code !
	 */
	if (--counter_10hz == 0) {
		counter_10hz = 100;
		softintrs.bits.int_10hz = 1;
	}			
	if (softintrs.bits.int_ledo) {
		__asm
		btg	_LATBbits, 2
		btg	_LATBbits, 3
		__endasm;
	}
	__asm
	retfie 1
	nop
	__endasm;
}

void irqh_timer4(void) __naked
{
	__asm
	bcf   _PIR4, 7
	btg   _LATA, 3;
	banksel _beep_duration
	movf  _beep_duration, w
	bz    irqh_timer4_ret ; _beep_duration not used
	decfsz _beep_duration, f
	bra  irqh_timer4_ret; _beep_duration not 0, keep beep
	bcf   _T4CON, 2  ; end beep
	bcf   _LATA, 3;
	irqh_timer4_ret:
	retfie 1
	__endasm;
}

void _irq (void) __interrupt 2 /* low priority */
{
	USART_INTR;
	if (PIR5 != 0)
		nmea2000_intr();
	if (PIR5bits.RXBnIF)
		PIE5bits.RXBnIE = 0; /* main loop will check */
	if (PIR5 != 0)
		PIE5 &= _PIE5_TXBnIE;
}
