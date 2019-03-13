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
#include "nmea2000_defs_tx.h"
#include <wxpilot.h>

bool private_command_status_rx::handle(const nmea2000_frame &f)
{

	if (command_factors_request == NULL) {
		command_factors_request = nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST));
		command_factors_request->setdst(f.getsrc());
		command_factors = nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS));
		command_factors->setdst(f.getsrc());
		std::cout << "Command address " << f.getsrc() << std::endl;
	}
	gettimeofday(&last_rx, NULL);
	int heading = f.frame2int16(0);
	int8_t error =  f.frame2int8(2);
	int8_t auto_mode =  f.frame2int8(3);
#define AUTO_OFF 0x0000
#define AUTO_STANDBY 0x0001
#define AUTO_HEAD 0x0002
	int8_t rudder =  -f.frame2int8(4);
	int8_t params_slot =  f.frame2int8(5);

	wxp->setPilotHeading(rad2deg(heading), (auto_mode == AUTO_HEAD));
	wxp->setRudder(rudder, true);
	if (addr != f.getsrc()) {
		command_factors_request->setdst(f.getsrc());
		command_factors->setdst(f.getsrc());
	}
	if (addr != f.getsrc() || group != params_slot || mode != auto_mode) {
		addr = f.getsrc();
		group = params_slot;
		mode = auto_mode;
		switch(auto_mode) {
		case AUTO_OFF:
			wxp->setStatus(f.getsrc(), params_slot, _T("Off"));
			break;
		case AUTO_STANDBY:
			wxp->setStatus(f.getsrc(), params_slot, _T("Standby"));
			break;
		case AUTO_HEAD:
			wxp->setStatus(f.getsrc(), params_slot, _T("On"));
			break;
		}
	}
	return true;
}

void private_command_status_rx::tick()
{
	struct timeval now, diff;
	gettimeofday(&now, NULL);
	timersub(&now, &last_rx, &diff);
	if (diff.tv_sec >= 5) {
		addr = -1;
		group = -1;
		mode = -1;

		wxp->setPilotHeading(0, false);
		wxp->setRudder(0, false);
		wxp->setStatus(-1, -1, _T("commande muette"));
	}
}
