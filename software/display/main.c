/* $Id: main.c,v 1.36 2019/03/12 19:24:19 bouyer Exp $ */
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
#include <string.h> 
#include <stdlib.h> 
#include <math.h> 
#include <autopilot_serial.h> 
#include <autopilot_spi.h> 
#include <nmea2000.h> 
#include <nmea2000_pgn.h> 
#include <eeprom.h> 
#include <raddeg.h> 
#include <font5x8.h> 
#include <font10x16.h> 
#define USE_2024
#ifdef USE_2024
#include <font20x24.h> 
#endif
#include <menu.h> 

extern unsigned char stack; 
extern unsigned char stack_end;

#pragma stack 0x100 256

void _reset (void) __naked __interrupt 0;
void _startup (void) __naked;

unsigned long nmea2000_user_id; 

unsigned short timer0_read(void);
void delay(unsigned short);
#pragma callee_saves timer0_read

/* hardware defines */
#define BACK_PORT	LATCbits.LATC2

#define SPKR_PORT	LATAbits.LATA3

#define ROTARY_A	PORTBbits.RB3
#define ROTARY_B	PORTBbits.RB2

#define SW1		(!PORTBbits.RB5)
#define SW2		(!PORTBbits.RB4)
#define SW3		(!PORTBbits.RB0)
#define SW4		(!PORTBbits.RB1)

/* beep freq: 19.53125Khz / x */
#define SPKR_440	44 /* 440Hz */
#define SPKR_1000	19 /* 1Khz */
static unsigned char beep_duration;
#define SPKR_440_D100	88 /* 0.1s */
#define SPKR_1000_D100	200 /* 0.1s */

/* LCD-relayted defines (hardware and protocol) */
#define A0 LATCbits.LATC1
#define A0_CTRL 0
#define A0_DISP 1
#define PAGE_ADDR(c) ((unsigned char)0xb0 | c)
#define COL_ADDR_H(c) ((unsigned char)0x10 | (c >> 4))
#define COL_ADDR_L(c) ((unsigned char)0x00 | (c & 0x0f))

/* various global variables */
unsigned char lcd_ready;

static struct nmea2000_msg msg;
static unsigned char nmea2000_data[NMEA2000_DATA_LENGTH];

union {
	struct nmea2000_navdata_data s;
	unsigned char d[sizeof(struct nmea2000_navdata_data)];
} nmea2000_navdata_data;
unsigned char nmea2000_navdata_id;
unsigned char nmea2000_navdata_len;

static unsigned int received_heading;
#define HEADING_INVALID 32767
static int received_pitch;
static int received_roll;
static long received_rot_rate;
static int received_compass_offset;

static unsigned int received_cog; /* also use HEADING_INVALID */
static unsigned char received_speed;
static unsigned char received_speedd;
static unsigned char cogsog_changed;
unsigned short last_nmea2000_cogsog;

static long received_xte; /* in m * 100 */
#define XTE_INVALID 0
static unsigned char xte_changed;
unsigned short last_nmea2000_xte;

static unsigned int received_towp_cog; /* also use HEADING_INVALID */
static unsigned long received_towp_dist; /* in m  * 100 */
static          char received_towp_speed;
static unsigned char received_towp_speedd;
static unsigned char towp_changed;
unsigned short last_nmea2000_navdata;

unsigned char nmea2000_capteur_address;
unsigned short last_capteur_data;
unsigned char nmea2000_command_address;
unsigned short last_command_data;

unsigned char received_hours;
#define HOUR_INVALID 0xff
int received_minutes;
unsigned char datetime_changed;
unsigned short last_datetime_data;

static union command_errors cmderr_list;
int command_received_heading;
unsigned char received_auto_mode;
char received_rudder;
char received_param_slot;
int command_target_heading;
struct private_command_factors command_factors;

static volatile unsigned short counter_1hz;
static volatile unsigned int softintrs;
#define INT_1HZ		0x0001
#define INT_SW_R	0x0002
#define INT_SW_1P	0x0004 /* key press */
#define INT_SW_2P	0x0008
#define INT_SW_3P	0x0010
#define INT_SW_4P	0x0020
#define INT_SW_1R	0x0040 /* key release */
#define INT_SW_2R	0x0080
#define INT_SW_3R	0x0100
#define INT_SW_4R	0x0200

union inputs_status {
	struct {
		unsigned rotary_a	: 1;
		unsigned rotary_b	: 1;
		unsigned pending_a	: 1;
		unsigned pending_b	: 1;
		unsigned sw1		: 1;
		unsigned sw2		: 1;
		unsigned sw3		: 1;
		unsigned sw4		: 1;
	} s;
	unsigned char v;
};

static union inputs_status inputs_status;
static volatile char rotary_pos;
static char saved_rotary_pos;
static unsigned char rotary_a;
static unsigned char rotary_b;
static unsigned char sw1;
static unsigned char sw2;
static unsigned char sw3;
static unsigned char sw4;
union switch_events {
	struct {
		unsigned sw1	: 1; /* short press */
		unsigned sw2	: 1;
		unsigned sw3	: 1;
		unsigned sw4	: 1;
		unsigned sw1l	: 1; /* long press */
		unsigned sw2l	: 1;
		unsigned sw3l	: 1;
		unsigned sw4l	: 1;
	} s;
	char v;
};
static union switch_events switch_events;

static void get_rotary_pos(void) 
{
		INTCONbits.GIE_GIEH=0;
		saved_rotary_pos = rotary_pos;
		rotary_pos = 0;
		softintrs &= ~INT_SW_R;
		INTCONbits.GIE_GIEH=1;
}
#define CLRWDT __asm__("clrwdt")

unsigned char backlight_pr;
#define BACKLIGHT_ADDR 0x00 /* address of backlight_pr save in eeprom */
static unsigned char lcd_col;
static unsigned char lcd_line;
#define DISPLAY_W	132
#define DISPLAY_H	4
char lcd_displaybuf[DISPLAY_W / 6];
#define DISPLAY_FONTSMALL_W	6
#define DISPLAY_FONTMEDIUM_W	10
#define DISPLAY_FONTLARGE_W	20
static void displaybuf_small(void);
static void displaybuf_medium(void);
static void displaybuf_large(void);
static void resetdisplay(void);
static void cleardisplay(void);
static void clearline(unsigned char) __wparam;
enum display_page display_page;
enum display_page previous_display_page;
enum display_page next_display_page;
unsigned short last_display_update;

static char page_displaymenu(const struct page_menu_entry *, char, char);

static void page_mainmenu(char) __wparam;
static void page_confmenu(char) __wparam;
static void page_maindata(char) __wparam;
static void page_light(char) __wparam;
static void page_charlist(char) __wparam;
static void page_rawdata_capt(char) __wparam;
static void page_rawdata_cmd(char) __wparam;
static void page_err(char) __wparam;
static void page_cmderr(char) __wparam;
static union displar_errs {
    struct {
	char nocapt	: 1;
	char nocmd	: 1;
    } bits;
    unsigned char byte;
} err_list, err_ack;

static void sw_beep(void);
static void check_light(void);
static unsigned char check_light_long;

static char next_page_from_auto_mode(void)
{
	if (received_auto_mode == 0)
		return MAIN_DATA;
	else if (received_auto_mode == AUTO_STANDBY)
		return ACT_PAGE;
	else
		return ENGAGED_DATA;
}

static void
send_command_engage(unsigned char mode, char slot)
{
	__data struct private_command_engage * d = (void *)&nmea2000_data[0];
	d->auto_mode = mode;
	d->heading = command_target_heading;
	d->params_slot = slot;
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
send_command_request_factors(char slot) __wparam
{
	__data struct private_command_factors_request *d = (void *)&nmea2000_data[0];
	d->slot = slot;
	msg.id.id = 0;
	msg.id.iso_pg =  (PRIVATE_COMMAND_FACTORS_REQUEST >> 8) & 0xff;
	msg.id.daddr = nmea2000_command_address;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct private_command_factors_request);
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMMAND_FACTORS_REQUEST failed\n");
}

static void
send_command_factors(void)
{
	__data struct private_command_factors *d = (void *)&nmea2000_data[0];
	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_COMMAND_FACTORS >> 8) & 0xff;
	msg.id.daddr = nmea2000_command_address;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.dlc = sizeof(struct private_command_factors);
	msg.data = &nmea2000_data[0];

	d->slot = command_factors.slot;
	d->factors[0] = command_factors.factors[0];
	d->factors[1] = command_factors.factors[1];
	d->factors[2] = command_factors.factors[2];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMMAND_FACTORS failed\n");
}

static void
send_command_errack(char ack) __wparam
{
	__data struct private_command_errack *d = (void *)&nmea2000_data[0];
	d->ack_errors.byte = ack;
	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_COMMAND_ERRACK >> 8) & 0xff;
	msg.id.daddr = nmea2000_command_address;
	msg.dlc = sizeof(struct private_command_errack);
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_COMMAND_ERRACK failed\n");
}

