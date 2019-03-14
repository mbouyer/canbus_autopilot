/* $Id: main.c,v 1.32 2019/03/08 19:26:34 bouyer Exp $ */
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
#include <autopilot_i2c.h> 
#include <nmea2000.h> 
#include <nmea2000_pgn.h> 
#include <eeprom.h> 

#include <l3dg20reg.h>
#include <lsm303.h>

#include "marg_filter.h"

extern char stack; 
extern char stack_end;

#pragma stack 0x100 256

void _reset (void) __naked __interrupt 0;
void _startup (void) __naked;

unsigned long nmea2000_user_id;

#define GYRO_DRDY  PORTBbits.RB3
#define ACCEL_DRDY PORTBbits.RB0

/* calibration datas */
#define ACCEL_X_OFFSET	-656
#define ACCEL_Y_OFFSET	 304
#define ACCEL_Z_OFFSET	 -232

#define MAGN_X_OFFSET	115
#define MAGN_Y_OFFSET	-80
#define MAGN_Z_OFFSET	111

#define GYRO_X_OFFSET_ADDR 0x100 /* address of gyro_x_offset save in eeprom */
#define GYRO_Y_OFFSET_ADDR 0x104 /* address of gyro_y_offset save in eeprom */
#define GYRO_Z_OFFSET_ADDR 0x108 /* address of gyro_z_offset save in eeprom */
#define COMPASS_OFFSET_ADDR 0x10e /* address of compass_offset save in eeprom */
static int compass_offset;

#define COMPASS_CORRECTION_TABLE_OFFSET 0x000
#define COMPASS_CORRECTION_TABLE_SIZE 124 /* 0 to 62831 / 512 + 1 */
static short compass_correction_table[COMPASS_CORRECTION_TABLE_SIZE];

static unsigned char gyro_int_count;
static struct gyro_data gyro_data_integrate;

static unsigned char accel_int_count;
static struct accel_data accel_data_integrate;
static struct magn_data magn_data_integrate;

static unsigned char gyro_loops;

static int accel_temp;

static unsigned char sid;

static struct nmea2000_msg msg;
static unsigned char nmea2000_data[NMEA2000_DATA_LENGTH];

static void get_data_am(void);
void get_data_w(void);
static void compute_values(void);
static void calibrate(void);
static unsigned char send_fast;

unsigned int timer0_read(void);
#pragma callee_saves timer0_read

static char counter_1hz;	       
static volatile char counter_10hz;	       
static volatile unsigned int softintrs;
#define INT_10HZ	 0x0001

#define TIMER0_5MS 192 /* 48 without PLL */

#define CLRWDT __asm__("clrwdt")      

union float_char {
	float f;
	char c[4];
};

union short_char {
	short s;
	char c[2];
};

static short
get_eeprom_short(unsigned int addr)
{
	union short_char s;
	s.c[0] = eeprom_read(addr);
	s.c[1] = eeprom_read(addr + 1);
	return s.s;
}

static void
write_eeprom_short(unsigned int addr, short s)
{
	union short_char sc;
	sc.s = s;
	eeprom_write(addr + 0, sc.c[0]);
	eeprom_write(addr + 1, sc.c[1]);
}

static float
get_eeprom_float(unsigned int addr)
{
	union float_char f;
	f.c[0] = eeprom_read(addr);
	f.c[1] = eeprom_read(addr + 1);
	f.c[2] = eeprom_read(addr + 2);
	f.c[3] = eeprom_read(addr + 3);
	return f.f;
}

static void
write_eeprom_float(unsigned int addr, float f)
{
	union float_char fc;
	fc.f = f;
	eeprom_write(addr + 0, fc.c[0]);
	eeprom_write(addr + 1, fc.c[1]);
	eeprom_write(addr + 2, fc.c[2]);
	eeprom_write(addr + 3, fc.c[3]);
}

static void
get_eeprom_datas(void)
{
	unsigned char i;
	compass_offset = get_eeprom_short(COMPASS_OFFSET_ADDR);

	for (i = 0; i < COMPASS_CORRECTION_TABLE_SIZE; i++) {
		compass_correction_table[i] =
		    get_eeprom_short(COMPASS_CORRECTION_TABLE_OFFSET + i * 2);
	}
#ifdef CAPTEUR_FAKE
	w_bx = 0;
	w_by = 0;
	w_bz = 0;
#else
	w_bx = get_eeprom_float(GYRO_X_OFFSET_ADDR);
	w_by = get_eeprom_float(GYRO_Y_OFFSET_ADDR);
	w_bz = get_eeprom_float(GYRO_Z_OFFSET_ADDR);
#endif
}

