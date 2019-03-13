/* $Id: menu.c,v 1.6 2017/06/05 11:00:19 bouyer Exp $ */
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

#include <menu.h>

/*
 * because of sdcc/gpasm bugs/limitations, this has to be alone in its own
 * file
 */
const struct page_menu_entry const page_menu[] = {
	{"cap", MAIN_DATA },
	{"auto", ENGAGED_DATA },
	{"cog/sog", COGSOG_DATA },
	{"waypoint", TOWP_DATA },
	{"heure", TIME_DATA },
	{"retro-eclairage", LIGHT },
	{"raw data", RAW_MENU }, 
	{"configuration", CONF_MENU }, 
	{"character list", CHAR_LIST }, 
};      
const struct page_menu_entry const conf_menu[] = {
	{"cap correction", CAP_CORR },
	{"gyro calibrate", GYRO_CORR },
	{"command factors", CONF_CMD_FACTORS },
	{"command config", CONF_CMD_CONFIG },
};      
const struct page_menu_entry const raw_menu[] = {
	{"capteur data", RAW_DATA_CAP },
	{"command data", RAW_DATA_CMD },
};      