static void
send_command_mob(void) __wparam
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
send_command_light(char cmd, char val)
{
	__data struct private_remote_control *d = (void *)&nmea2000_data[0];
	d->control_type = CONTROL_LIGHT;
	d->control_subtype = cmd;
	if (cmd == CONTROL_LIGHT_VAL) {
		d->control_data[0] = val;
		msg.dlc = CONTROL_LIGHT_VAL_SIZE;
	} else {
		msg.dlc = CONTROL_LIGHT_SIZE;
	}
	msg.id.id = 0;
	msg.id.iso_pg = (PRIVATE_REMOTE_CONTROL >> 8) & 0xff;
	msg.id.daddr = NMEA2000_ADDR_GLOBAL;
	msg.id.priority = NMEA2000_PRIORITY_INFO;
	msg.data = &nmea2000_data[0];
	if (! nmea2000_send_single_frame(&msg))
		printf("send PRIVATE_REMOTE_CONTROL failed\n");
}

void
user_handle_iso_request(unsigned long pgn)
{
	(pgn);
	/* XXX */
}

void
user_receive()
{
	unsigned long pgn;

	pgn = ((unsigned long)rid.page << 16) | ((unsigned long)rid.iso_pg << 8);
	if (rid.iso_pg > 239)
		pgn |= rid.daddr;

	switch(pgn) {
	case NMEA2000_ATTITUDE:
	{
		struct nmea2000_attitude_data *d = (void *)rdata;
		received_heading = d->yaw;
		received_pitch = d->pitch;
		received_roll = d->roll;
		nmea2000_capteur_address = rid.saddr;
		last_capteur_data = timer0_read();
		err_list.bits.nocapt = 0;
		err_ack.bits.nocapt = 0;
		break;
	}
	case NMEA2000_RATEOFTURN:
	{
		struct nmea2000_rateofturn_data *d = (void *)rdata;
		nmea2000_capteur_address = rid.saddr;
		received_rot_rate = d->rate;
		break;
	}
	case NMEA2000_COGSOG:
	{
		long speed;
		struct nmea2000_cogsog_data *d = (void *)rdata;
		received_cog = d->cog;
		speed = d->sog;
		speed = speed * 360UL / 1852UL;
		received_speed = speed / 10;
		received_speedd = speed % 10;
		cogsog_changed = 1;
		last_nmea2000_cogsog = timer0_read();
		break;
	}

	case NMEA2000_XTE:
	{
		struct nmea2000_xte_data *d = (void *)rdata;
		received_xte = d->xte;
		xte_changed = 1;
		last_nmea2000_xte = timer0_read();
		break;
	}
	case NMEA2000_NAVDATA:
	{
		unsigned char idx = (rdata[0] & FASTPACKET_IDX_MASK);
		unsigned char id =  (rdata[0] & FASTPACKET_ID_MASK);
		char i, j;

		if (idx == 0) {
			/* new head packet */
			nmea2000_navdata_id = id;
			nmea2000_navdata_len = rdata[1];
			for (i = 0;
			     i < 6 && nmea2000_navdata_len > 0; i++) {
				nmea2000_navdata_data.d[i] = rdata[i+2];
				nmea2000_navdata_len--;
			}
		} else if (
		    id == nmea2000_navdata_id) {
			j = 1;
			/* i = 6 + (idx - 1) * 7 : i = idx * 7 - 1 */
			for (i = idx * 7 - 1, j = 1; 
			    i < sizeof(nmea2000_navdata_data) && j < 8 && nmea2000_navdata_len > 0;
			    i++, j++) {
				nmea2000_navdata_data.d[i] = rdata[j];
				nmea2000_navdata_len--;
			}
		}
		if (nmea2000_navdata_len == 0) {
			long speed;
			/* packet complete */
			received_towp_cog =
			    nmea2000_navdata_data.s.bearing_p2d;
			received_towp_dist = 
			    nmea2000_navdata_data.s.dist_to_wp;
			speed = nmea2000_navdata_data.s.wp_closing_speed;
			speed = speed * 360UL / 1852UL;
			received_towp_speed = speed / 10;
			received_towp_speedd = speed % 10;
			towp_changed = 1;
			last_nmea2000_navdata = timer0_read();
		}
		break;
	}
	case NMEA2000_DATETIME:
	{
		struct nmea2000_datetime_data *d = (void *)rdata;
		received_minutes = d->time / 600000UL;
		received_minutes += d->local_offset;
		received_hours = received_minutes / 60;
		received_minutes = received_minutes % 60;
		datetime_changed = 1;
		last_datetime_data = timer0_read();
		break;
	}
	case PRIVATE_COMPASS_OFFSET:
	{
		struct private_compass_offset_data *compass_offset_data = (void *)rdata;
		received_compass_offset =
		    compass_offset_data->offset;
		break;
	}
	case PRIVATE_COMMAND_STATUS:
	{
		struct private_command_status *command_status = (void *)rdata;
		last_command_data = timer0_read();
		nmea2000_command_address = rid.saddr;
		command_received_heading = command_status->heading;
		received_auto_mode = command_status->auto_mode;
		cmderr_list.byte = command_status->command_errors.byte;
		received_rudder = command_status->rudder;
		received_param_slot = command_status->params_slot;
		err_list.bits.nocmd = 0;
		err_ack.bits.nocmd = 0;
		break;
	}
	case PRIVATE_COMMAND_FACTORS:
	{
		struct private_command_factors *d = (void *)rdata;
		command_factors.slot = d->slot;
		command_factors.factors[0] = d->factors[0];
		command_factors.factors[1] = d->factors[1];
		command_factors.factors[2] = d->factors[2];
		break;
	}
	}
}

void
lcd_putchar (char c) __wparam
{
	if (c == '\n') {
		lcd_col = 0;
		lcd_line++;
		if (lcd_line > 3)
			lcd_line  = 0;
	}
	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(lcd_line));
	spi_write(COL_ADDR_H(lcd_col));
	spi_write(COL_ADDR_L(lcd_col));
	A0 = A0_DISP;
	if (c == '\n') {
		unsigned char i;
		for (i = 0; i < 132; i++) {
			spi_write(0x00);
		}
	} else {
		unsigned char i;
		const char *font = get_font5x8(c);
		spi_write(0);
		for (i = 0; i < 5; i++) {
			spi_write(font[i]);
		}
		lcd_col +=6;
		if (lcd_col > 125) {
			lcd_col = 0;
		}
	}
}

PUTCHAR(c)
{
	if (PORTBbits.RB7) {
		usart_putchar(c);
	}
	if (lcd_ready) {
		lcd_putchar(c);
	}
}

static void
resetdisplay(void)
{
	int i;
	lcd_ready = 0;

	LATCbits.LATC0 = 0; /* reset display */
	for (i = 0; i < 1000; i++)
		; /* wait */
	LATCbits.LATC0 = 1; /* release display reset */
	A0 = A0_CTRL;
	spi_write(0xe2); /* reset */
	spi_write(0x40); /* display start line 0 */

	spi_write(0xA1); /* ADC reverse */

	spi_write(0xC0); /* Normal com0-com3 */

	spi_write(0xA6); /* Display normal */

	spi_write(0xA2); /* Bias 1/9 */

	spi_write(0x2F); /* Booster, Regulator and Follower on */


	spi_write(0xF8); /* Set internal Booster to 3x/4x */
	spi_write(0x00);


	spi_write(0x23); /* contrast */
	spi_write(0x81);
	spi_write(0x14);

	spi_write(0xac); /* static indicator */
	spi_write(0x00);

	cleardisplay();
	A0 = A0_CTRL;
	spi_write(0xaf); /* display on */
	lcd_ready = 1;
}

static void
cleardisplay()
{
	unsigned char page;
	/* initialize display ram */
	for (page = 0; page < DISPLAY_H; page++) {
		clearline(page);
	}
}
static void
clearline(unsigned char line) __wparam
{
	unsigned char col;
	
	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(line));
	spi_write(COL_ADDR_H(0));
	spi_write(COL_ADDR_L(0));
	A0 = A0_DISP;
	for (col = 0; col < DISPLAY_W; col++) {
		spi_write(0x00);
	}
}

static void
print_light()
{
	sprintf(lcd_displaybuf, "LIGHT");
	lcd_line = 0;
	lcd_col = DISPLAY_W - DISPLAY_FONTSMALL_W * 5 - 1;
	displaybuf_small();
}

static void
print_gostop()
{
	switch (received_auto_mode) {
	case AUTO_OFF:
	case AUTO_STANDBY:
		sprintf(lcd_displaybuf, "GO");
		break;
	default:
		sprintf(lcd_displaybuf, "STOP");
		break;
	}
	lcd_line = 0;
	lcd_col = 0;
	displaybuf_small();
}

static void
print_menupage()
{
	sprintf(lcd_displaybuf, "MENU");
	lcd_line = DISPLAY_H - 1;
	lcd_col = 0;
	displaybuf_small();
	sprintf(lcd_displaybuf, "PAGE");
	lcd_line = DISPLAY_H - 1;
	lcd_col = DISPLAY_W - DISPLAY_FONTSMALL_W * 5 - 1;
	displaybuf_small();
}

static void
displaybuf_small()
{
	const char *font;
	char *cp;
	unsigned char i;

	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(lcd_line));
	spi_write(COL_ADDR_H(lcd_col));
	spi_write(COL_ADDR_L(lcd_col));
	A0 = A0_DISP;
