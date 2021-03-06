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
#include "pilotstatus.h"

pilotStatus::pilotStatus(wxWindow *parent, wxWindowID id)
	: wxPanel(parent, id)
{
	pilotsizer = new wxFlexGridSizer(2, 5, 5);

	wxSizerFlags datafl(0);
	datafl.Expand().Left();
	wxSizerFlags labelfl(0);
	labelfl.Right();

#define LABELTEXT(t) \
    new wxStaticText(this, -1, wxT(t))

	pilotsizer->Add(LABELTEXT("Address:"), labelfl);
	Taddr = new wxStaticText(this, -1, wxT(""));
	pilotsizer->Add(Taddr, datafl);

	pilotsizer->Add(LABELTEXT("Param Group:"), labelfl);
	Tgroup = new wxStaticText(this, -1, wxT(""));
	pilotsizer->Add(Tgroup, datafl);

	pilotsizer->Add(LABELTEXT("Mode:"), labelfl);
	wxSize txtsize = GetTextExtent(wxT("commande muette"));
	Tmode = new wxStaticText(this, -1, wxT(""),
	    wxDefaultPosition, txtsize, wxST_NO_AUTORESIZE);
	pilotsizer->Add(Tmode, datafl);

	radiosizer = new wxFlexGridSizer(2, 5, 5);

	radiosizer->Add(LABELTEXT("batt:"), labelfl);
	Rtxv = new wxStaticText(this, -1, wxT(""));
	radiosizer->Add(Rtxv, datafl);

	radiosizer->Add(LABELTEXT("state:"), labelfl);
	txtsize = GetTextExtent(wxT("0x00"));
	Rstate = new wxStaticText(this, -1, wxT(""),
	    wxDefaultPosition, txtsize, wxST_NO_AUTORESIZE);
	radiosizer->Add(Rstate, datafl);

	radiosizer->Add(LABELTEXT("RSSI:"), labelfl);
	Rrssi = new wxStaticText(this, -1, wxT(""));
	radiosizer->Add(Rrssi, datafl);

	wxSizerFlags pilotfl(0);
	pilotfl.Left();
	wxSizerFlags radiofl(0);
	radiofl.Right();

	mainsizer = new wxFlexGridSizer(2, 0, 0);
	mainsizer->Add(pilotsizer, pilotfl);
	mainsizer->Add(radiosizer, radiofl);

	SetSizerAndFit(mainsizer);
}

void
pilotStatus::address(int a)
{
	if (a == -1)
		Taddr->SetLabel(_T(""));
	else
		Taddr->SetLabel(wxString::Format(_T("%d"), a));
}

void
pilotStatus::group(int g)
{
	if (g == -1)
		Taddr->SetLabel(_T("   "));
	else
		Tgroup->SetLabel(wxString::Format(_T("%d"), g));
}

void
pilotStatus::status(const wxString &s)
{
	Tmode->SetLabel(s);
}

void
pilotStatus::radio(int txv, int state, int rssi)
{
	Rtxv->SetLabel(wxString::Format(_T("%d"), txv));
	Rstate->SetLabel(wxString::Format(_T("0x%x"), state));
	Rrssi->SetLabel(wxString::Format(_T("%d"), rssi));
}
