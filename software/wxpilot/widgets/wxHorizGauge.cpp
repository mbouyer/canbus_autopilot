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
#include "wxHorizGauge.h"

wxHorizGauge::wxHorizGauge(wxWindow *parent, wxWindowID id, int r, int h) :
	wxWindow(parent, id, wxDefaultPosition, wxSize(-1, h), wxBORDER_NONE),
	range(r),
	height(h)
{
	value = 0;
	valid = 0;
	Connect(wxEVT_PAINT, wxPaintEventHandler(wxHorizGauge::OnPaint));
	Connect(wxEVT_SIZE, wxSizeEventHandler(wxHorizGauge::OnSize));
}

void wxHorizGauge::OnPaint(wxPaintEvent& event)
{
	wxSize size = GetSize();
	wxPaintDC dc(this);

	int width = size.GetWidth();
	int pwidth = width / 2;
	double pxperunit = (double)width / 2 / range;


	if (!valid) {
		/* black background, no info */
		dc.SetPen(wxPen(wxColour(0, 0, 0))); 
		dc.SetBrush(wxBrush(wxColour(0, 0, 0)));
		dc.DrawRectangle(0, 0, width, height);
		return;
	}

	// background
	dc.SetPen(wxPen(wxColour(255, 255, 255))); 
	dc.SetBrush(wxBrush(wxColour(255, 255, 255)));
	dc.DrawRectangle(0, 0, width, height);

	// middle marker
	dc.SetPen(wxPen(wxColour(90, 80, 60)));
	dc.DrawLine(width / 2, 0, width / 2, height);

	// draw marks
	dc.SetPen(wxPen(wxColour(0, 0, 0)));
	for (int i = -range; i <= range; i += 10) {
		int x = (width / 2) + i * pxperunit;
		dc.DrawLine(x, 0, x, height / 2);
	}
	int l = value * pxperunit;
	if (value < 0) {
		l = -l;
		dc.SetPen(wxPen(wxColour(240, 0, 0))); 
		dc.SetBrush(wxBrush(wxColour(240, 0, 0)));
		dc.DrawRectangle(width / 2 - l, height / 2, l, height / 2);
	} else {
		dc.SetPen(wxPen(wxColour(0, 240, 0))); 
		dc.SetBrush(wxBrush(wxColour(0, 240, 0)));
		dc.DrawRectangle(width / 2, height / 2, l, height / 2);
	}
}

void wxHorizGauge::OnSize(wxSizeEvent& event)
{
	Refresh();
}

void
wxHorizGauge::SetGauge(int n, bool v)
{
	if (value != n || valid != v) {
		valid = v;
		value = n;
		Refresh();
	}
}
