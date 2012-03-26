/*
 * PeakMeter.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PeakMeter.h"

PeakMeter::PeakMeter(CHuiWindow *_win, const string &_id)
{
	win = _win;
	id = _id;
	peak_r = 0;
	peak_l = 0;

	win->EventMX(id, "hui:redraw", this, (void(HuiEventHandler::*)())&PeakMeter::OnDraw);
}

PeakMeter::~PeakMeter()
{
}

void PeakMeter::Set(float _peak_r, float _peak_l)
{
	peak_r = _peak_r;
	peak_l = _peak_l;

	win->Redraw(id);
}

void PeakMeter::OnDraw()
{
	HuiDrawingContext *c = win->BeginDraw(id);
	//c->DrawStr(0, 0, f2s(peak_r, 3));
	int w = c->width;
	int h = c->height;
	c->SetColor(Black);
	c->SetColor(White);
	//c->DrawRect(0, 0, w, h);
	c->DrawRect(2, 2, w-4, h/2 - 2);
	c->DrawRect(2, h/2 + 2, w-4, h - 2);
	c->SetColor(SetColorHSB(1, (1 - peak_r) * 0.33f, 1, 1));
	c->DrawRect(2, 2,      (int)((float)(w - 4) * pow(peak_r, 0.5)), h/2 - 2);
	c->DrawRect(2, h/2 + 2, (int)((float)(w - 4) * pow(peak_l, 0.5)), h - 2);

	c->End();
}
