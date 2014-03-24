/*
 * MiniConsole.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "MiniConsole.h"
#include "BottomBar.h"
#include "../Helper/PeakMeter.h"
#include "../../Audio/AudioOutput.h"

MiniConsole::MiniConsole(AudioOutput *_output)
{
	output = _output;
	AddControlTable("!noexpandx", 0, 0, 5, 1, "grid");
	SetTarget("grid", 0);
	AddButton("Test", 0, 0, 0, 0, "test");
	AddDrawingArea("!width=100,height=30,noexpandx,noexpandy", 1, 0, 0, 0, "peaks");
	AddButton(_("Mischpult"), 2, 0, 0, 0, "show_mixing_console");
	AddButton(_("Effekte"), 3, 0, 0, 0, "show_fx_console");

	peak_meter = new PeakMeter(this, "peaks", output);

	EventM("show_mixing_console", this, &MiniConsole::OnShowMixingConsole);
	EventM("show_fx_console", this, &MiniConsole::OnShowFxConsole);
}

MiniConsole::~MiniConsole()
{
}

void MiniConsole::OnShow()
{
	peak_meter->Enable(true);
}

void MiniConsole::OnHide()
{
	peak_meter->Enable(false);
}

void MiniConsole::OnShowFxConsole()
{
	((BottomBar*)parent)->Choose(BottomBar::FX_CONSOLE);
}

void MiniConsole::OnShowMixingConsole()
{
	((BottomBar*)parent)->Choose(BottomBar::MIXING_CONSOLE);
}

