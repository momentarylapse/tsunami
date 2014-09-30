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
#include "MidiEditor.h"
#include "../../lib/hui/Controls/HuiControl.h"
#include "../AudioView.h"

BottomBar::BottomBar(AudioView *view, AudioFile *audio, AudioOutput *output, Log *log) :
	Observable("BottomBar")
{
	AddControlTable("!noexpandy,height=300", 0, 0, 3, 1, "root_grid");
	SetTarget("root_grid", 0);
	AddControlTable("!noexpandx,width=130", 0, 0, 1, 2, "button_grid");
	AddSeparator("!vertical", 1, 0, 0, 0, "");
	AddControlTable("", 2, 0, 1, 20, "console_grid");
	SetTarget("button_grid", 0);
	AddButton("!noexpandy,flat", 0, 0, 0, 0, "close");
	SetImage("close", "hui:close");
	AddListView("!nobar\\name", 0, 1, 0, 0, "choose");
	fx_console = new FxConsole(view, audio);
	synth_console = new SynthConsole(view, audio);
	mixing_console = new MixingConsole(audio, output);
	curve_console = new CurveConsole(view, audio);
	sample_manager = new SampleManager(audio);
	log_dialog = new LogDialog(log);
	midi_editor = new MidiEditor(view, audio);
	Embed(mixing_console, "console_grid", 0, 0);
	Embed(fx_console, "console_grid", 0, 1);
	Embed(synth_console, "console_grid", 0, 2);
	Embed(midi_editor, "console_grid", 0, 3);
	Embed(sample_manager, "console_grid", 0, 4);
	Embed(curve_console, "console_grid", 0, 5);
	Embed(log_dialog, "console_grid", 0, 6);

	view->Subscribe(this);

	//menu = new HuiMenu;
	foreachi(HuiPanel *p, children, i){
		AddString("choose", ((BottomBarConsole*)p)->title);
		//string id = "bottom_bar_choose_" + i2s(i);
		//menu->AddItemCheckable(((BottomBarConsole*)p)->title, id);
		//EventM(id, (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnChooseByMenu);
	}

	EventMX("choose", "hui:select", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::OnChoose);
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

/*void BottomBar::OnOpenChooseMenu()
{
	foreachi(HuiControl *c, menu->item, i)
		c->Check(i == active_console);
	menu->OpenPopup(this, 0, 0);
}

void BottomBar::OnChooseByMenu()
{
	Choose(HuiGetEvent()->id.tail(1)._int());
}*/

void BottomBar::OnChoose()
{
	int n = GetInt("");
	if (n >= 0)
		Choose(n);
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
	SetInt("choose", console);
	active_console = console;
	if (!visible)
		Show();
	Notify();
}

bool BottomBar::IsActive(int console)
{
	return (active_console == console) && visible;
}

