/*
 * MiniConsole.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "MiniConsole.h"
#include "../Helper/PeakMeter.h"
#include "../../Audio/AudioOutput.h"

MiniConsole::MiniConsole(AudioOutput *_output)
{
	output = _output;
	AddControlTable("!noexpandx", 0, 0, 5, 1, "grid");
	SetTarget("grid", 0);
	AddButton("Test", 0, 0, 0, 0, "test");
	AddDrawingArea("!width=100,height=30,noexpandx,noexpandy", 1, 0, 0, 0, "peaks");


	peak_meter = new PeakMeter(this, "peaks", output);
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