#ifdef DISPLAY_FAKE
	printf("%s\n", lcd_displaybuf);
#endif
	for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
		spi_write(0);
		font = get_font5x8(*cp);
		for (i = 0; i < 5; i++) {
			spi_write(font[i]);
		}
	}
}

static void
displaybuf_medium()
{
	const char *font;
	char *cp;
	unsigned char i;

	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(lcd_line));
	spi_write(COL_ADDR_H(lcd_col));
	spi_write(COL_ADDR_L(lcd_col));
	A0 = A0_DISP;
	for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
		font = get_font10x16(*cp);
		for (i = 0; i < 19; i+=2) {
			spi_write(font[i]);
		}
	}
	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(lcd_line+1));
	spi_write(COL_ADDR_H(lcd_col));
	spi_write(COL_ADDR_L(lcd_col));
#ifdef DISPLAY_FAKE
	printf("%s\n", lcd_displaybuf);
#endif
	A0 = A0_DISP;
	for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
		font = get_font10x16(*cp);
		for (i = 1; i < 20; i+=2) {
			spi_write(font[i]);
		}
	}
}

/*
 * use same font as displaybuf_medium() but with 2 pixels per dot
 * line is unused here, is uses the 4 lines
 */
static void
displaybuf_large()
{
#ifdef USE_2024
	const char * font;
	char *cp;
	unsigned char i;
	unsigned char l;

	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(lcd_line));
	spi_write(COL_ADDR_H(lcd_col));
	spi_write(COL_ADDR_L(lcd_col));
	A0 = A0_DISP;
	for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
		font = get_font20x24(*cp);
		for (i = 0; i < 60; i+=3) {
			spi_write((font[i] << 4) & 0xf0);
		}
	}
#ifdef DISPLAY_FAKE
	printf("%s\n", lcd_displaybuf);
#endif
	for (l = 1; l < 3; l++) {
		A0 = A0_CTRL;
		spi_write(PAGE_ADDR(lcd_line+l));
		spi_write(COL_ADDR_H(lcd_col));
		spi_write(COL_ADDR_L(lcd_col));
		A0 = A0_DISP;
		for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
			font = get_font20x24(*cp);
			for (i = l; i < 60; i+=3) {
				spi_write(((font[i - 1] >> 4) & 0x0f) |
					  ((font[i] << 4) & 0xf0));
			}
		}
	}
	A0 = A0_CTRL;
	spi_write(PAGE_ADDR(lcd_line+3));
	spi_write(COL_ADDR_H(lcd_col));
	spi_write(COL_ADDR_L(lcd_col));
	A0 = A0_DISP;
	for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
		font = get_font20x24(*cp);
		for (i = 2; i < 60; i+=3) {
			spi_write((font[i] >> 4) & 0x0f);
		}
	}
#else /* USE_2024 */
	const char * font;
	const char * myfontp;
	unsigned char mask;
	char i, j, f;
	char *cp;
	unsigned char line;

#ifdef DISPLAY_FAKE
	printf("%s\n", lcd_displaybuf);
#endif
	for (line = 0; line < 4; line++) {
		A0 = A0_CTRL;
		spi_write(PAGE_ADDR(line));
		spi_write(COL_ADDR_H(lcd_col));
		spi_write(COL_ADDR_L(lcd_col));
		A0 = A0_DISP;
		for (cp = lcd_displaybuf; *cp != '\0'; cp++) {
			font = get_font10x16(*cp);
			for (i = 0; i < 10; i++) {
				myfontp = &font[(i << 1) + (line >> 1)];
				if (line & 1)
					mask = 0x80;
				else
					mask = 0x08;
				f = 0;
				for (j = 0; j < 4; j++) {
					f = f << 2;
					if ((*myfontp) & mask)
						f |= 3;
					mask = mask >> 1;
				}
				spi_write(f);
				spi_write(f);
			}
		}
	}
#endif
}

static char
page_displaymenu(const struct page_menu_entry *menu, char size, char pagechange)
{
	char start, end;
	enum display_page ret;
	if (softintrs & INT_SW_R) {
		softintrs &= ~INT_SW_R;
		if (rotary_pos < 0)
			rotary_pos = 0;
		if (rotary_pos > size - 1)
			rotary_pos = size - 1;
		sw_beep();
		cleardisplay();
		pagechange = 1;
	}
	start = rotary_pos - 1;
	end = rotary_pos + 1;

	if (start < 0) {
		start = 0;
		end += 1;
	}
	if (end >= size) {
		end  = size - 1;
		start = end - 2;
		if (start < 0)
			start = 0;
	}

	lcd_line = 0;
	if (pagechange) {
		for(; start <= end; start++) {
			lcd_col = DISPLAY_FONTSMALL_W;
			if (start == rotary_pos) {
#ifdef DISPLAY_FAKE
				sprintf(lcd_displaybuf, "%c %s", '>', menu[start].string);
#else
				sprintf(lcd_displaybuf, "%c %s", 28, menu[start].string);
#endif
			} else {
				sprintf(lcd_displaybuf, "  %s", menu[start].string);
			}
			displaybuf_small();
			lcd_line++;
		}
		print_light();
		sprintf(lcd_displaybuf, "SELECT");
		lcd_line = DISPLAY_H - 1;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "CANCEL");
		lcd_line = DISPLAY_H - 1;
		lcd_col = DISPLAY_W - 6 * DISPLAY_FONTSMALL_W - 1;
		displaybuf_small();
	}
	ret = NOP;
	if (switch_events.s.sw4) {
		ret = menu[rotary_pos].page;
	}
	if (switch_events.s.sw3) {
		ret = BACK;
	}
	check_light();
	if (next_display_page == LIGHT)
		return LIGHT;
	return ret;
}

static void
page_mainmenu(char pagechange) __wparam
{
	static enum display_page menu_previous_display_page;
	char new_page;

	if (pagechange) {
		rotary_pos = 0;
		menu_previous_display_page = previous_display_page;
	}
	new_page = page_displaymenu(page_menu, 9, pagechange);
	switch(new_page) {
	case BACK:
		next_display_page = menu_previous_display_page;
		break;
	case NOP:
		next_display_page = MAIN_MENU;
		break;
	default:
		next_display_page = new_page;
		break;
	}
}

static void
page_confmenu(char pagechange) __wparam
{
	static enum display_page menu_previous_display_page;
	char new_page;

	if (pagechange) {
		rotary_pos = 0;
		menu_previous_display_page = previous_display_page;
	}
	new_page = page_displaymenu(conf_menu, 4, pagechange);
	switch(new_page) {
	case BACK:
		next_display_page = menu_previous_display_page;
		break;
	case NOP:
		next_display_page = CONF_MENU;
		break;
	default:
		next_display_page = new_page;
		break;
	}
}

static void
page_rawmenu(char pagechange) __wparam
{
	static enum display_page menu_previous_display_page;
	char new_page;

	if (pagechange) {
		rotary_pos = 0;
		menu_previous_display_page = previous_display_page;
	}
	new_page = page_displaymenu(raw_menu, 2, pagechange);
	switch(new_page) {
	case BACK:
		next_display_page = menu_previous_display_page;
		break;
	case NOP:
		next_display_page = RAW_MENU;
		break;
	default:
		next_display_page = new_page;
		break;
	}
}

static void
page_maindata(char pagechange) __wparam
{
	static unsigned int heading;
	static unsigned char previous_auto_mode;

	if (pagechange) {
		print_gostop();
		print_light();
		print_menupage();
		previous_auto_mode = received_auto_mode;
		heading = received_heading;
	}
	if (pagechange ||
	    (heading != received_heading &&
	     timer0_read() - last_display_update > 5000)) {
		last_display_update = timer0_read();
		heading = received_heading;
		if (heading == HEADING_INVALID)
			sprintf(lcd_displaybuf, "---");
		else
			sprintf(lcd_displaybuf, "%3d", rad2deg(heading));
		lcd_line = 0;
		lcd_col = DISPLAY_W / 2 - DISPLAY_FONTLARGE_W * 3 / 2 - 1;
		displaybuf_large();
	}
	if (previous_auto_mode != received_auto_mode) {
		switch (received_auto_mode) {
		case AUTO_OFF:
			break;
		case AUTO_STANDBY:
			next_display_page = ACT_PAGE;
			break;
		default:
			next_display_page = ENGAGED_DATA;
			break;
		}
		previous_auto_mode = received_auto_mode;
	}

	check_light();
	if (switch_events.s.sw3) {
		if (received_auto_mode == AUTO_HEAD) {
			next_display_page = ENGAGED_DATA;
		} else {
			next_display_page = COGSOG_DATA;
		}
	}
	if (switch_events.s.sw4) {
		next_display_page = MAIN_MENU;
	}
	if (switch_events.s.sw2) {
		next_display_page = ACT_PAGE;
		command_target_heading = deg2rad(rad2deg(received_heading));
	}
}

