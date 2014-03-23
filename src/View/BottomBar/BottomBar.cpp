/*
 * BottomBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "MiniConsole.h"
#include "MixingConsole.h"
#include "FxConsole.h"

BottomBar::BottomBar(AudioFile *audio, AudioOutput *output) :
	Observable("BottomBar")
{
	AddControlTable("!noexpandy", 0, 0, 1, 3, "grid");
	mini_console = new MiniConsole;
	fx_console = new FxConsole(audio);
	mixing_console = new MixingConsole(audio, output);
	Embed(mini_console, "grid", 0, 0);
	Embed(fx_console, "grid", 0, 1);
	Embed(mixing_console, "grid", 0, 2);

	Choose(MINI_CONSOLE);
}

BottomBar::~BottomBar()
{
}

void BottomBar::Choose(int console)
{
	foreachi(HuiPanel *p, children, i){
		if (i == console)
			p->Show();
		else
			p->Hide();
	}
	active_console = console;
	Notify("Change");
}

void BottomBar::SetTrack(Track *t)
{
	fx_console->SetTrack(t);
}

