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

#ifndef _WXPILOT_H_
#define _WXPILOT_H_

/* Keep in sync with PRIVATE_COMMAND_FACTORS PGN definition */
#define FACTOR_ERR 0
#define FACTOR_DIF 1
#define FACTOR_DIF2 2
#define NFACTORS  3

class PilotFrame;

class wxpilot : public wxApp
{
  public:
	virtual bool OnInit();
	static wxString AppName();
	static wxString ErrMsgPrefix();
	void setAtttitude(double, bool);
	void setRot(double, bool);
	void setPilotHeading(double, bool);
	void setRudder(double, bool);
	void setStatus(int, int, const wxString &);
	void setFactors(int, int[NFACTORS]);
	void setRadio(int, int, int);
  private:
	PilotFrame *frame;
};

extern wxpilot *wxp;

#endif /* _WXPILOT_H_ */
