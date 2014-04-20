/*
 * CurveConsole.cpp
 *
 *  Created on: 19.04.2014
 *      Author: michi
 */

#include "CurveConsole.h"

CurveConsole::CurveConsole(AudioView *_view, AudioFile *_audio) :
	BottomBarConsole(_("Kurven")),
	Observer("CurveConsole")
{
	view = _view;
	audio = _audio;

	AddControlTable("", 0, 0, 2, 1, "root");
	SetTarget("root", 0);
	AddDrawingArea("", 0, 0, 0, 0, "area");
	AddControlTable("", 1, 0, 1, 2, "controller");
	SetTarget("controller", 0);
	AddListView("name\\ziel", 0, 0, 0, 0, "list");
	AddButton("add", 0, 1, 0, 0, "add");
}

CurveConsole::~CurveConsole()
{
}

void CurveConsole::OnUpdate(Observable* o, const string &message)
{
}

