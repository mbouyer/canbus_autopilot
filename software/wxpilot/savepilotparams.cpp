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

#include "savepilotparams.h"

savePilotParams::savePilotParams(wxWindow *parent) :
    config(wxT("noconfig"))
{

	filedialog = new wxFileDialog(parent);
	command_factors_request_tx = (private_command_factors_request_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST));
}

bool savePilotParams::doSave(void)
{
	if (filedialog->ShowModal() != wxID_OK) {
		delete filedialog;
		return false;
	}
	fileName = filedialog->GetPath();
	delete filedialog;
	return requestSlot(0);
}

bool savePilotParams::requestSlot(int s)
{
	slot = s;
	command_factors_request_tx->update(slot);
	if (!nmea2000P->send_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST, true)) {
		wxMessageDialog *dial = new wxMessageDialog(NULL, 
		    wxString::Format(_T("errror requesting slot %d"), slot),
		    wxT("Error"), wxOK | wxICON_ERROR);
		dial->ShowModal();
		delete dial;
		return false;
	}
	return true;
}

bool
savePilotParams::setValues(int s, int v[NFACTORS])
{
	if (s != slot)
		return true;

	config.Write(wxString::Format(_T("/group%d/error"), s), (long)v[0]);
	config.Write(wxString::Format(_T("/group%d/ROT"), s), (long)v[1]);
	config.Write(wxString::Format(_T("/group%d/accel"), s), (long)v[2]);
	s++;
	if (s < NPARAMS) {
		return requestSlot(s);
	}

	wxFileOutputStream os(fileName);
	if (!os.IsOk()) {
		/* error dialog already done */
		return false;
	}
	if (!config.Save(os) || !os.Close()) {
		wxMessageDialog *dial = new wxMessageDialog(NULL, 
		    wxString::Format(_T("errror saving file %s %s"),
		    fileName.mb_str(), strerror(errno)),
		    wxT("Error"), wxOK | wxICON_ERROR);
		dial->ShowModal();
		delete dial;
		return false;
	}
		
	wxMessageDialog *dial = new wxMessageDialog(NULL, 
	    wxString::Format(_T("Params saved to %s"), fileName.mb_str()),
	    wxT("Info"), wxOK);
	dial->ShowModal();
	delete dial;
	return false;
}
