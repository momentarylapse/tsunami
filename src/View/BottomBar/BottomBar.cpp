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
	AddControlTable("!noexpandy,height=300", 0, 0, 3, 1, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 1, 3, "button_grid");
	AddSeparator("!vertical", 1, 0, 0, 0, "");
	AddControlTable("", 2, 0, 1, 20, "console_grid");
	SetTarget("button_grid", 0);
	AddButton("!noexpandy,flat", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddButton("!noexpandy,flat", 0, 1, 0, 0, "next");
	SetImage("next", "hui:forward");
	AddText("!big,bold,angle=90,expandy\\...", 0, 2, 0, 0, "title");
	fx_console = new FxConsole(audio);
	mixing_console = new MixingConsole(audio, output);
	Embed(mixing_console, "console_grid", 0, 0);
	Embed(fx_console, "console_grid", 0, 1);

	EventM("next", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnNext);
	EventM("close", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnClose);

	Choose(MIXING_CONSOLE);
	visible = true;
}

BottomBar::~BottomBar()
{
}

void BottomBar::OnClose()
{
	Hide();
}

void BottomBar::OnNext()
{
	Choose((active_console + 1) % 2);
}

void BottomBar::OnShow()
{
	visible = true;
	Notify("Change");
}

void BottomBar::OnHide()
{
	visible = false;
	Notify("Change");
}

void BottomBar::Choose(int console)
{
	foreachi(HuiPanel *p, children, i){
		if (i == console){
			SetString("title", "!big,bold\\" + ((BottomBarConsole*)p)->title);
			p->Show();
		}else
			p->Hide();
	}
	active_console = console;
	if (visible)
		Notify("Change");
	else
		Show();
}

bool BottomBar::IsActive(int console)
{
	return (active_console == console) && visible;
}

void BottomBar::SetTrack(Track *t)
{
	fx_console->SetTrack(t);
}

