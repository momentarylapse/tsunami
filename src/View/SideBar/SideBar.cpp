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
#include "../../Session.h"
#include "../../TsunamiWindow.h"
#include "../BottomBar/BottomBar.h"

const int SideBar::WIDTH_DEFAULT = 380;
const int SideBar::WIDTH_LARGE = 750;

SideBar::SideBar(Session *_session)
{
	session = _session;
	add_revealer("!slide=left", 0, 0, "revealer");
	set_target("revealer");
	add_grid("!noexpandx,width=380,expandy", 0, 0, "root_grid0");
	set_target("root_grid0");
	add_separator("!vertical,expandy", 0, 0, "");
	add_grid("!expandx,expandy,margin-right=5,margin-bottom=5", 1, 0, "root_grid");
	set_target("root_grid");
	add_grid("", 0, 0, "button_grid");
	add_separator("", 0, 1, "");
	add_grid("", 0, 2, "console_grid");
	set_target("button_grid");
	add_button("!noexpandx,flat", 0, 0, "close");
	set_image("close", "hui:close");
	add_button("!noexpandx,flat", 1, 0, "large");
	set_image("large", "hui:up");
	add_label("!big,bold,expandx,center\\...", 2, 0, "title");

	is_large = false;
	hide_control("large", true);

	song_console = new SongConsole(session);
	sample_manager = new SampleManagerConsole(session);
	global_fx_console = new GlobalFxConsole(session);
	track_console = new TrackConsole(session);
	midi_editor_console = new MidiEditorConsole(session);
	fx_console = new FxConsole(session);
	curve_console = new CurveConsole(session);
	synth_console = new SynthConsole(session);
	midi_fx_console = new MidiFxConsole(session);
	sample_ref_console = new SampleRefConsole(session);
	capture_console = new CaptureConsole(session);

	add_console(song_console);
	add_console(sample_manager);
	add_console(global_fx_console);
	add_console(track_console);
	add_console(midi_editor_console);
	add_console(fx_console);
	add_console(curve_console);
	add_console(synth_console);
	add_console(midi_fx_console);
	add_console(sample_ref_console);
	add_console(capture_console);

	event("close", [=]{ on_close(); });
	event("large", [=]{ on_large(); });

	reveal("revealer", false);
	visible = false;
	active_console = -1;

	subscribe(session->view, [=]{ session->view->on_update(); }); // EVIL HACK?!?
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
	if (allow_close())
		session->set_mode("default");
	//_hide();
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
		set_options("root_grid0", format("width=%d", WIDTH_LARGE));
		set_image("large", "hui:down");
	}else{
		set_options("root_grid0", format("width=%d", WIDTH_DEFAULT));
		set_image("large", "hui:up");
	}
	hide_control("large", !is_large);
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

// FIXME: this is the official closing function...
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
	set_string("title", consoles[active_console]->title);

	notify();
}

void SideBar::open(int console)
{
	if (console == FX_CONSOLE){
		session->win->bottom_bar->open(BottomBar::FAKE_FX_CONSOLE);
		return;
	}
	if (console == MIDI_FX_CONSOLE){
		session->win->bottom_bar->open(BottomBar::FAKE_MIDI_FX_CONSOLE);
		return;
	}
	choose(console);

	if (!visible)
		_show();
	notify();
}

bool SideBar::is_active(int console)
{
	return (active_console == console) and visible;
}

bool SideBar::allow_close()
{
	if (!visible or active_console < 0)
		return true;
	return consoles[active_console]->allow_close();
}


SideBarConsole::SideBarConsole(const string &_title, Session *_session)
{
	title = _title;
	session = _session;
	song = session->song;
	view = session->view;
}

