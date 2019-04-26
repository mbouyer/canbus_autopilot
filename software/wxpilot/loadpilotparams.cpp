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

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <iostream>
#include <N2K/NMEA2000.h>
#include <errno.h>
#include <string.h>

#include "loadpilotparams.h"

loadPilotParams::loadPilotParams(wxWindow *parent)
{

	filedialog = new wxFileDialog(parent);
	command_factors_tx = (private_command_factors_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS));
}

bool loadPilotParams::doLoad(void)
{
	if (filedialog->ShowModal() != wxID_OK) {
		delete filedialog;
		return false;
	}
	fileName = filedialog->GetPath();
	delete filedialog;
	wxFileInputStream is(fileName);
	if (!is.IsOk()) {
		/* dialog error already displayed */
		return false;
	}
	config = new wxFileConfig(is);
	if (config == NULL) {
		wxMessageDialog *dial = new wxMessageDialog(NULL, 
		    wxString::Format(_T("errror loading file %s %s"),
		    fileName.mb_str(), strerror(errno)),
		    wxT("Error"), wxOK | wxICON_ERROR);
		dial->ShowModal();
		delete dial;
		return false;
	}

	int v[NFACTORS];
	for (int slot = 0; slot < NPARAMS; slot++) {
		v[0] = config->ReadLong(wxString::Format(_T("/group%d/error"), slot), -1);
		v[1] = config->ReadLong(wxString::Format(_T("/group%d/ROT"), slot), -1);
		v[2] = config->ReadLong(wxString::Format(_T("/group%d/accel"), slot), -1);
		if (v[0] > 0 && v[1] > 0 && v[2] > 0) {
			command_factors_tx->update(slot, v);
			nmea2000P->send_bypgn(PRIVATE_COMMAND_FACTORS, true);
		}
	}
	delete config;
	return true;
}
