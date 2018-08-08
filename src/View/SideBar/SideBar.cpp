/*
 * SideBar.cpp
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#include "SideBar.h"
#include "TrackConsole.h"
#include "FxConsole.h"
#include "CurveConsole.h"
#include "SynthConsole.h"
#include "MidiFxConsole.h"
#include "../AudioView.h"
#include "MidiEditorConsole.h"
#include "SampleManagerConsole.h"
#include "SampleRefConsole.h"
#include "SongConsole.h"
#include "CaptureConsole.h"
#include "ModuleConsole.h"
#include "../../Session.h"

const int WIDTH_DEFAULT = 380;
const int WIDTH_LARGE = 750;

SideBar::SideBar(Session *session)
{
	addRevealer("!slide-left", 0, 0, "revealer");
	setTarget("revealer");
	addGrid("!noexpandx,width=380,expandy", 0, 0, "root_grid0");
	setTarget("root_grid0");
	addSeparator("!vertical,expandy", 0, 0, "");
	addGrid("!expandx,expandy,margin-right=5,margin-bottom=5", 1, 0, "root_grid");
	setTarget("root_grid");
	addGrid("", 0, 0, "button_grid");
	addSeparator("", 0, 1, "");
	addGrid("", 0, 2, "console_grid");
	setTarget("button_grid");
	addButton("!noexpandx,flat", 0, 0, "close");
	setImage("close", "hui:close");
	addButton("!noexpandx,flat", 1, 0, "large");
	setImage("large", "hui:up");
	addLabel("!big,bold,expandx,center\\...", 2, 0, "title");

	is_large = false;
	hideControl("large", true);

	song_console = new SongConsole(session);
	sample_manager = new SampleManagerConsole(session);
	//global_fx_console = new FxConsole(session);
	track_console = new TrackConsole(session);
	midi_editor_console = new MidiEditorConsole(session);
	fx_console = new FxConsole(session);
	curve_console = new CurveConsole(session);
	synth_console = new SynthConsole(session);
	midi_fx_console = new MidiFxConsole(session);
	sample_ref_console = new SampleRefConsole(session);
	capture_console = new CaptureConsole(session);
	module_console = new ModuleConsole(session);

	add_console(song_console);
	add_console(sample_manager);
	//add_console(global_fx_console);
	add_console(track_console);
	add_console(midi_editor_console);
	add_console(fx_console);
	add_console(curve_console);
	add_console(synth_console);
	add_console(midi_fx_console);
	add_console(sample_ref_console);
	add_console(capture_console);
	add_console(module_console);

	event("close", std::bind(&SideBar::on_close, this));
	event("large", std::bind(&SideBar::on_large, this));

	reveal("revealer", false);
	visible = false;
	active_console = -1;

	subscribe(session->view, std::bind(&AudioView::onUpdate, session->view)); // EVIL HACK?!?
}

SideBar::~SideBar()
{
}

void SideBar::add_console(SideBarConsole *c)
{
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	c->hide();
}

void SideBar::on_close()
{
	_hide();
}

void SideBar::on_large()
{
	set_large(!is_large);
}

void SideBar::set_large(bool large)
{
	if (large == is_large)
		return;
	is_large = large;
	if (is_large){
		setOptions("root_grid0", format("width=%d", WIDTH_LARGE));
		setImage("large", "hui:down");
	}else{
		setOptions("root_grid0", format("width=%d", WIDTH_DEFAULT));
		setImage("large", "hui:up");
	}
	hideControl("large", !is_large);
	if (active_console >= 0)
		consoles[active_console]->on_set_large(is_large);

}

void SideBar::_show()
{
	if ((!visible) and (active_console >= 0))
		consoles[active_console]->on_enter();

	reveal("revealer", true);
	visible = true;
	notify();
}

void SideBar::_hide()
{
	if ((visible) and (active_console >= 0))
		consoles[active_console]->on_leave();

	reveal("revealer", false);
	visible = false;
	notify();
}

void SideBar::choose(int console)
{
	if (active_console >= 0){
		if (visible)
			consoles[active_console]->on_leave();
		consoles[active_console]->hide();
	}
	set_large(false);

	active_console = console;

	consoles[active_console]->show();
	if (visible)
		consoles[active_console]->on_enter();
	setString("title", consoles[active_console]->title);

	notify();
}

void SideBar::open(int console)
{
	choose(console);

	if (!visible)
		_show();
	notify();
}

bool SideBar::is_active(int console)
{
	return (active_console == console) and visible;
}



SideBarConsole::SideBarConsole(const string &_title, Session *_session)
{
	title = _title;
	session = _session;
	song = session->song;
	view = session->view;
}