static void
page_engageddata(char pagechange) __wparam
{
	static int heading;
	static int previous_target_heading;

	if (pagechange) {
		sprintf(lcd_displaybuf, "STOP");
		lcd_line = 0;
		lcd_col = 0;
		displaybuf_small();
		print_light();
		sprintf(lcd_displaybuf, "MENU");
		lcd_line = DISPLAY_H - 1;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "PA/PR");
		lcd_line = 3;
		lcd_col = DISPLAY_W - DISPLAY_FONTSMALL_W * 5 - 1;
		displaybuf_small();
		heading = previous_target_heading = HEADING_INVALID;
		rotary_pos = 0;
	}
	if (received_auto_mode == AUTO_OFF) {
		send_command_engage(AUTO_OFF, received_param_slot);
		sw_beep();
		next_display_page = MAIN_DATA;
		return;
	}
	if (received_auto_mode == AUTO_HEAD) {
		command_target_heading = command_received_heading;
		if (softintrs & INT_SW_R) {
			int deg_command_target_heading;
			get_rotary_pos();
			deg_command_target_heading =
			    rad2deg(command_target_heading);
			deg_command_target_heading += saved_rotary_pos;
			if (deg_command_target_heading < 0)
				deg_command_target_heading += 360;
			else if (deg_command_target_heading > 359)
				deg_command_target_heading -= 360;
			sw_beep();
			command_target_heading =
			    deg2rad(deg_command_target_heading);
			send_command_engage(AUTO_HEAD, received_param_slot);
			command_received_heading = command_target_heading;
		}

		if (pagechange ||
		    ((heading != received_heading ||
		      previous_target_heading != command_target_heading) &&
		      timer0_read() - last_display_update > 5000)) {
			int heading_error;
			last_display_update = timer0_read();
			heading = received_heading;
			previous_target_heading = command_target_heading;
			sprintf(lcd_displaybuf, "%3d",
			    rad2deg(command_target_heading));
			lcd_line = 0;
			lcd_col =
			    DISPLAY_W / 2 - DISPLAY_FONTLARGE_W * 3 / 2 - 1;
			displaybuf_large();
			heading_error = rad2deg(heading) -
			    rad2deg(command_target_heading);
			if (heading_error > 180)
				heading_error = heading_error - 360;
			else if (heading_error < -180)
				heading_error = 360 + heading_error;
			sprintf(lcd_displaybuf, "%4d", heading_error);
			lcd_line = 1;
			lcd_col =
			    DISPLAY_W / 2 + DISPLAY_FONTLARGE_W * 3 / 2 - 2;
			displaybuf_medium();
			sprintf(lcd_displaybuf, "P%02d", received_param_slot);
			lcd_line = 1;
			lcd_col = 0;
			displaybuf_medium();
		}
	}
	check_light();
	if (switch_events.s.sw4) {
		next_display_page = MAIN_MENU;
	}
	if (switch_events.s.sw3l) {
		next_display_page = CONF_CMD_CONFIG;
	}
	if (switch_events.s.sw3) {
		next_display_page = COGSOG_DATA;
	}
	if (switch_events.s.sw2) {
		send_command_engage(AUTO_STANDBY, received_param_slot);
		next_display_page = ACT_PAGE;
	}
}

static void
page_act(char pagechange) __wparam
{
	static unsigned short last_xmit;
	if (pagechange) {
		send_command_engage(AUTO_STANDBY, received_param_slot);
		sprintf(lcd_displaybuf, "GO");
		lcd_line = 0;
		lcd_col = 0;
		displaybuf_small();
		print_light();
		sprintf(lcd_displaybuf, "CANCEL");
		lcd_line = DISPLAY_H - 1;
		lcd_col = DISPLAY_W - 6 * DISPLAY_FONTSMALL_W - 1;
		displaybuf_small();
		sprintf(lcd_displaybuf, "att. verin");
		lcd_line = 1;
		lcd_col = DISPLAY_W / 2 - DISPLAY_FONTMEDIUM_W * 9 / 2 - 1;
		displaybuf_medium();
		rotary_pos = 0;
		last_xmit = timer0_read();
	}
	check_light();
	if (softintrs & INT_SW_R) {
		softintrs &= ~INT_SW_R;
	}
	if ((timer0_read() - last_xmit) > 1000 && rotary_pos != 0) {
		__data struct private_command_acuator *d = (void *)&nmea2000_data[0];
		get_rotary_pos();
		msg.id.id = 0;
		msg.id.iso_pg = (PRIVATE_COMMAND_ACUATOR >> 8) & 0xff;
		msg.id.daddr = nmea2000_command_address;
		msg.id.priority = NMEA2000_PRIORITY_INFO;
		msg.dlc = sizeof(struct private_command_acuator);
		msg.data = &nmea2000_data[0];
		d->move = saved_rotary_pos;
		if (! nmea2000_send_single_frame(&msg))
			printf("send PRIVATE_COMMAND_ACUATOR failed\n");
		last_xmit = timer0_read();
	}
	if (switch_events.s.sw3) {
		send_command_engage(AUTO_OFF, received_param_slot);
		next_display_page = MAIN_DATA;
	}
	if (switch_events.s.sw2) {
		send_command_engage(AUTO_HEAD, received_param_slot);
		next_display_page = ENGAGED_DATA;
	}
}

static void
page_cogsog(char pagechange) __wparam
{
	if (pagechange) {
		print_gostop();
		print_light();
		print_menupage();
	}
	if (pagechange ||
	    (cogsog_changed &&
	     timer0_read() - last_display_update > 5000)) {
		cogsog_changed = 0;
		last_display_update = timer0_read();
		if (received_cog != HEADING_INVALID) {
			sprintf(lcd_displaybuf, "%3d  %2d.%1dkn",
			    urad2deg(received_cog), received_speed,
			    received_speedd);
		} else {
			sprintf(lcd_displaybuf, "---  --.-kn");
		}
		lcd_line = 1;
		lcd_col = DISPLAY_FONTMEDIUM_W * 1;
		displaybuf_medium();
	}

	check_light();
	if (switch_events.s.sw3) {
		next_display_page = TOWP_DATA;
	}
	if (switch_events.s.sw4) {
		next_display_page = MAIN_MENU;
	}
	if (switch_events.s.sw2) {
		send_command_engage(AUTO_STANDBY, received_param_slot);
		next_display_page = ACT_PAGE;
	}
}

static void
page_towp(char pagechange) __wparam
{
	unsigned int dist;
	unsigned int distd;
	unsigned char display_updated = 0;

	if (pagechange) {
		print_gostop();
		print_light();
		print_menupage();
	}
	if (pagechange ||
	    (towp_changed &&
	     timer0_read() - last_display_update > 5000)) {
		towp_changed = 0;
		display_updated++;
		if (received_towp_cog == HEADING_INVALID) {
			sprintf(lcd_displaybuf, "---  ---nm ");
		} else if (received_towp_dist > 1852000UL) { /* 10nm */
			dist = received_towp_dist / 185200UL;
			sprintf(lcd_displaybuf, "%3d %4dnm ",
			    urad2deg(received_towp_cog), dist);
		} else if (received_towp_dist > 18520UL) { /* 0.1nm */
			dist = received_towp_dist / 185200UL;
			distd = (received_towp_dist % 185200UL) / 1852UL;
			sprintf(lcd_displaybuf, "%3d %1d.%02dnm ",
			    urad2deg(received_towp_cog), dist, distd);
		} else { /* less than 0.1nm */
			dist = received_towp_dist / 100;
			sprintf(lcd_displaybuf, "%3d  %3dm  ",
			    urad2deg(received_towp_cog), dist);
		}
		lcd_line = 1;
		lcd_col = DISPLAY_FONTMEDIUM_W * 2;
		displaybuf_medium();
		if (received_towp_cog == HEADING_INVALID || received_towp_speed < 0) {
			sprintf(lcd_displaybuf, "       ");
		} else {
			sprintf(lcd_displaybuf, "%2d.%1dkn",
			    received_towp_speed, received_towp_speedd);
		}
		lcd_line = DISPLAY_H - 1;
		lcd_col = DISPLAY_FONTSMALL_W * 6;
		displaybuf_small();
	}

	if (pagechange ||
	    (xte_changed &&
	     timer0_read() - last_display_update > 5000)) {
		xte_changed = 0;
		display_updated++;
		if (received_xte == XTE_INVALID) {
			sprintf(lcd_displaybuf, "          ");
		} else {
			char l, r;
			long abs_xte;
			if (received_xte < 0) {
				l = ' ';
				r = '>';
				abs_xte = -received_xte;
			} else {
				l = '<';
				r = ' ';
				abs_xte = received_xte;
			}
			if (abs_xte > 1852000UL) { /* 10nm */
				dist = abs_xte / 185200UL;
				sprintf(lcd_displaybuf, "%c %4dnm %c",
				    l, dist, r);
			} else if (abs_xte > 18520UL) { /* 0.1nm */
				dist = abs_xte / 185200UL;
				distd = (abs_xte % 185200UL) / 1852UL;
				sprintf(lcd_displaybuf, "%c %1d.%02dnm %c",
				    l, dist, distd, r);
			} else { /* less than 0.1nm */
				dist = abs_xte / 100;
				sprintf(lcd_displaybuf, "%c  %3dm  %c",
				    l, dist, r);
			}
		}
		lcd_line = 0;
		lcd_col = DISPLAY_FONTSMALL_W * 6;
		displaybuf_small();
	}

	if (display_updated)
		last_display_update = timer0_read();

	check_light();
	if (switch_events.s.sw3) {
		next_display_page = TIME_DATA;
	}
	if (switch_events.s.sw4) {
		next_display_page = MAIN_MENU;
	}
	if (switch_events.s.sw2) {
		send_command_engage(AUTO_STANDBY, received_param_slot);
		next_display_page = ACT_PAGE;
	}
}