static int
r2d(int r)
{
	float d;
	d = (float)r / 174.53293F;
	return (int)d;
}

static void
send_private_compass_offset(unsigned char addr) __wparam
{
	struct private_compass_offset_data *data = (void *)&nmea2000_data[0];
	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_COMPASS_OFFSET >> 8) & 0xff;
	msg.id.daddr = addr;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct private_compass_offset_data);
	msg.data = &nmea2000_data[0];
	data->offset = compass_offset;
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMPASS_OFFSET failed\n");
	printf("sent compass_offset %d to %d\n", compass_offset, addr);
}

static void
send_nmea2000_rateofturn(void)
{
	struct nmea2000_rateofturn_data *dr = (void *)&nmea2000_data[0];
	PGN2ID(NMEA2000_RATEOFTURN, msg.id);
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct nmea2000_rateofturn_data);
	msg.data = &nmea2000_data[0];
	dr->sid = sid;
	dr->rate = (long)(rate_of_turn * -10000000.0F);
	if (! nmea2000_send_single_frame(&msg))
		printf("send NMEA2000_RATEOFTURN failed\n");
}

static void
send_nmea2000_attitude(void)
{
	struct nmea2000_attitude_data *da = (void *)&nmea2000_data[0];
	msg.id.id = 0;
	PGN2ID(NMEA2000_ATTITUDE, msg.id);
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct nmea2000_attitude_data);
	msg.data = &nmea2000_data[0];
	da->sid = sid;
	da->yaw = heading;
	da->pitch = pitch;
	da->roll = roll;
	if (! nmea2000_send_single_frame(&msg))
		printf("send NMEA2000_ATTITUDE failed\n");
}

void
user_handle_iso_request(unsigned long pgn)
{
	printf("ISO_REQUEST for %ld from %d\n", pgn, rid.saddr);
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
}

void
user_receive()
{
	unsigned long pgn;

	pgn = ((unsigned long)rid.page << 16) | ((unsigned long)rid.iso_pg << 8);
	if (rid.iso_pg > 239)
		pgn |= rid.daddr;

	switch(pgn) {
	case PRIVATE_COMPASS_OFFSET:
	{
		struct private_compass_offset_data *
		    compass_offset_data = (void *)rdata;
		compass_offset =
		    compass_offset_data->offset;
		write_eeprom_short(COMPASS_OFFSET_ADDR,
		    compass_offset);
		printf("compass offset now %d from %d\n", compass_offset, rid.saddr);
		break;
	}
	case PRIVATE_CALIBRATE_COMPASS:
		calibrate();
		break;

	case PRIVATE_COMMAND_STATUS:
	{
		struct private_command_status *d = (void *)rdata;
		if (d->auto_mode & AUTO_HEAD)
			send_fast = 1;
		else
			send_fast = 0;
	}
	}
}


