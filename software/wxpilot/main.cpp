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
#include <wx/config.h>
#include "wxpilot.h"
#include "pilotparams.h"
#include "pilotstatus.h"
#include <N2K/NMEA2000.h>
#include <N2K/NMEA2000Properties.h>
#include <N2K/NMEA2000PropertiesDialog.h>
#include <widgets/wxCompass.h>
#include <widgets/wxHorizGauge.h>

const int NUMBER_UPDATE_ID = 100000;
const int myID_F_N2KCONF =	10;
const int myID_DATAUP =		100;

class PilotFrame : public wxFrame
{
public:
	PilotFrame(const wxString& title);
	typedef enum dataup {
		data_nav = 0,
		data_params,
		data_status,
	} dataup_t; 
	void wake(dataup_t);
	double boatHeading;
	bool boatHeadingV;
	double pilotHeading;
	bool pilotHeadingV;
	double boatRot;
	bool boatRotV;
	double rudder;
	bool rudderV;
	int addr;
	int group;
	wxString mode;
	int paramslot;
	int paramvalues[3];

private:
	wxConfig *config;
	wxPanel *mainpanel;
	wxBoxSizer *mainsizer;
	wxMenuBar *menubar;
	wxMenu *file;
	wxCompass *compass;
	wxHorizGauge *rudderW;
	pilotParams *pilotparams;
	pilotStatus *pilotstatus;

	void OnN2KConfig(wxCommandEvent & event);
	void OnDataUpdate(wxCommandEvent & event);
	void OnQuit(wxCommandEvent & event);
	void OnClose(wxCloseEvent & event);
};

PilotFrame::PilotFrame(const wxString& title)
	: wxFrame(NULL, wxID_ANY, title)
{
	int x, y, w, h;
	config = new wxConfig(wxpilot::AppName());
	if (config) {
		x = config->ReadLong("/Position/X", -1);
		y = config->ReadLong("/Position/Y", -1);
		w = config->ReadLong("/Position/W", -1);
		h = config->ReadLong("/Position/H", -1);
	} else {
		x = y = w = h = -1;
	}
	boatHeadingV = pilotHeadingV = boatRotV = rudderV = false;
	nmea2000P = new nmea2000;
	NMEA2000PropertiesP = new NMEA2000Properties(config);
	//mainpanel = new wxPanel(this, wxID_ANY);
	mainsizer = new wxBoxSizer( wxVERTICAL );
	menubar = new wxMenuBar;
	file = new wxMenu;
	file->Append(myID_F_N2KCONF, _T("N2k Config"));
	file->Append(wxID_EXIT, _T("&Quit"));
	menubar->Append(file, _T("&File"));
	SetMenuBar(menubar);
	Connect(wxEVT_CLOSE_WINDOW,
		wxCloseEventHandler(PilotFrame::OnClose));
	Connect(myID_F_N2KCONF, wxEVT_COMMAND_MENU_SELECTED,
		wxCommandEventHandler(PilotFrame::OnN2KConfig));
	Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
		wxCommandEventHandler(PilotFrame::OnQuit));
	Connect(myID_DATAUP, wxEVT_COMMAND_TEXT_UPDATED,
		wxCommandEventHandler(PilotFrame::OnDataUpdate));

	compass = new wxCompass(this);
	mainsizer->Add( compass, 0, wxEXPAND | wxALL, 5 );

	rudderW = new wxHorizGauge(this, wxID_ANY);
	mainsizer->Add( rudderW, 0, wxEXPAND | wxALL, 5 );

	pilotstatus = new pilotStatus(this);
	mainsizer->Add( pilotstatus, 0, wxEXPAND | wxALL, 5 );

	pilotparams = new pilotParams(this);
	mainsizer->Add( pilotparams, 0, wxEXPAND | wxALL, 5 );

	SetSizerAndFit(mainsizer);
	this->SetSize(x, y, w, h);
}