static void
page_time(char pagechange) __wparam
{
	if (pagechange) {
		print_gostop();
		print_light();
		print_menupage();
	}

	if (pagechange ||
	    (datetime_changed &&
	     timer0_read() - last_display_update > 5000)) {
		datetime_changed = 0;
		last_display_update = timer0_read();
		if (received_hours != HOUR_INVALID) {
			sprintf(lcd_displaybuf, "%2d:%02d",
			    received_hours, received_minutes);
		} else {
			sprintf(lcd_displaybuf, "--:--");
		}
		lcd_line = 1;
		lcd_col = DISPLAY_W / 2 - DISPLAY_FONTMEDIUM_W * 5 / 2 - 1;
		displaybuf_medium();
	}

	check_light();
	if (switch_events.s.sw3) {
		next_display_page = MAIN_DATA;
	}
	if (switch_events.s.sw4) {
		next_display_page = MAIN_MENU;
	}
	if (switch_events.s.sw2) {
		send_command_engage(AUTO_STANDBY, received_param_slot);
		next_display_page = ACT_PAGE;
	}
}

static void
page_light(char pagechange) __wparam
{
	char val_changed = 0;
	unsigned char i;
	static enum display_page page_previous_display_page;
	unsigned int msg_val;

	if (pagechange) {
		page_previous_display_page = previous_display_page;
		sprintf(lcd_displaybuf, "OK");
		lcd_line = DISPLAY_H - 1;
		lcd_col = 0;
		displaybuf_small();
	}
	if (softintrs & INT_SW_R) {
		short tmp_backlight_pr;
		val_changed = 1;
		get_rotary_pos();
		tmp_backlight_pr = backlight_pr;
		tmp_backlight_pr += saved_rotary_pos;
		if (tmp_backlight_pr < 1)
			tmp_backlight_pr = 1;
		if (tmp_backlight_pr > 250)
			tmp_backlight_pr = 250;
		backlight_pr = tmp_backlight_pr;
		sw_beep();
	}
	if (val_changed || pagechange) {
		CCPR2L = backlight_pr;
		A0 = A0_CTRL;
		spi_write(PAGE_ADDR(1));
		spi_write(COL_ADDR_H(DISPLAY_W / 2 - (126 / 2) - 1));
		spi_write(COL_ADDR_L(DISPLAY_W / 2 - (126 / 2) - 1));
		A0 = A0_DISP;
		for (i = 0; i < 126; i++) {
			if (i <= backlight_pr / 2 || i % 25 == 0)
				spi_write(0xff);
			else
				spi_write(0x01);
		}
		A0 = A0_CTRL;
		spi_write(PAGE_ADDR(2));
		spi_write(COL_ADDR_H(DISPLAY_W / 2 - (126 / 2) - 1));
		spi_write(COL_ADDR_L(DISPLAY_W / 2 - (126 / 2) - 1));
		A0 = A0_DISP;
		for (i = 0; i < 126; i++) {
			if (i <= backlight_pr / 2 || i % 25 == 0)
				spi_write(0xff);
			else
				spi_write(0x80);
		}
		msg_val = (int)backlight_pr * 100UL / 250UL;
		send_command_light(CONTROL_LIGHT_VAL, msg_val);
	}
	if (switch_events.s.sw4) {
		eeprom_write(BACKLIGHT_ADDR, backlight_pr);
		next_display_page = page_previous_display_page;
	}
}

static void
page_charlist(char pagechange) __wparam
{
	char val_changed = 0;
	unsigned char i;

	if (pagechange) {
		sprintf(lcd_displaybuf, "OK");
		lcd_line = DISPLAY_H - 1;
		lcd_col = 0;
		displaybuf_small();
		rotary_pos = 0;
	}
	if (softintrs & INT_SW_R) {
		val_changed = 1;
		softintrs &= ~INT_SW_R;
		sw_beep();
	}
	if (val_changed || pagechange) {
		i = (unsigned char) rotary_pos;
		sprintf(lcd_displaybuf, "%3d %c", i, i);
		lcd_line = 1;
		lcd_col = DISPLAY_W / 2 - 1;
		displaybuf_small();
	}
	if (switch_events.s.sw4) {
		next_display_page = next_page_from_auto_mode();
	}
}

static void
page_rawdata_capt(char pagechange) __wparam
{
	int rot_rate_deg;
	
	if (pagechange || timer0_read() - last_display_update > 5000) {
		last_display_update = timer0_read();
		sprintf(lcd_displaybuf, "y %3d p %4d r %4d", 
		    rad2deg(received_heading), rad2deg(received_pitch) - 180,
		    rad2deg(received_roll) - 180);
		lcd_line = 0;
		lcd_col = 0;
		displaybuf_small();
		rot_rate_deg = (float)received_rot_rate / 174.53293F;
		sprintf(lcd_displaybuf, "rot %7d addr %3u", 
		    rot_rate_deg, (unsigned int)nmea2000_capteur_address);
		lcd_line = 1;
		lcd_col = 0;
		displaybuf_small();
	}

	if (pagechange) {
		sprintf(lcd_displaybuf, "OK");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
	}
	if (switch_events.s.sw4) {
		next_display_page = next_page_from_auto_mode();
	}
}

static void
page_rawdata_cmd(char pagechange) __wparam
{
	if (pagechange || timer0_read() - last_display_update > 5000) {
		last_display_update = timer0_read();
		sprintf(lcd_displaybuf, "mode %d rudder %4d%%", 
		    received_auto_mode, received_rudder);
		lcd_line = 0;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "addr %3u param %2d", 
		    (unsigned int)nmea2000_command_address, received_param_slot);
		lcd_line = 1;
		lcd_col = 0;
		displaybuf_small();
	}

	if (pagechange) {
		sprintf(lcd_displaybuf, "OK");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
	}
	if (switch_events.s.sw4) {
		next_display_page = next_page_from_auto_mode();
	}
}


static void
page_capcorr(char pagechange) __wparam
{
	if (pagechange) {
		received_compass_offset = HEADING_INVALID;
		if ((timer0_read() - last_capteur_data) > 15000) {
			sprintf(lcd_displaybuf, "No capteur");
		} else {
			__data struct iso_request_data *data = (void *)&nmea2000_data[0];
			msg.id.id = 0;
			msg.id.iso_pg =  (ISO_REQUEST >> 8) & 0xff;
			msg.id.daddr = nmea2000_capteur_address;
			msg.id.priority = NMEA2000_PRIORITY_INFO;
			msg.dlc = sizeof(struct iso_request_data);
			msg.data = &nmea2000_data[0];
#if 0
			data->pgn[0] = (PRIVATE_COMPASS_OFFSET >> 16) & 0xff;
			data->pgn[1] = (PRIVATE_COMPASS_OFFSET >> 8) & 0xff;
			data->pgn[2] = (PRIVATE_COMPASS_OFFSET) & 0xff;
#endif
			data->pgn = PRIVATE_COMPASS_OFFSET;
			if (! nmea2000_send_single_frame(&msg)) {
				sprintf(lcd_displaybuf,
				    "send req to %d failed\n",
				    nmea2000_capteur_address);
			} else {
				sprintf(lcd_displaybuf,
				    "req sent to %d", nmea2000_capteur_address);
			}
			lcd_line = 0;
			lcd_col = 0;
			displaybuf_small();
		}
		sprintf(lcd_displaybuf, "OK");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
		rotary_pos = 0;
		sprintf(lcd_displaybuf, "CANCEL");
		lcd_line = DISPLAY_H - 1;
		lcd_col = DISPLAY_W - 6 * DISPLAY_FONTSMALL_W - 1;
		displaybuf_small();
	} else if (received_compass_offset != HEADING_INVALID) {
		int deg_offset = rad2deg(received_compass_offset) - 180;
		if (softintrs & INT_SW_R) {
			get_rotary_pos();
			deg_offset += saved_rotary_pos;
		}
		if (deg_offset > 180)
			deg_offset = -179;
		else if (deg_offset < -179)
			deg_offset = 180;
		received_compass_offset = deg2rad(deg_offset + 180);
		sprintf(lcd_displaybuf, "offset: %4d          ", deg_offset);
		lcd_line = 0;
		lcd_col = 0;
		displaybuf_small();
		
		if (switch_events.s.sw4) {
			__data struct private_compass_offset_data *data =
			    (void *)&nmea2000_data[0];
			next_display_page = RAW_DATA_CAP;
			msg.id.id = 0;
			msg.id.iso_pg = (PRIVATE_COMPASS_OFFSET >> 8) & 0xff;
			msg.id.daddr = nmea2000_capteur_address;
			msg.id.priority = NMEA2000_PRIORITY_INFO;
			msg.dlc = sizeof(struct private_compass_offset_data);
			msg.data = &nmea2000_data[0];
			data->offset = received_compass_offset;
			nmea2000_send_single_frame(&msg);
		}
	}
	if (switch_events.s.sw3) {
		next_display_page = RAW_DATA_CAP;
	}
}

