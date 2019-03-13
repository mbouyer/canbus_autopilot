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
#include <iostream>
#include <N2K/NMEA2000.h>

#include "pilotparams.h"

pilotParams::pilotParams(wxWindow *parent, wxWindowID id)
	: wxPanel(parent, id)
{

	command_factors_tx = (private_command_factors_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS));
	command_factors_request_tx = (private_command_factors_request_tx *)nmea2000P->get_frametx(nmea2000P->get_tx_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST));
	slot = wxNOT_FOUND;

	wxSizerFlags ctrlfl(0);
	ctrlfl.Expand().Border(wxALL, 5);
	controlsizer = new wxBoxSizer( wxVERTICAL );

	paramsGroupsList = new wxArrayString;
	for (int i = 0; i < NPARAMS; i++) {
		wxString str;
		str.Printf("group %d", i);
		paramsGroupsList->Add(str);
	}
	paramsGroups = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, *paramsGroupsList, wxCB_READONLY);
	paramsGroups->Connect(wxEVT_COMBOBOX, wxCommandEventHandler(pilotParams::OnParamGroupSelect), NULL, this);
	controlsizer->Add(paramsGroups, ctrlfl);
	applyButton = new wxButton(this, wxID_APPLY, _T("Apply"));
	applyButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, 
	      wxCommandEventHandler(pilotParams::OnApply), NULL, this);
	controlsizer->Add(applyButton,  ctrlfl);

	resetButton = new wxButton(this, wxID_CANCEL, _T("Reset"));
	resetButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, 
	      wxCommandEventHandler(pilotParams::OnReset), NULL, this);
	controlsizer->Add(resetButton,  ctrlfl);

	inputsizer = new wxBoxSizer( wxVERTICAL );
	wxSizerFlags intputfl(0);
	intputfl.Expand().Border(wxALL, 5);

	param_p = new wxNumberControl(this, wxID_ANY);
	inputsizer->Add(param_p, intputfl);
	param_i = new wxNumberControl(this, wxID_ANY);
	inputsizer->Add(param_i, intputfl);
	param_d = new wxNumberControl(this, wxID_ANY);
	inputsizer->Add(param_d, intputfl);

	mainsizer = new wxBoxSizer( wxHORIZONTAL );
	mainsizer->Add(controlsizer, wxSizerFlags(1).Expand());
	mainsizer->Add(inputsizer, wxSizerFlags(1).Expand());
	SetSizerAndFit(mainsizer);
}

void
pilotParams::OnParamGroupSelect(wxCommandEvent & event)
{
	int n = paramsGroups->GetSelection();
	if (n == wxNOT_FOUND)
		return;
	if (!setSlot(n)) {
		std::cerr << "OnParamGroupSelect: setSlot failed" << std::endl;
		paramsGroups->SetSelection(slot);
	}
}


void
pilotParams::OnApply(wxCommandEvent & event)
{
	if (slot == wxNOT_FOUND)
		return;
	int values[3];
	values[0] = param_p->GetValue();
	values[1] = param_i->GetValue();
	values[2] = param_d->GetValue();
	command_factors_tx->update(slot, values);
	nmea2000P->send_bypgn(PRIVATE_COMMAND_FACTORS, true);
}

void
pilotParams::OnReset(wxCommandEvent & event)
{
	if (slot == wxNOT_FOUND) {
		param_p->SetValue(0);
		param_i->SetValue(0);
		param_d->SetValue(0);
	} else {
		requestSlot(slot);
	}
}

bool
pilotParams::requestSlot(int s)
{
	command_factors_request_tx->update(slot);
	return nmea2000P->send_bypgn(PRIVATE_COMMAND_FACTORS_REQUEST, true);
}

bool
pilotParams::setSlot(int s)
{	
	if (s == wxNOT_FOUND)
		return false;
	if (slot != s) {
		int oldslot = slot;
		slot = s;
		if (!requestSlot(s)) {
			std::cerr << "setSlot " << slot <<
			    ": requestSlot failed, back to "
			    << oldslot <<  std::endl;
			slot = oldslot;
			return false;
		}
	}
	return true;
}

void
pilotParams::setValues(int s, int v[3])
{
	if (s != slot)
		return;
	paramsGroups->SetSelection(s);
	param_p->SetValue(v[0]);
	param_i->SetValue(v[1]);
	param_d->SetValue(v[2]);
}
