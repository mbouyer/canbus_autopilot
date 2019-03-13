/* $Id: menu.h,v 1.6 2017/06/05 11:00:19 bouyer Exp $ */
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

enum display_page {     
        MAIN_DATA,      
        ENGAGED_DATA,      
        COGSOG_DATA,      
        TOWP_DATA,      
        TIME_DATA,      
	LIGHT,
	ACT_PAGE,
	MAIN_MENU,
	CONF_MENU,
	RAW_MENU,
	RAW_DATA_CMD,
	RAW_DATA_CAP,
	CHAR_LIST,
	CAP_CORR,
	GYRO_CORR,
	CONF_CMD_FACTORS,
	CONF_CMD_CONFIG,
	PAGE_ERR,
	PAGE_CMDERR,
	NOP,
	BACK,
};

struct page_menu_entry {
	const char * const string;
	const enum display_page page;
};              

extern const struct page_menu_entry const page_menu[];
extern const struct page_menu_entry const conf_menu[];
extern const struct page_menu_entry const raw_menu[];