static void
page_gyrocorr(char pagechange) __wparam
{
	if (pagechange) {
		if ((timer0_read() - last_capteur_data) > 15000) {
			sprintf(lcd_displaybuf, "No capteur");
		} else {
			msg.id.id = 0;
			msg.id.iso_pg =  (PRIVATE_CALIBRATE_COMPASS >> 8) & 0xff;
			msg.id.daddr = nmea2000_capteur_address;
			msg.id.priority = NMEA2000_PRIORITY_INFO;
			msg.dlc = 0;
			if (! nmea2000_send_single_frame(&msg)) {
				sprintf(lcd_displaybuf,
				    "send cal to %d failed\n");
			} else {
				sprintf(lcd_displaybuf,
				    "cal sent to %d", nmea2000_capteur_address);
			}
			lcd_line = 0;
			lcd_col = 0;
			displaybuf_small();
		}
		sprintf(lcd_displaybuf, "OK");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
	}
		
	if (switch_events.s.sw4) {
		next_display_page = RAW_DATA_CAP;
	}
}

static void
page_conf_cmd_factors(char pagechange) __wparam
{
	static char edit_digit;
	static char edit_factor;
	static char selecting;
	static char last_slot;
	if (pagechange) {
		command_factors.slot = -1;
		last_slot = -1;
		selecting = 1;
		edit_factor = 3;
		edit_digit = 3;
		if ((timer0_read() - last_command_data) > 15000) {
			sprintf(lcd_displaybuf, "No capteur");
		} else {
			send_command_request_factors(received_param_slot);
			sprintf(lcd_displaybuf, "slot erreur ROT accel");
		}
		lcd_line = 0;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "SELECT");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "CANCEL");
		lcd_line = DISPLAY_H - 1;
		lcd_col = DISPLAY_W - 6 * DISPLAY_FONTSMALL_W - 1;
		displaybuf_small();
	} else if (command_factors.slot != -1) {
		last_slot = command_factors.slot;
		command_factors.slot = -1; 
		rotary_pos = 0;
		sprintf(lcd_displaybuf, "%4d   %04d %04d %04d",
		    last_slot,
		    command_factors.factors[FACTOR_ERR],
		    command_factors.factors[FACTOR_DIF],
		    command_factors.factors[FACTOR_DIF2]);
		lcd_line = 1;
		lcd_col = 0;
		displaybuf_small();
		clearline(2);
		sprintf(lcd_displaybuf, "%c", 30);
		lcd_line = 2;
		switch(edit_factor) {
		case 3:
			lcd_col = DISPLAY_FONTSMALL_W * 3;
			break;
		default:
			lcd_col = DISPLAY_FONTSMALL_W *
			    (7 + (5 * edit_factor) + (3 - edit_digit));
			break;
		}
		displaybuf_small();
	}

	if (softintrs & INT_SW_R) {
		get_rotary_pos();
		if (edit_factor == 3) {
			char new_slot;
			/* select slot */
			new_slot = last_slot + saved_rotary_pos;
			if (new_slot >= 0) {
				send_command_request_factors(new_slot);
				sw_beep();
			}
		} else if (selecting == 1) {
			char new_digit;
			new_digit = edit_digit - saved_rotary_pos;
			while  (new_digit < 0) {
				/* next factor */
				new_digit += 4;
				edit_factor = edit_factor + 1;
				if (edit_factor == 3) {
					new_digit = 3;
					edit_factor = 3;
					break;
				}
			}
			while  (new_digit > 3) {
				/* next factor */
				new_digit -= 4;
				edit_factor = edit_factor - 1;
				if (edit_factor == 0) {
					new_digit = 3;
					edit_factor = 3;
					break;
				}
			}
			edit_digit = new_digit;
			command_factors.slot = last_slot; /* cause redraw */
			sw_beep();
		} else {
			/* changing current digit */
			int new_factor;
			new_factor = command_factors.factors[edit_factor];

			switch(edit_digit) {
			case 3:
				new_factor += (int)saved_rotary_pos * (int)1000;
				break;
			case 2:
				new_factor += (int)saved_rotary_pos * (int)100;
				break;
			case 1:
				new_factor += (int)saved_rotary_pos * (int)10;
				break;
			case 0:
				new_factor += saved_rotary_pos;
				break;
			}
			if (new_factor >= 0 && new_factor < 10000)
				command_factors.factors[edit_factor] =
				    new_factor;
				 command_factors.slot = last_slot;
				 sw_beep();
			}
	}

	if (switch_events.s.sw4) {
		command_factors.slot = last_slot;
		if (selecting == 0) {
			/* send new value and go back to selecting */
			send_command_factors();
			sprintf(lcd_displaybuf, "SELECT");
			lcd_line = 3;
			lcd_col = 0;
			displaybuf_small();
			selecting = 1;
		} else {
			/* change digit if applicable */
			if (edit_factor < 3) {
				sprintf(lcd_displaybuf, "SET   ");
				lcd_line = 3;
				lcd_col = 0;
				displaybuf_small();
				selecting = 0;
			} else {
				edit_factor = 0;
			}
		}
	}
	if (switch_events.s.sw3) {
		if (selecting == 0) {
			/* go back to selecting, without sending */
			sprintf(lcd_displaybuf, "SELECT");
			lcd_line = 3;
			lcd_col = 0;
			displaybuf_small();
			selecting = 1;
		} else {
			/* out of this menu */
			next_display_page = next_page_from_auto_mode();
		}
	}
}

static void
page_conf_cmd_config(char pagechange) __wparam
{
	static char last_slot;
	if (pagechange) {
		command_factors.slot = received_param_slot;
		last_slot = received_param_slot;
		rotary_pos = 0;
		sprintf(lcd_displaybuf, "SELECT");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "CANCEL");
		lcd_line = DISPLAY_H - 1;
		lcd_col = DISPLAY_W - 6 * DISPLAY_FONTSMALL_W - 1;
		displaybuf_small();
	} else if (command_factors.slot != -1) {
		last_slot = command_factors.slot;
		command_factors.slot = -1; 
		sprintf(lcd_displaybuf, "Prog %2d", last_slot);
		lcd_line = 1;
		lcd_col = DISPLAY_W / 2 - (7 / 2) * DISPLAY_FONTMEDIUM_W - 1;
		displaybuf_medium();
	}

	if (softintrs & INT_SW_R) {
		char new_slot;
		get_rotary_pos();
		/* select slot */
		new_slot = last_slot + saved_rotary_pos;
		if (new_slot >= 0) {
			send_command_request_factors(new_slot);
			sw_beep();
		}
	}

	if (switch_events.s.sw4) {
		send_command_engage(received_auto_mode, last_slot);
		next_display_page = next_page_from_auto_mode();
	}
	if (switch_events.s.sw3) {
		next_display_page = next_page_from_auto_mode();
	}
}

static void
page_err(char pagechange) __wparam
{
	const char *msg;
	unsigned char msglen;
	static enum display_page page_previous_display_page;
	static unsigned char previous_backlight;
	static unsigned last_err;
	union displar_errs toack;

	if (pagechange) {
		page_previous_display_page = previous_display_page;
		previous_backlight = CCPR2L;
		CCPR2L = 120;
		PR4 = SPKR_1000;
		T4CONbits.TMR4ON = 1;
		beep_duration = 0;
		last_err = 0;
	}
	if (err_list.byte == 0) {
		next_display_page = page_previous_display_page;
		CCPR2L = previous_backlight;
		T4CONbits.TMR4ON = 0;
		SPKR_PORT = 0;
		return;
	}
	if (err_list.byte != last_err) {
		clearline(1);
		if (err_list.bits.nocapt) {
			msg = "CAPTEUR MUET";
			msglen = 12;
			toack.bits.nocapt = 1;
		} else if (err_list.bits.nocmd) {
			msg = "COMMANDE MUETTE";
			msglen = 15;
			toack.bits.nocmd = 1;
		} else {
			msg = "INCONUE";
			msglen = 7;
		}

		lcd_col = DISPLAY_W / 2 - (msglen / 2) * DISPLAY_FONTMEDIUM_W - 1;
		lcd_line = 1;
		sprintf(lcd_displaybuf, "%s", msg);
		displaybuf_medium();

		sprintf(lcd_displaybuf, "ACK");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
		last_err = err_list.byte;
	}

	if (switch_events.s.sw4) {
		err_ack.byte |= toack.byte;
		err_list.byte &= ~toack.byte;
	}
}

