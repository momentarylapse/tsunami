/*
 * BottomBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "MixingConsole.h"
#include "LevelConsole.h"
#include "CurveConsole.h"
#include "FxConsole.h"
#include "SynthConsole.h"
#include "LogDialog.h"
#include "SampleManager.h"
#include "MidiFxConsole.h"
#include "../../lib/hui/Controls/HuiControl.h"
#include "../AudioView.h"
#include "MiniBar.h"

BottomBar::BottomBar(AudioView *view, AudioFile *audio, AudioOutput *output, Log *log) :
	Observable("BottomBar")
{
	ready = false;
	console_when_ready = MIXING_CONSOLE;

	addRevealer("", 0, 0, 0, 0, "revealer");
	setTarget("revealer", 0);
	addGrid("!noexpandy,height=300,expandx", 0, 0, 1, 2, "root_grid0");
	setTarget("root_grid0", 0);
	addSeparator("!horizontal,expandx", 0, 0, 0, 0, "");
	addGrid("!expandx", 0, 1, 3, 1, "root_grid");
	setTarget("root_grid", 0);
	addGrid("!noexpandx,width=130", 0, 0, 1, 2, "button_grid");
	addSeparator("!vertical", 1, 0, 0, 0, "");
	addGrid("", 2, 0, 1, 20, "console_grid");
	setTarget("button_grid", 0);
	addButton("!noexpandy,flat", 0, 0, 0, 0, "close");
	setImage("close", "hui:close");
	addListView("!nobar\\name", 0, 1, 0, 0, "choose");
	log_dialog = new LogDialog(log);
	mixing_console = new MixingConsole(audio, output, view->stream);
	level_console = new LevelConsole(audio, view);
	fx_console = new FxConsole(NULL, audio);
	sample_manager = new SampleManager(audio);
	curve_console = new CurveConsole(view, audio);
	track_fx_console = new FxConsole(view, audio);
	track_synth_console = new SynthConsole(view, audio);
	track_midi_fx_console = new MidiFxConsole(view, audio);
	addConsole(log_dialog, "");
	addConsole(mixing_console, "");
	addConsole(level_console, "");
	addConsole(fx_console, "");
	addConsole(sample_manager, "");
	addConsole(curve_console, "");
	addConsole(track_fx_console, "\t");
	addConsole(track_synth_console, "\t");
	addConsole(track_midi_fx_console, "\t");

	view->subscribe(this);

	eventX("choose", "hui:select", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::onChoose);
	event("close", (HuiPanel*)this, (void(HuiPanel::*)())&BottomBar::onClose);

	visible = true;
	ready = true;
	HuiPanel::show();
	choose(console_when_ready);
	reveal("revealer", true);
}

BottomBar::~BottomBar()
{
}

void BottomBar::onClose()
{
	hide();
}

void BottomBar::show()
{
	HuiPanel::show();
	reveal("revealer", true);
	visible = true;
	notify();
}

void BottomBar::hide()
{
	reveal("revealer", false);
	visible = false;
	notify();
}

void BottomBar::addConsole(BottomBarConsole *c, const string &list_name)
{
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	addString("choose", list_name + c->title);
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

void BottomBar::onChoose()
{
	int n = getInt("");
	if (n >= 0)
		choose(n);
}

void BottomBar::onShow()
{
	visible = true;
	notify();
}

void BottomBar::onHide()
{
	visible = false;
	notify();
}

void BottomBar::choose(int console)
{
	if (!ready){
		console_when_ready = console;
		return;
	}

	foreachi(BottomBarConsole *c, consoles, i){
		if (i == console)
			c->show();
		else
			c->hide();
	}
	setInt("choose", console);
	active_console = console;
	if (!visible)
		show();
	notify();
}

bool BottomBar::isActive(int console)
{
	return (active_console == console) && visible;
}