void
main(void) __naked
{
	unsigned char c;
	unsigned char i;
	static unsigned int poll_count;
	static unsigned int hours_count;


	sid = 0;
	softintrs = 0;
	counter_10hz = 100;
	counter_1hz = 10;

	/* switch PLL on */
	OSCTUNEbits.PLLEN = 1;

	/* configure sleep mode: PRI_IDLE */
	OSCCONbits.SCS = 0;
	OSCCONbits.IDLEN = 1;
#if 0
	/* reset eeprom datas */
	for (i = 0; i < COMPASS_CORRECTION_TABLE_SIZE; i++) {
		short true_heading;
		true_heading = ((int)i * 512 - 31415);

		/* last element is the same as first */
		if (i == COMPASS_CORRECTION_TABLE_SIZE - 1) 
			true_heading = -31415;
		write_eeprom_short(COMPASS_CORRECTION_TABLE_OFFSET + i * 2,
		    true_heading);
	}
	write_eeprom_short(COMPASS_OFFSET_ADDR, 0);
	write_eeprom_float(GYRO_X_OFFSET_ADDR, 0);
	write_eeprom_float(GYRO_Y_OFFSET_ADDR, 0);
	write_eeprom_float(GYRO_Z_OFFSET_ADDR, 0);
	__asm__("reset");
#endif
		

	get_eeprom_datas();

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
	PIR1bits.TMR2IF = 0;
	IPR1bits.TMR2IP = 1; /* high priority interrupt */
	PIE1bits.TMR2IE = 1;

	/* configure UART for 38400 Bps at 40Mhz */
	SPBRG2 = 0x0f;

	USART_INIT(0);
	I2C_INIT;
	filterInit();

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

	/* port B input 0 and 3 */
	TRISBbits.TRISB0 = 1;
	TRISBbits.TRISB3 = 1;
	ANCON1bits.ANSEL10 = 0;

	stdout = STREAM_USER; /* Use the macro PUTCHAR with printf */

	printf("compass offset is %d\n", compass_offset);
	printf("correction table:\n");
	for (i = 0; i < COMPASS_CORRECTION_TABLE_SIZE; i++) {
		heading = 512 * i - 31415;
		if (i == COMPASS_CORRECTION_TABLE_SIZE - 1)
			heading = -31415;
		printf("%3d %6d %6d\n", i, heading, (int)compass_correction_table[i]);
	}

	/* enable watch dog timer */  
	WDTCON = 0x01;

#ifndef CAPTEUR_FAKE
	for (i = 0; i < 5; i++) {
		int wait;
		c = 0;
		(void)i2c_readreg(L3DG20_ADDRESS, L3DG20_WHO_AM_I, &c, 1);
		printf("who am I read 0x%x\n", c);
		if (c == 0xd4)
			break;
		for (wait = 0; wait < 10000; wait++) 
			CLRWDT; /* wait */
	}

	if (c == 0xd4) {
		printf("who am i 0x%x\n", c);
	} else {
		printf("who am i failed (0x%d)\n", c);
		goto end;
	}
	if (i2c_readreg(L3DG20_ADDRESS, L3DG20_OUT_TEMP, &c, 1)) {
		printf("temp %d\n", c);
	} else {
		printf("temp failed\n");
		goto end;
	}

	/* setup gyro */
	i2c_writereg(L3DG20_ADDRESS, L3DG20_CTRL_REG1, 0);
	i2c_writereg(L3DG20_ADDRESS, L3DG20_CTRL_REG1,
	    L3DG20_CTRL_REG1_PD | L3DG20_CTRL_REG1_ZEN |
	    L3DG20_CTRL_REG1_XEN | L3DG20_CTRL_REG1_YEN);
	i2c_writereg(L3DG20_ADDRESS, L3DG20_CTRL_REG2,
	    0);
	i2c_writereg(L3DG20_ADDRESS, L3DG20_CTRL_REG3,
	    L3DG20_CTRL_REG3_I1_BOOT | L3DG20_CTRL_REG3_DRDY);
	i2c_writereg(L3DG20_ADDRESS, L3DG20_CTRL_REG4,
	    0);
	i2c_writereg(L3DG20_ADDRESS, L3DG20_CTRL_REG5,
	    /* L3DG20_CTRL_REG5_FIFO_EN */ 0);
	i2c_writereg(L3DG20_ADDRESS, L3DG20_FIFO_CTRL,
	    /* L3DG20_FIFO_CTRL_FM_STRTOFIO | 0 */ 0);


	/* setup accel */
	i2c_writereg(LSM303_A_ADDR, LSM303_A_CTRL_REG1,
	    LSM303_A_CTRL_REG1_ODR_10HZ | LSM303_A_CTRL_REG1_ZEN |
	    LSM303_A_CTRL_REG1_YEN | LSM303_A_CTRL_REG1_XEN);
	i2c_writereg(LSM303_A_ADDR, LSM303_A_CTRL_REG3,
	    LSM303_A_CTRL_REG3_I1_DRDY1|LSM303_A_CTRL_REG3_I1_DRDY2);
	i2c_writereg(LSM303_A_ADDR, LSM303_A_CTRL_REG4,
	    LSM303_A_CTRL_REG4_HR);

	/* setup magnetometer */
	i2c_writereg(LSM303_M_ADDR, LSM303_M_CRA_REG,
	    LSM303_M_CRA_REG_TEMPEN | LSM303_M_CRA_REG_DO15);
	i2c_writereg(LSM303_M_ADDR, LSM303_M_CRB_REG,
	    LSM303_M_CRB_REGG1_3);
	i2c_writereg(LSM303_M_ADDR, LSM303_M_MR_REG,
	    LSM303_M_MR_REG_CONT);
#endif

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

	gyro_loops = 0;
	gyro_int_count = accel_int_count = 0;
	send_fast = 0;

	accel_data_integrate.accel_x = 0;
	accel_data_integrate.accel_y = 0;
	accel_data_integrate.accel_z = 0;

	magn_data_integrate.magn_x = 0;
	magn_data_integrate.magn_y = 0;
	magn_data_integrate.magn_z = 0;

	gyro_data_integrate.gyro_x = 0;
	gyro_data_integrate.gyro_y = 0;
	gyro_data_integrate.gyro_z = 0;

	hours_count = 14400; /* save data to eeprom every 4 hours */
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

		get_data_w();
		if (softintrs & INT_10HZ) {
			softintrs &= ~INT_10HZ;
			counter_1hz--;
			get_data_am();
			if (accel_int_count > 0 && gyro_int_count > 0) {
				long fixed_heading;
				int base_heading;
				int orig_heading;
				long diff;
				unsigned char i;

				compute_values();
				orig_heading = heading;
				i = (((heading + 31415) & 0xfe00) >> 9);
				base_heading = 512 * i - 31415;
				fixed_heading = (long)heading + (long)compass_correction_table[i];
				diff = (int)compass_correction_table[i + 1] - (int)compass_correction_table[i];
				/* fixed_heading += (diff * (heading - base_heading)) / 512 */
				diff = diff * (heading - base_heading);
				diff = diff / 512;
				fixed_heading += diff;
				fixed_heading += compass_offset;
				if (fixed_heading < -31415)
					heading = fixed_heading + 62831;
				else if (fixed_heading > 31415)
					heading = fixed_heading - 62831;
				else
					heading = fixed_heading;

				if (send_fast || counter_1hz == 0) {
					send_nmea2000_attitude();
					send_nmea2000_rateofturn();
					sid++;
				}
				if (counter_1hz == 0) {
					counter_1hz = 10;
					if (nmea2000_addr_status == ADDR_STATUS_OK) {
#if 0
						printf("%02x gyro %10ld "
						    "pitch %6d roll %6d "
						    "heading %6d %6d %d\n", 
						    sid,
						    (long)(rate_of_turn * 10000000.0F),
						    pitch,
						    roll,
						    orig_heading,
						    heading,
						    (int)gyro_loops);
#else
						printf("%02x gyro %10ld "
						    "pitch %6d roll %6d "
						    "heading %6d %6d %6d %d\n", 
						    sid,
						    (long)(rate_of_turn * 10000000.0F),
						    r2d(pitch),
						    r2d(roll),
						    r2d(heading) + 180,
						    orig_heading,
						    heading,
						    (int)gyro_loops);
#endif
					}
					hours_count--;
					if (hours_count == 0) {
						hours_count = 14400;
						write_eeprom_float(
						    GYRO_X_OFFSET_ADDR, w_bx);
						write_eeprom_float(
						    GYRO_Y_OFFSET_ADDR, w_by);
						write_eeprom_float(
						    GYRO_Z_OFFSET_ADDR, w_bz);
					}
				}
			}
			gyro_loops = 0;
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
		if (c == 'c')
			calibrate();
		goto again;
	}
	printf("returning\n");
	write_eeprom_float(GYRO_X_OFFSET_ADDR, w_bx);
	write_eeprom_float(GYRO_Y_OFFSET_ADDR, w_by);
	write_eeprom_float(GYRO_Z_OFFSET_ADDR, w_bz);
	while (PIE3bits.TX2IE)
		; /* wait for transmit to complete */
	INTCONbits.PEIE=0; /* disable peripheral interrupts */
	INTCONbits.GIE=0;  /* disable interrrupts */
}