static void
page_cmderr(char pagechange) __wparam
{
	const char *msg;
	unsigned char msglen;
	static enum display_page page_previous_display_page;
	static unsigned char previous_backlight;
	static unsigned last_err;
	union command_errors toack;

	if (pagechange) {
		page_previous_display_page = previous_display_page;
		previous_backlight = CCPR2L;
		CCPR2L = 120;
		PR4 = SPKR_1000;
		T4CONbits.TMR4ON = 1;
		beep_duration = 0;
		last_err = 0;
	}
	if (cmderr_list.byte == 0) {
		next_display_page = page_previous_display_page;
		CCPR2L = previous_backlight;
		T4CONbits.TMR4ON = 0;
		SPKR_PORT = 0;
		return;
	}
	if (cmderr_list.byte != last_err) {
		clearline(1);
		if (cmderr_list.bits.no_capteur_data) {
			msg = "CAPTEUR MUET";
			msglen = 12;
			toack.bits.no_capteur_data = 1;
		} else if (cmderr_list.bits.no_rudder_data) {
			msg = "ANGLE DE BARRE";
			msglen = 14;
			toack.bits.no_rudder_data = 1;
		} else if (cmderr_list.bits.output_overload) {
			msg = "SURCHARGE";
			msglen = 9;
			toack.bits.output_overload = 1;
		} else if (cmderr_list.bits.output_error) {
			msg = "PWR ERROR";
			msglen = 9;
			toack.bits.output_error = 1;
		} else {
			msg = "INCONUE";
			msglen = 7;
		}

		lcd_col = DISPLAY_W / 2 - (msglen / 2) * DISPLAY_FONTMEDIUM_W - 1;
		lcd_line = 1;
		sprintf(lcd_displaybuf, "%s", msg);
		displaybuf_medium();

		sprintf(lcd_displaybuf, "ACK");
		lcd_line = 3;
		lcd_col = 0;
		displaybuf_small();
		sprintf(lcd_displaybuf, "ERREUR COMMANDE");
		lcd_line = 0;
		lcd_col = 20;
		displaybuf_small();
		last_err = cmderr_list.byte;
	}

	if (switch_events.s.sw4) {
		send_command_errack(toack.byte);
	}
}

static void
sw_beep()
{
	// unsigned short count;
	PR4 = SPKR_1000;
	T4CONbits.TMR4ON = 1;
	beep_duration = SPKR_1000_D100 / 2; /* 0.05s */
#if 0
	count = timer0_read();
	while (timer0_read() - count < 500)
		; // wait
	T4CONbits.TMR4ON = 0;
	SPKR_PORT = 0;
#endif
}

static void
check_light()
{
	if (switch_events.s.sw1) {
		if (CCPR2L == 0) {
			/* light on */
			CCPR2L = backlight_pr;
			send_command_light(CONTROL_LIGHT_ON, 0);
		} else {
			/* light off */
			CCPR2L = 0;
			send_command_light(CONTROL_LIGHT_OFF, 0);
		}
	}
	if (switch_events.s.sw1l) {
		next_display_page = LIGHT;
		CCPR2L = backlight_pr;
		send_command_light(CONTROL_LIGHT_ON, 0);
	}
}

