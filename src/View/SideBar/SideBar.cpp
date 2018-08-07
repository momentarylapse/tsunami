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
	addLabel("!big,expandx,center\\...", 1, 0, "title");

	song_console = new SongConsole(session);
	sample_manager = new SampleManagerConsole(session);
	global_fx_console = new FxConsole(session);
	track_console = new TrackConsole(session);
	midi_editor_console = new MidiEditorConsole(session);
	fx_console = new FxConsole(session);
	curve_console = new CurveConsole(session);
	synth_console = new SynthConsole(session);
	midi_fx_console = new MidiFxConsole(session);
	sample_ref_console = new SampleRefConsole(session);
	capture_console = new CaptureConsole(session);
	module_console = new ModuleConsole(session);

	addConsole(song_console);
	addConsole(sample_manager);
	addConsole(global_fx_console);
	addConsole(track_console);
	addConsole(midi_editor_console);
	addConsole(fx_console);
	addConsole(curve_console);
	addConsole(synth_console);
	addConsole(midi_fx_console);
	addConsole(sample_ref_console);
	addConsole(capture_console);
	addConsole(module_console);

	event("close", std::bind(&SideBar::onClose, this));

	reveal("revealer", false);
	visible = false;
	active_console = -1;

	subscribe(session->view, std::bind(&AudioView::onUpdate, session->view)); // EVIL HACK?!?
}

SideBar::~SideBar()
{
}

void SideBar::addConsole(SideBarConsole *c)
{
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	c->hide();
}

void SideBar::onClose()
{
	_hide();
}

void SideBar::_show()
{
	if ((!visible) and (active_console >= 0))
		consoles[active_console]->onEnter();

	reveal("revealer", true);
	visible = true;
	notify();
}

void SideBar::_hide()
{
	if ((visible) and (active_console >= 0))
		consoles[active_console]->onLeave();

	reveal("revealer", false);
	visible = false;
	notify();
}

void SideBar::choose(int console)
{
	if (active_console >= 0){
		if (visible)
			consoles[active_console]->onLeave();
		consoles[active_console]->hide();
	}

	active_console = console;

	consoles[active_console]->show();
	if (visible)
		consoles[active_console]->onEnter();
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

bool SideBar::isActive(int console)
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

