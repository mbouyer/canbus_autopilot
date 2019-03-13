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
#include "wxCompass.h"

wxCompass::wxCompass(wxWindow *parent, wxWindowID id) :
	wxWindow(parent, id, wxDefaultPosition, wxSize(-1, 30), wxBORDER_NONE)
{
	heading = 0;
	headingV = 0;
	targetHeading = 0;
	targetHeadingV = 0;
	rot = 0;
	rotV = 0;
	Connect(wxEVT_PAINT, wxPaintEventHandler(wxCompass::OnPaint));
	Connect(wxEVT_SIZE, wxSizeEventHandler(wxCompass::OnSize));
}

int
wxCompass::normDeg(int deg)
{
	if (deg < 0)
		return deg + 360;
	if (deg >= 360)
		return deg - 360;
	return deg;
}

void wxCompass::OnPaint(wxPaintEvent& event)
{
	wxSize size = GetSize();
	wxFont font(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
	    wxFONTWEIGHT_NORMAL, false, wxT("Courier 10 Pitch"));
	wxPaintDC dc(this);
	dc.SetFont(font);
	int pxPerMark = dc.GetTextExtent("8888").GetWidth() / 2;
	const int DegsPerMark = 5;

	int width = size.GetWidth();
	int nummarks = width / pxPerMark + 1;
	int pwidth = width;


	if (nummarks * DegsPerMark > 360) {
		nummarks = 360 / DegsPerMark;
		pwidth = nummarks * pxPerMark;
	}

	if (!headingV) {
		/* black background, no info */
		dc.SetPen(wxPen(wxColour(0, 0, 0))); 
		dc.SetBrush(wxBrush(wxColour(0, 0, 0)));
		dc.DrawRectangle((width - pwidth) / 2, 0, pwidth, 30);
		return;
	}

	// background
	dc.SetPen(wxPen(wxColour(255, 255, 184))); 
	dc.SetBrush(wxBrush(wxColour(255, 255, 184)));
	dc.DrawRectangle((width - pwidth) / 2, 0, pwidth, 30);

	// middle marker
	dc.SetPen(wxPen(wxColour(90, 80, 60)));
	dc.DrawLine(width / 2, 0, width / 2, 30);

	// draw compass marks
	dc.SetPen(wxPen(wxColour(0, 0, 0)));
	for (int i = -nummarks / 2; i <= nummarks / 2; i++) {
		int deg = normDeg((heading / DegsPerMark + i) * DegsPerMark);
		int x = (width / 2) + i * pxPerMark - ((heading % DegsPerMark) * pxPerMark) / DegsPerMark;
		int h;
		if ((deg % 10) == 0) {
			h = 8;
			wxString t = wxString::Format(wxT("%d"), deg);
			int tsize = dc.GetTextExtent(t).GetWidth();
			dc.DrawText(t, x - tsize / 2, 10);
		} else {
			h = 4;
		}
		dc.DrawLine(x, 0, x, h);
	}
	// rate of turn indicator
	// +/- 10 deg full scale
	const int rotScale = 10;
	if (rotV) {
		double lrot = rot;
		if (lrot < -rotScale)
			lrot = -rotScale;
		if (lrot > rotScale)
			lrot = rotScale;
		dc.SetPen(wxPen(wxColour(0, 0, 100))); 
		dc.SetBrush(wxBrush(wxColour(0, 0, 100)));
		int x = (width / 2) + lrot * ( pwidth / 2) / rotScale;
		if (lrot < 0)
			dc.DrawLine(x, 25, width / 2, 25);
		else
			dc.DrawLine(width / 2, 25, x, 25);
		dc.DrawCircle(x, 25, 3);
	}
	if (targetHeadingV) {
		wxPoint m[] = { {0, 0}, {4, 8}, {-4, 8}, {0, 0} };
		wxPoint ml[] = { {0, 4}, {8, 0}, {8, 8}, {0, 4} };
		wxPoint mr[] = { {0, 4}, {-8, 0}, {-8, 8}, {0, 4} };
		int left = normDeg(heading - targetHeading);
		int right = normDeg(targetHeading - heading);
		if (left < right) {
			if (left > nummarks * DegsPerMark / 2) {
				dc.SetPen(wxPen(wxColour(240, 0, 0))); 
				dc.DrawLines(4, ml, (width - pwidth) / 2, 5);
			} else {
				int x = (width / 2) - (left * pxPerMark / DegsPerMark);
				dc.SetPen(wxPen(wxColour(0, 150, 0))); 
				dc.DrawLines(4, m, x, 5);
			}
		} else {
			if (right > nummarks * DegsPerMark / 2) {
				dc.SetPen(wxPen(wxColour(240, 0, 0))); 
				dc.DrawLines(4, mr, (width + pwidth) / 2, 5);
			} else {
				int x = (width / 2) + (right * pxPerMark / DegsPerMark);
				dc.SetPen(wxPen(wxColour(0, 150, 0))); 
				dc.DrawLines(4, m, x, 5);
			}
		}
	}
}

void wxCompass::OnSize(wxSizeEvent& event)
{
	Refresh();
}

void
wxCompass::SetHeading(int n, bool valid)
{
	if (heading != n || headingV != valid) {
		headingV = valid;
		heading = n;
		Refresh();
	}
}

void
wxCompass::SetTargetHeading(int n, bool valid)
{
	if (targetHeading != n || targetHeadingV != valid) {
		targetHeadingV = valid;
		targetHeading = n;
		Refresh();
	}
}

void
wxCompass::SetRot(double n, bool valid)
{
	if (rot != n || rotV != valid) {
		rotV = valid;
		rot = n;
		Refresh();
	}
}
