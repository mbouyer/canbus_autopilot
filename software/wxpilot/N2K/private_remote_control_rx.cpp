/*
 * Copyright (c) 2019 Manuel Bouyer
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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

#include "NMEA2000.h"
#include "nmea2000_defs_rx.h"
#include <wxpilot.h>

#define CONTROL_MOB     0x00
#define CONTROL_MOB_MARK        0x00
#define CONTROL_MOB_SIZE 2
#define CONTROL_LIGHT   0x01
#define CONTROL_LIGHT_OFF       0x00
#define CONTROL_LIGHT_ON        0x01
#define CONTROL_LIGHT_VAL       0x02
#define CONTROL_LIGHT_SIZE     2
#define CONTROL_LIGHT_VAL_SIZE 3
#define CONTROL_RESET   0x02
#define CONTROL_RESET_SIZE 1
#define CONTROL_MUTE    0x02
#define CONTROL_MUTE_SIZE 2
#define CONTROL_BEEP    0x04
#define CONTROL_BEEP_SHORT      0x00
#define CONTROL_BEEP_LONG       0x01  
#define CONTROL_BEEP_SIZE 2
#define CONTROL_REMOTE_RADIO    0x05
#define CONTROL_REMOTE_RADIO_SIZE 5


bool
private_remote_control_rx::handle(const nmea2000_frame &f)
{
	int type, subtype;
	
	type = f.frame2uint8(0);
	subtype = f.frame2uint8(1);
	switch(type) {
	case CONTROL_REMOTE_RADIO:
	{
		if (f.getlen() != CONTROL_REMOTE_RADIO_SIZE || subtype != 0)
			return false;
		int last_txv = f.frame2uint8(2);
		int last_state = f.frame2uint8(3);
		int last_rssi = f.frame2uint8(4);
		wxp->setRadio(last_txv, last_state, last_rssi);
		return true;
		break;
	}
	default:
		return false;
	}
}