/* compute new pitch & roll, then new heading */
static void
compute_values(void)
{
	float ftemp;

	/* compute normalized accelerometer and magnetometer data */
	accel_data_integrate.accel_x -= ACCEL_X_OFFSET;
	accel_data_integrate.accel_y -= ACCEL_Y_OFFSET;
	accel_data_integrate.accel_z -= ACCEL_Z_OFFSET;
	ftemp = 1 / sqrtf((float)(
	    accel_data_integrate.accel_x * accel_data_integrate.accel_x + 
	    accel_data_integrate.accel_y * accel_data_integrate.accel_y + 
	    accel_data_integrate.accel_z * accel_data_integrate.accel_z));
	a_x = (float)(accel_data_integrate.accel_x) * ftemp;
	a_y = -(float)(accel_data_integrate.accel_y) * ftemp;
	a_z = -(float)(accel_data_integrate.accel_z) * ftemp;

	magn_data_integrate.magn_x -= MAGN_X_OFFSET;
	magn_data_integrate.magn_y -= MAGN_Y_OFFSET;
	magn_data_integrate.magn_z -= MAGN_Z_OFFSET;

	ftemp = (float)(
	    magn_data_integrate.magn_x * magn_data_integrate.magn_x +
	    magn_data_integrate.magn_y * magn_data_integrate.magn_y);
	/* Z gain != X or Y gain */
	m_z = (float)(magn_data_integrate.magn_z) * 1.122449F;
	ftemp += m_z * m_z;
	ftemp = 1 / sqrtf(ftemp);
	m_x = (float)(magn_data_integrate.magn_x) * ftemp;
	m_y = - (float)(magn_data_integrate.magn_y) * ftemp;
	m_z = - (float)(magn_data_integrate.magn_z) * ftemp;

	/* convert to rad/s, output is 8.75 md/s per digit */
	ftemp = 0.0001527163F / (float)gyro_int_count;
	w_x = (float)gyro_data_integrate.gyro_x * ftemp;
	w_y = (float)gyro_data_integrate.gyro_y * ftemp;
	w_z = - (float)gyro_data_integrate.gyro_z * ftemp;

	gyro_int_count = 0;
	accel_int_count = 0;
	gyro_data_integrate.gyro_x = 0;
	gyro_data_integrate.gyro_y = 0;
	gyro_data_integrate.gyro_z = 0;
	filterUpdate();
}

