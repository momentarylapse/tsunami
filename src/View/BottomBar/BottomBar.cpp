/*
 * BottomBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "MiniConsole.h"
#include "MixingConsole.h"
#include "CurveConsole.h"
#include "FxConsole.h"
#include "SynthConsole.h"
#include "LogDialog.h"
#include "SampleManager.h"
#include "../../lib/hui/Controls/HuiControl.h"

BottomBar::BottomBar(AudioView *view, AudioFile *audio, AudioOutput *output, Log *log) :
	Observable("BottomBar")
{
	AddControlTable("!noexpandy,height=300", 0, 0, 3, 1, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("", 0, 0, 1, 4, "button_grid");
	AddSeparator("!vertical", 1, 0, 0, 0, "");
	AddControlTable("", 2, 0, 1, 20, "console_grid");
	SetTarget("button_grid", 0);
	AddButton("!noexpandy,flat", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddButton("!noexpandy,flat", 0, 1, 0, 0, "choose");
	SetImage("choose", "hui:forward");
	AddText("!big,angle=90,expandy\\...", 0, 2, 0, 0, "title");
	fx_console = new FxConsole(view, audio);
	synth_console = new SynthConsole(view, audio);
	mixing_console = new MixingConsole(audio, output);
	curve_console = new CurveConsole(view, audio);
	sample_manager = new SampleManager(audio);
	log_dialog = new LogDialog(log);
	Embed(mixing_console, "console_grid", 0, 0);
	Embed(fx_console, "console_grid", 0, 1);
	Embed(synth_console, "console_grid", 0, 2);
	Embed(sample_manager, "console_grid", 0, 3);
	Embed(curve_console, "console_grid", 0, 4);
	Embed(log_dialog, "console_grid", 0, 5);

	menu = new HuiMenu;
	foreachi(HuiPanel *p, children, i){
		string id = "bottom_bar_choose_" + i2s(i);
		menu->AddItemCheckable(((BottomBarConsole*)p)->title, id);
		EventM(id, (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnChooseByMenu);
	}

	EventM("choose", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnOpenChooseMenu);
	EventM("close", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnClose);

	visible = true;
	Choose(MIXING_CONSOLE);
}

BottomBar::~BottomBar()
{
}

void BottomBar::OnClose()
{
	Hide();
}

void BottomBar::OnOpenChooseMenu()
{
	foreachi(HuiControl *c, menu->item, i)
		c->Check(i == active_console);
	menu->OpenPopup(this, 0, 0);
}

void BottomBar::OnChooseByMenu()
{
	Choose(HuiGetEvent()->id.tail(1)._int());
}

void BottomBar::OnShow()
{
	visible = true;
	Notify();
}

void BottomBar::OnHide()
{
	visible = false;
	Notify();
}

void BottomBar::Choose(int console)
{
	foreachi(HuiPanel *p, children, i){
		if (i == console){
			SetString("title", "!big\\" + ((BottomBarConsole*)p)->title);
			p->Show();
		}else
			p->Hide();
	}
	active_console = console;
	if (visible)
		Notify();
	else
		Show();
}

bool BottomBar::IsActive(int console)
{
	return (active_console == console) && visible;
}

