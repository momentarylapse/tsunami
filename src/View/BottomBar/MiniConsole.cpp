/*
 * MiniConsole.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "MiniConsole.h"
#include "BottomBar.h"
#include "../Helper/PeakMeter.h"
#include "../../Audio/AudioStream.h"

MiniConsole::MiniConsole(AudioStream *_stream)
{
	stream = _stream;
	AddControlTable("!noexpandx", 0, 0, 5, 1, "grid");
	SetTarget("grid", 0);
	AddButton("Test", 0, 0, 0, 0, "test");
	AddDrawingArea("!width=100,height=30,noexpandx,noexpandy", 1, 0, 0, 0, "peaks");
	AddButton(_("Mischpult"), 2, 0, 0, 0, "show_mixing_console");
	AddButton(_("Effekte"), 3, 0, 0, 0, "show_fx_console");

	peak_meter = new PeakMeter(this, "peaks", stream);

	EventM("show_mixing_console", this, &MiniConsole::onShowMixingConsole);
	EventM("show_fx_console", this, &MiniConsole::onShowFxConsole);
}

MiniConsole::~MiniConsole()
{
}

void MiniConsole::onShow()
{
	peak_meter->enable(true);
}

void MiniConsole::onHide()
{
	peak_meter->enable(false);
}

void MiniConsole::onShowFxConsole()
{
	((BottomBar*)parent)->choose(BottomBar::FX_CONSOLE);
}

void MiniConsole::onShowMixingConsole()
{
	((BottomBar*)parent)->choose(BottomBar::MIXING_CONSOLE);
}