void
get_data_am(void)
{
#ifdef CAPTEUR_FAKE
	accel_int_count = 1;
	accel_data_integrate.accel_x = 100L;
	accel_data_integrate.accel_y = -200L;
	accel_data_integrate.accel_z = -1000L;
	magn_data_integrate.magn_x = 1000L;
	magn_data_integrate.magn_z = 10L;
	magn_data_integrate.magn_y = 2000L;
#else
	int data_xyz[3];
	unsigned char data[6];

	if (ACCEL_DRDY) {
		accel_int_count++;
		if (i2c_readreg(LSM303_A_ADDR, LSM303_A_OUT_X_L,
		    (char *)data_xyz, 6)) {
			accel_data_integrate.accel_x = data_xyz[0];
			accel_data_integrate.accel_y = data_xyz[1];
			accel_data_integrate.accel_z = data_xyz[2];
		} else {
			printf("read acc XYZ failed\n");
		}
		if (i2c_readreg(LSM303_M_ADDR, LSM303_M_OUT_X_H,
		    data, 6)) {
			magn_data_integrate.magn_x = (data[0] << 8 ) | data[1];
			magn_data_integrate.magn_z = (data[2] << 8 ) | data[3];
			magn_data_integrate.magn_y = (data[4] << 8 ) | data[5];
		} else {
			printf("read magn XYZ failed\n");
		}
		if (i2c_readreg(LSM303_M_ADDR, LSM303_M_TEMP_OUT_H,
		    data, 2)) {
			accel_temp =
			    (((int)data[0]) << 4) | (data[1] >> 4);
		} else {
			printf("read accel temp failed\n");
		}
	}
#endif
}

void
get_data_w(void)
{
#ifdef CAPTEUR_FAKE
	gyro_data_integrate.gyro_x = 0L;
	gyro_data_integrate.gyro_y = 0L;
	gyro_data_integrate.gyro_z = 0L;
	gyro_int_count  = 1;
#else
	int data_xyz[3];

	if (GYRO_DRDY) {
		gyro_int_count++;
		gyro_loops++;
		if (! i2c_readreg(L3DG20_ADDRESS, L3DG20_OUT_X_L,
		    (char *)data_xyz, 6)) {
			printf("read gyro XYZ failed\n");
		}
		gyro_data_integrate.gyro_y += data_xyz[0];
		gyro_data_integrate.gyro_x += data_xyz[1];
		gyro_data_integrate.gyro_z += data_xyz[2];
		if (gyro_int_count == 0) {
			/* if we did read too much values */
			gyro_data_integrate.gyro_x = 0;
			gyro_data_integrate.gyro_y = 0;
			gyro_data_integrate.gyro_z = 0;
		}
	}
#endif
}

