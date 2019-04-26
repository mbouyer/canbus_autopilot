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

#include <wxpilot.h>
#include <wx/combobox.h>
#include <widgets/wxNumberControl.h>
#include <N2K/nmea2000_defs_tx.h>
#define NPARAMS 6

class pilotParams: public wxPanel
{
  public:
	pilotParams(wxWindow *parent, wxWindowID id=wxID_ANY);
	void setValues(int, int values[NFACTORS]);
	bool setSlot(int slot);
	void update(void);
  private:
	wxBoxSizer *mainsizer;
	wxBoxSizer *controlsizer;
	wxFlexGridSizer *inputsizer;
	wxComboBox *paramsGroups;
	wxArrayString *paramsGroupsList;
	wxButton *applyButton;
	wxButton *resetButton;

	wxNumberControl *param[NFACTORS];

	private_command_factors_tx *command_factors_tx;
	private_command_factors_request_tx *command_factors_request_tx;

	int values[NFACTORS];
	int slot;

	bool requestSlot(int);
	void OnParamGroupSelect(wxCommandEvent & event);
	void OnApply(wxCommandEvent & event);
	void OnReset(wxCommandEvent & event);
};