void PilotFrame::OnN2KConfig(wxCommandEvent & WXUNUSED(event))
{
	NMEA2000PropertiesDialog *N2KPropDialog = new NMEA2000PropertiesDialog(this);
	N2KPropDialog->ShowModal();
	delete N2KPropDialog;

}

void PilotFrame::wake(dataup_t t)
{
	wxCommandEvent event(wxEVT_COMMAND_TEXT_UPDATED, myID_DATAUP);
	event.SetInt(t);
	GetEventHandler()->AddPendingEvent( event );
}

void PilotFrame::OnDataUpdate(wxCommandEvent & event)
{
	dataup_t t;
	t = (dataup_t)event.GetInt();
	switch(t) {
	case PilotFrame::dataup_t::data_nav:
		compass->SetHeading(boatHeading, boatHeadingV);
		compass->SetTargetHeading(pilotHeading, pilotHeadingV);
		compass->SetRot(boatRot, boatRotV);
		rudderW->SetGauge(rudder, rudderV);
		break;
	case PilotFrame::dataup_t::data_status:
		pilotstatus->address(addr);
		pilotstatus->group(group);
		pilotstatus->status(mode);
		pilotparams->setSlot(group);
		break;
	case PilotFrame::dataup_t::data_params:
		pilotparams->setValues(paramslot, paramvalues);
		break;
	}
}

void PilotFrame::OnQuit(wxCommandEvent & WXUNUSED(event))
{
	Close(true);
}

void PilotFrame::OnClose(wxCloseEvent & event)
{
	if (config) {
		int x, y, w, h;
		GetClientSize(&w, &h);
		GetPosition(&x, &y);
		config->Write(_T("/Position/x"), (long) x);
		config->Write(_T("/Position/y"), (long) y);
		config->Write(_T("/Position/w"), (long) w);
		config->Write(_T("/Position/h"), (long) h);
		config->Flush();
		std::cout <<  "saved config file ... " << std::endl;
	}
	delete nmea2000P;
	std::cout <<  "Exiting ... " << std::endl;
	event.Skip();
}

IMPLEMENT_APP(wxpilot)

wxpilot *wxp;

bool wxpilot::OnInit()
{
	wxp = this;
	frame = new PilotFrame(AppName());
	frame->Show(true);
	nmea2000P->Init();

	return true;
}

#define APPNAME "wxpilot"

wxString wxpilot::AppName() 
{
	static wxString s(APPNAME);
	return s;
}
wxString wxpilot::ErrMsgPrefix() 
{
	static wxString s(APPNAME ": ");
	return s;
}

void wxpilot::setAtttitude(double d, bool valid)
{
	frame->boatHeadingV = valid;
	if (valid)
		frame->boatHeading = d;
	frame->wake(PilotFrame::dataup_t::data_nav);
}

void wxpilot::setRot(double d, bool valid)
{
	frame->boatRotV = valid;
	if (valid)
		frame->boatRot = d;
	frame->wake(PilotFrame::dataup_t::data_nav);
}

void wxpilot::setPilotHeading(double d, bool valid)
{
	frame->pilotHeadingV = valid;
	if (valid)
		frame->pilotHeading = d;
	frame->wake(PilotFrame::dataup_t::data_nav);
}

void wxpilot::setRudder(double d, bool valid)
{
	frame->rudderV = valid;
	if (valid)
		frame->rudder = d;
	frame->wake(PilotFrame::dataup_t::data_nav);
}

void wxpilot::setStatus(int addr, int group, const wxString &mode)
{
	frame->addr = addr;
	frame->group = group;
	frame->mode = mode;
	frame->wake(PilotFrame::dataup_t::data_status);
}

void wxpilot::setFactors(int slot, int values[3])
{
	frame->paramslot = slot;
	for (int i = 0; i < 3; i++)
		frame->paramvalues[i] = values[i];
	frame->wake(PilotFrame::dataup_t::data_params);
}