void
main(void) __naked
{
	char c;
	int second_counts = 0;
	static unsigned short poll_count;
	unsigned short count;
	char new_page;
	static unsigned short sw1_count;
	static unsigned short sw2_count;
	static unsigned short sw3_count;
	static unsigned short sw4_count;


	softintrs = 0;
	inputs_status.v = 0;
	counter_1hz = 500;

	rotary_pos = 0;

	lcd_ready = 0;

	received_heading = received_cog = received_towp_cog = HEADING_INVALID;
	received_xte = XTE_INVALID;

	nmea2000_navdata_len = 0xff;

	/* everything is low priority by default */
	IPR1 = 0;
	IPR2 = 0;
	IPR3 = 0;
	IPR4 = 0;
	IPR5 = 0;
	INTCON = 0;
	INTCON2 = 0;
	INTCON3 = 0;

	USART_INIT(0);
	SPI_INIT;

	INTCON2bits.NOT_RBPU = 0; /* enable port B pullups */
	TRISB |= 0xbf; /* all but RB6 are inputs */
	WPUB = 0x3f; /* activate pullups on RB0-RB5 */
	ANCON1 &= 0xf8;


	RCONbits.IPEN=1; /* enable interrupt priority */

	/* configure timer0 as free-running counter at 9.765625Khz */
	T0CON = 0x07; /* b00000111: internal clock, 1/256 prescaler */
	INTCONbits.TMR0IF = 0;
	INTCONbits.TMR0IE = 0; /* no interrupt */
	T0CONbits.TMR0ON = 1;

	/* configure timer2 for 500hz interrupt */
	PMD1bits.TMR2MD=0;
	T2CON = 0x21; /* b00100001: postscaller 1/5, prescaler 1/4 */
	PR2 = 250; /* 500hz output */
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

	INTCONbits.GIE_GIEH=1;  /* enable high-priority interrupts */
	INTCONbits.PEIE_GIEL=1; /* enable low-priority interrrupts */

	/* RA3 output (speaker) */
	SPKR_PORT = 0;
	TRISAbits.TRISA3 = 0;
	ANCON0bits.ANSEL3 = 0;

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

	backlight_pr = eeprom_read(BACKLIGHT_ADDR);
	if (backlight_pr < 1 || backlight_pr > 250)
		backlight_pr = 250 / 2;

	/* port C 2 output (backlight), using PWM2 */
	CCPTMRSbits.C2TSEL=0; /* select timer2 */
	CCPR2L = 0; /* start off */
	CCP2CON = 0x0c; /* PWM mode */
	TRISCbits.TRISC2 = 0;

	stdout = STREAM_USER; /* Use the macro PUTCHAR with printf */

	printf("hello world\n");

	/* init LDC */
	TRISAbits.TRISA6 = 0;
	LATAbits.LATA6 = 0; /* chip select CS1B */

	TRISCbits.TRISC1 = 0;
	A0 = A0_CTRL;

	TRISCbits.TRISC0 = 0;
	resetdisplay();
	/* enable watch dog timer */
	WDTCON = 0x01;
	lcd_col = 0;
	lcd_line = 0;
	printf("ready");
	poll_count = timer0_read();
	while (nmea2000_addr_status != ADDR_STATUS_OK) {
		nmea2000_poll(5);
		while ((timer0_read() - poll_count) < 48)
			; // wait 5ms
		poll_count = timer0_read();
		CLRWDT;
	}
	printf(", addr %d, id %ld\n", nmea2000_addr, nmea2000_user_id);
	PR4 = SPKR_440;
	T4CONbits.TMR4ON = 1;
	count = timer0_read();
	while (timer0_read() - count < 10000)
		; // wait
	T4CONbits.TMR4ON = 0;
	SPKR_PORT = 0;
	CCPR2L = backlight_pr; /* backlight on */

	previous_display_page = next_display_page = display_page = MAIN_DATA;
	new_page = 1;
	cleardisplay();
	err_list.byte = 0;
	err_ack.byte = 0;
	cmderr_list.byte = 0;
	received_auto_mode = AUTO_OFF;
	check_light_long = 0;

again:
	while (1) {
		CLRWDT;
		if (PIR3bits.RC2IF) {
			char c = RCREG2;
			if (c == 'r')
				break;
#ifdef DISPLAY_FAKE
			switch(c) {
			case '1':
				if (sw1 != 0x7f) {
					sw1 = 0x7f;
					softintrs |= INT_SW_1P;
				}
				break;
			case '2':
				if (sw2 != 0x7f) {
					sw2 = 0x7f;
					softintrs |= INT_SW_2P;
				}
				break;
			case '3':
				if (sw3 != 0x7f) {
					sw3 = 0x7f;
					softintrs |= INT_SW_3P;
				}
				break;
			case '4':
				if (sw4 != 0x7f) {
					sw4 = 0x7f;
					softintrs |= INT_SW_4P;
				}
				break;
			case '0':
				if (sw1 == 0x7f) {
					sw1 = 0;
					softintrs |= INT_SW_1R;
				}
				if (sw2 == 0x7f) {
					sw2 = 0;
					softintrs |= INT_SW_2R;
				}
				if (sw3 == 0x7f) {
					sw3 = 0;
					softintrs |= INT_SW_3R;
				}
				if (sw4 == 0x7f) {
					sw4 = 0;
					softintrs |= INT_SW_4R;
				}
				break;
			case '-':
				rotary_pos -= 1;
				softintrs |= INT_SW_R;
				break;
			case '+':
				rotary_pos += 1;
				softintrs |= INT_SW_R;
				break;
			}
#endif
		}

		if (PIR5bits.RXBnIF)
			nmea2000_receive();

		if (nmea2000_addr_status == ADDR_STATUS_CLAIMING) {
			if ((timer0_read() - poll_count) > 48) {
				nmea2000_poll(5);
				poll_count = timer0_read();
			}
		};

		if (softintrs & INT_1HZ) {
			second_counts++;
			softintrs &= ~INT_1HZ;
		}

		switch_events.v = 0;
		if (inputs_status.s.sw1) {
			if ((timer0_read() - sw1_count) > 10000) {
				switch_events.s.sw1l = 1;
				inputs_status.s.sw1 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_1R) {
			softintrs &= ~INT_SW_1R;
			if (inputs_status.s.sw1) {
				switch_events.s.sw1 = 1;
				inputs_status.s.sw1 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_1P) {
			softintrs &= ~INT_SW_1P;
			if (inputs_status.s.sw1 == 0) {
				inputs_status.s.sw1 = 1;
				sw1_count = timer0_read();
			}
		}

		if (inputs_status.s.sw2) {
			if ((timer0_read() - sw2_count) > 10000) {
				switch_events.s.sw2l = 1;
				inputs_status.s.sw2 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_2R) {
			softintrs &= ~INT_SW_2R;
			if (inputs_status.s.sw2) {
				switch_events.s.sw2 = 1;
				inputs_status.s.sw2 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_2P) {
			softintrs &= ~INT_SW_2P;
			if (inputs_status.s.sw2 == 0) {
				inputs_status.s.sw2 = 1;
				sw2_count = timer0_read();
			}
		}

		if (inputs_status.s.sw3) {
			if ((timer0_read() - sw3_count) > 10000) {
				switch_events.s.sw3l = 1;
				inputs_status.s.sw3 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_3R) {
			softintrs &= ~INT_SW_3R;
			if (inputs_status.s.sw3) {
				switch_events.s.sw3 = 1;
				inputs_status.s.sw3 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_3P) {
			softintrs &= ~INT_SW_3P;
			if (inputs_status.s.sw3 == 0) {
				inputs_status.s.sw3 = 1;
				sw3_count = timer0_read();
			}
		}

		if (inputs_status.s.sw4) {
			if ((timer0_read() - sw4_count) > 10000) {
				switch_events.s.sw4l = 1;
				inputs_status.s.sw4 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_4R) {
			softintrs &= ~INT_SW_4R;
			if (inputs_status.s.sw4) {
				switch_events.s.sw4 = 1;
				inputs_status.s.sw4 = 0;
				sw_beep();
			}
		}
		if (softintrs & INT_SW_4P) {
			softintrs &= ~INT_SW_4P;
			if (inputs_status.s.sw4 == 0) {
				inputs_status.s.sw4 = 1;
				sw4_count = timer0_read();
			}
		}

		if (switch_events.s.sw2l) {
			switch_events.s.sw2l = 0;
			send_command_mob();
			next_display_page = TOWP_DATA;
		}

		switch(display_page) {
		case MAIN_DATA:
			page_maindata(new_page);
			break;
		case ACT_PAGE:
			page_act(new_page);
			break;
		case ENGAGED_DATA:
			page_engageddata(new_page);
			break;
		case COGSOG_DATA:
			page_cogsog(new_page);
			break;
		case TOWP_DATA:
			page_towp(new_page);
			break;
		case TIME_DATA:
			page_time(new_page);
			break;
		case LIGHT:
			page_light(new_page);
			break;
		case MAIN_MENU:
			page_mainmenu(new_page);
			break;
		case CONF_MENU:
			page_confmenu(new_page);
			break;
		case RAW_MENU:
			page_rawmenu(new_page);
			break;
		case RAW_DATA_CAP:
			page_rawdata_capt(new_page);
			break;
		case RAW_DATA_CMD:
			page_rawdata_cmd(new_page);
			break;
		case CHAR_LIST:
			page_charlist(new_page);
			break;
		case PAGE_ERR:
			page_err(new_page);
			break;
		case PAGE_CMDERR:
			page_cmderr(new_page);
			break;
		case CAP_CORR:
			page_capcorr(new_page);
			break;
		case GYRO_CORR:
			page_gyrocorr(new_page);
			break;
		case CONF_CMD_FACTORS:
			page_conf_cmd_factors(new_page);
			break;
		case CONF_CMD_CONFIG:
			page_conf_cmd_config(new_page);
			break;
		default:
			printf("unknown menu %d\n", display_page);
			goto end;
		}

		if ((timer0_read() - last_nmea2000_cogsog) > 50000 &&
		    received_cog != HEADING_INVALID) {
			received_cog = HEADING_INVALID;
			cogsog_changed = 1;
		};
		if ((timer0_read() - last_nmea2000_xte) > 50000 &&
		    received_xte != XTE_INVALID) {
			received_xte = XTE_INVALID;
			xte_changed = 1;
		};
		if ((timer0_read() - last_nmea2000_navdata) > 50000 &&
			received_towp_cog != HEADING_INVALID) {
			received_towp_cog = HEADING_INVALID;
			towp_changed = 1;
		};
		if ((timer0_read() - last_datetime_data) > 50000 &&
			received_hours != HOUR_INVALID) {
			received_hours = HOUR_INVALID;
			datetime_changed = 1;
		};
		if ((timer0_read() - last_capteur_data) > 50000) {
			if (!err_list.bits.nocapt && !err_ack.bits.nocapt)
				err_list.bits.nocapt = 1;
			received_heading = HEADING_INVALID;
		} 
		if ((timer0_read() - last_command_data) > 50000) {
			if (!err_list.bits.nocmd && !err_ack.bits.nocmd)
				err_list.bits.nocmd = 1;
		} 
		if (err_list.byte)
			next_display_page = PAGE_ERR;
		else if (cmderr_list.byte)
			next_display_page = PAGE_CMDERR;

		if (next_display_page != display_page) {
			resetdisplay();
			if (display_page != PAGE_ERR &&
			    display_page != PAGE_CMDERR)
				previous_display_page = display_page;
			display_page = next_display_page;
			new_page = 1;
		} else {
			new_page = 0;
		}
	}

	printf("suspended ");
	while ((c = getchar()) != 'r') {
		printf("resumed\n");
		goto again;
	}
end:
	printf("returning\n");
	while (PIE3bits.TX2IE)
		; /* wait for transmit to complete */
	INTCONbits.PEIE=0; /* disable peripheral interrupts */
	INTCONbits.GIE=0;  /* disable interrrupts */
}

unsigned short
timer0_read() __naked
{
	/* return TMR0L | (TMR0H << 8), reading TMR0L first */
	__asm
	movf	_TMR0L, w
	movff	_TMR0H, _PRODL
	return
	__endasm;
}

void
delay(unsigned short d)
{
	short start = timer0_read();
	while (timer0_read() < start + d)
		; /* wait */
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
	__asm
	MOVFF	r0x00, POSTDEC1
	__endasm;
	counter_1hz--;
	if (counter_1hz == 0) {
		counter_1hz = 500;
		softintrs |= INT_1HZ;
	}
#ifndef DISPLAY_FAKE
	rotary_a = rotary_a << 1;
	rotary_a &= (unsigned char)0xf;
	if (ROTARY_A)
		rotary_a |= (unsigned char)1;
	rotary_b = rotary_b << 1;
	rotary_b &= (unsigned char)0xf;
	if (ROTARY_B)
		rotary_b |= (unsigned char)1;
	if (rotary_a == 0x0f) {
		if (!inputs_status.s.rotary_a) { /* rising edge on A */
			if (inputs_status.s.pending_b) {
				/* already got rising edge on B */
				rotary_pos -= (char)1;
				softintrs |= INT_SW_R;
				inputs_status.s.pending_b = 0;
			} else {
				inputs_status.s.pending_a = 1;
			}
		}
		inputs_status.s.rotary_a = 1;
	} else if (rotary_a == 0) {
		inputs_status.s.rotary_a = 0;
		inputs_status.s.pending_a = 0;
	}
	if (rotary_b == 0x0f) {
		if (!inputs_status.s.rotary_b) { /* rising edge on B */
			if (inputs_status.s.pending_a) {
				/* already got rising edge on A */
				rotary_pos += (char)1;
				softintrs |= INT_SW_R;
				inputs_status.s.pending_a = 0;
			} else {
				inputs_status.s.pending_b = 1;
			}
		}
		inputs_status.s.rotary_b = 1;
	} else if (rotary_b == 0) {
		inputs_status.s.rotary_b = 0;
		inputs_status.s.pending_b = 0;
	}

	if (SW1) {
		if (sw1 == 0x7f) {
			softintrs |= INT_SW_1P;
		}
		sw1 = sw1 << 1;
		sw1 |= (unsigned char)1;
	} else {
		if (sw1 == 0x80) {
			softintrs |= INT_SW_1R;
		}
		sw1 = sw1 << 1;
	}
	if (SW2) {
		if (sw2 == 0x7f) {
			softintrs |= INT_SW_2P;
		}
		sw2 = sw2 << 1;
		sw2 |= (unsigned char)1;
	} else {
		if (sw2 == 0x80) {
			softintrs |= INT_SW_2R;
		}
		sw2 = sw2 << 1;
	}
	if (SW3) {
		if (sw3 == 0x7f) {
			softintrs |= INT_SW_3P;
		}
		sw3 = sw3 << 1;
		sw3 |= (unsigned char)1;
	} else {
		if (sw3 == 0x80) {
			softintrs |= INT_SW_3R;
		}
		sw3 = sw3 << 1;
	}
	if (SW4) {
		if (sw4 == 0x7f) {
			softintrs |= INT_SW_4P;
		}
		sw4 = sw4 << 1;
		sw4 |= (unsigned char)1;
	} else {
		if (sw4 == 0x80) {
			softintrs |= INT_SW_4R;
		}
		sw4 = sw4 << 1;
	}
#endif
	__asm
	MOVFF	PREINC1, r0x00
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



void _irql (void) __interrupt 2 /* low priority */
{
	USART_INTR;
	if (PIR5 != 0)
		nmea2000_intr();
	if (PIR5bits.RXBnIF)
		PIE5bits.RXBnIE = 0; /* main loop will check */
	if (PIR5 != 0)
		PIE5 &= _PIE5_TXBnIE;
}