static void
calibrate(void)
{
	static int previous_heading;
	static long computed_heading;
	static long start_heading;
	static int delta;
	static float gyro_adj;
	unsigned char i, j;
	long h;
	signed char rot;
#define UNDEFINED_VALUE 32767

	previous_heading = heading;
	/* step 1: wait for crossing north heading */
	printf("cal: wait for start\n");
	while (1) {
		get_data_w();
		if (softintrs & INT_10HZ) {
			softintrs &= ~INT_10HZ;
			counter_1hz--;
			get_data_am();
			if (accel_int_count > 0 && gyro_int_count > 0) {
				CLRWDT;
				compute_values();
				if (previous_heading > 30000 &&
				    heading < -30000) {
					rot = -1;
					break;
				} else if (previous_heading < -30000 &&
				    heading > 30000) {
					rot = 1;
					break;
				}
				previous_heading = heading;
			}

			if (counter_1hz == 0) {
				counter_1hz = 10;
					printf("gyro %10ld pitch %6d roll %6d heading %6d %d\n", 
					    (long)(rate_of_turn * 10000000.0F),
					    pitch,
					    roll,
					    heading,
					    (int)gyro_loops);
			}
			gyro_loops = 0;
		}
	}

	printf("cal: calibrate compass\n");
	previous_heading = heading;
	if (rot == -1) {
		start_heading = computed_heading =  -31415;
	} else {
		start_heading = computed_heading =  31416;
	}
	counter_1hz = 10;
	for (i = 0; i < COMPASS_CORRECTION_TABLE_SIZE; i++)
		compass_correction_table[i] = UNDEFINED_VALUE;
	i = (((heading + 31415) & 0xfe00) >> 9);
	compass_correction_table[i] = computed_heading / 2;
	printf("%3d %6d %6d\n", i, heading, computed_heading);
	counter_1hz = 10;
	while (1) {
		get_data_w();
		if (softintrs & INT_10HZ) {
			softintrs &= ~INT_10HZ;
			counter_1hz--;
			get_data_am();
			if (accel_int_count > 0 && gyro_int_count > 0) {
				CLRWDT;
				compute_values();
				/* rate_of_turn * -10000.0 * deltaT */
				computed_heading += (int)(rate_of_turn * -1000.0F);
				if (rot == -1 && computed_heading > 0 &&
				    previous_heading > 30000 &&
				    heading < -30000) {
					break;
				}
				if (rot == 1 && computed_heading < 0 &&
				    previous_heading < -30000 &&
				    heading > 30000) {
					break;
				}
				j = (((heading + 31415) & 0xfe00) >> 9);
				if (rot == -1 &&
				    j >= i + 1 && j <= i + 10) {
					compass_correction_table[j] = computed_heading / 2;
					i = j;
				} else if (rot == 1 &&
				    j <= i - 1 && j >= i - 10) {
					compass_correction_table[j] = computed_heading / 2;
					i = j;
				}
				previous_heading = heading;
			}

			if (counter_1hz == 0) {
				counter_1hz = 10;
				printf("gyro %10ld, %3d %6d %6ld\n",
				    (long)(rate_of_turn * 10000000.0F),
				    i, heading, computed_heading);
			}
			gyro_loops = 0;
		}
	}
	if (computed_heading > 0) {
		computed_heading += 31415;
	} else {
		computed_heading = 31415 - computed_heading;
	}
	/* compute gyro error over a full turn */
	gyro_adj = 62831.853 / (float)computed_heading;
	printf("gyro_adj %d\n", (int)(gyro_adj * 1000.0F));
	if (compass_correction_table[COMPASS_CORRECTION_TABLE_SIZE - 1] != UNDEFINED_VALUE) {
		compass_correction_table[0] =
		    compass_correction_table[COMPASS_CORRECTION_TABLE_SIZE - 1];
	} else if (compass_correction_table[0] != UNDEFINED_VALUE) {
		compass_correction_table[COMPASS_CORRECTION_TABLE_SIZE - 1] =
		    compass_correction_table[0];
	}
	j = 0;
	if (compass_correction_table[0] != UNDEFINED_VALUE) {
		i = 0;
	} else {
		for (i = COMPASS_CORRECTION_TABLE_SIZE - 2; i != 0; i--) {
			if (compass_correction_table[i] != UNDEFINED_VALUE)
				break;
		}
		if (i == 0)
			return;
	}

	computed_heading =
	    ((long)compass_correction_table[i] * 2L) - start_heading;
	computed_heading = (long)((float)computed_heading * gyro_adj);
	compass_correction_table[i] = computed_heading + start_heading;

	while (j < COMPASS_CORRECTION_TABLE_SIZE) {
		j++;
		for (; j < COMPASS_CORRECTION_TABLE_SIZE; j++) {
			if (compass_correction_table[j] != UNDEFINED_VALUE)
				break;
		}
		if (j == COMPASS_CORRECTION_TABLE_SIZE)
			break;
		computed_heading =
		    ((long)compass_correction_table[j] * 2L) - start_heading;
		computed_heading = (long)((float)computed_heading * gyro_adj);
		compass_correction_table[j] = computed_heading + start_heading;
		if (i > j) {
			delta = (compass_correction_table[j] - compass_correction_table[i]) / (COMPASS_CORRECTION_TABLE_SIZE - 1 + j - i);
		} else {
			delta = (compass_correction_table[j] - compass_correction_table[i]) / (j - i);
		}
		previous_heading = compass_correction_table[i];
		i++;
		for (; i != j; i++) {
			if (i == COMPASS_CORRECTION_TABLE_SIZE - 2)
				i = 0;
			compass_correction_table[i] = previous_heading + delta;
			previous_heading = compass_correction_table[i];
		}
	}

	/* store correction not absolute value in table */
	for (i = 0; i < COMPASS_CORRECTION_TABLE_SIZE; i++) {
		heading = 512 * i - 31415;
		if (i == COMPASS_CORRECTION_TABLE_SIZE - 1)
			heading = -31415;
		compass_correction_table[i]  -= heading;
	}
	/* smoothing, from https://en.wikipedia.org/wiki/Savitzky%E2%80%93Golay_filter */
	for (i = 3; i < COMPASS_CORRECTION_TABLE_SIZE - 1 + 3; i++) {
		j = i % (COMPASS_CORRECTION_TABLE_SIZE - 1);
		h = -2L * (long)compass_correction_table[(i-3) % (COMPASS_CORRECTION_TABLE_SIZE - 1)] +
		    3L * (long)compass_correction_table[(i-2) % (COMPASS_CORRECTION_TABLE_SIZE - 1)] +
		    6L * (long)compass_correction_table[(i-1) % (COMPASS_CORRECTION_TABLE_SIZE - 1)] +
		    7L * (long)compass_correction_table[j] +
		    6L * (long)compass_correction_table[(i+1) % (COMPASS_CORRECTION_TABLE_SIZE - 1)] +
		    3L * (long)compass_correction_table[(i+2) % (COMPASS_CORRECTION_TABLE_SIZE - 1)] +
		    -2L * (long)compass_correction_table[(i+3) % (COMPASS_CORRECTION_TABLE_SIZE - 1)];
		write_eeprom_short(COMPASS_CORRECTION_TABLE_OFFSET + j * 2,
		    (h / 21));
	}
	get_eeprom_datas();
	j = COMPASS_CORRECTION_TABLE_SIZE-1;
	write_eeprom_short(COMPASS_CORRECTION_TABLE_OFFSET + j * 2,
	    compass_correction_table[0]);
	compass_correction_table[j] = compass_correction_table[0];
	
	printf("final correction table:\n");
	for (i = 0; i < COMPASS_CORRECTION_TABLE_SIZE; i++) {
		heading = 512 * i - 31415;
		if (i == COMPASS_CORRECTION_TABLE_SIZE - 1)
			heading = -31415;
		printf("%3d %6d %6d\n", i, heading,
		    (int)compass_correction_table[i]);
	}
	return;
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
	USART_INTR;
	if (PIR5 != 0) {
		nmea2000_intr();
	}
	if (PIR5bits.RXBnIF)
		PIE5bits.RXBnIE = 0; /* main loop will check */

	if (PIR5 != 0)
		PIE5 &= _PIE5_TXBnIE;
}
