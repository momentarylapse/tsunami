/*
 * SideBar.cpp
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#include "SideBar.h"
#include "TrackConsole.h"
#include "CurveConsole.h"
#include "EffectsConsole.h"
#include "DummyEditorConsole.h"
#include "AudioEditorConsole.h"
#include "MidiEditorConsole.h"
#include "BarsEditorConsole.h"
#include "SampleManagerConsole.h"
#include "SampleRefConsole.h"
#include "SongConsole.h"
#include "CaptureConsole.h"
#include "SignalChainPanel.h"
#include "../audioview/AudioView.h"
#include "../bottombar/BottomBar.h"
#include "../helper/BreadCrumps.h"
#include "../TsunamiWindow.h"
#include "../../Session.h"
#include "../../EditModes.h"

namespace tsunami {

extern const int CONFIG_PANEL_WIDTH;

SideBar::SideBar(Session *_session, hui::Panel *parent) {
	set_parent(parent);
	set_id("side-bar");
	session = _session;
	add_expander("!slide=left", 0, 0, "revealer");
	set_target("revealer");
	add_grid(format("!noexpandx,width=%d,expandy", CONFIG_PANEL_WIDTH), 0, 0, "root_grid0");
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
	add_label("!huge,bold,expandx,center\\...", 2, 0, "title");

	hide_control("large", true);

	song_console = new SongConsole(session, this);
	sample_manager = new SampleManagerConsole(session, this);
	track_console = new TrackConsole(session, this);
	effects_console = new EffectsConsole(session, this);
	dummy_editor_console = new DummyEditorConsole(session, this);
	audio_editor_console = new AudioEditorConsole(session, this);
	midi_editor_console = new MidiEditorConsole(session, this);
	bars_editor_console = new BarsEditorConsole(session, this);
	curve_console = new CurveConsole(session, this);
	sample_ref_console = new SampleRefConsole(session, this);
	capture_console = new CaptureConsole(session, this);
	signal_chain_console = new SignalChainPanel(session, this);

	add_console(song_console);
	add_console(sample_manager);
	add_console(track_console);
	add_console(effects_console);
	add_console(dummy_editor_console);
	add_console(audio_editor_console);
	add_console(midi_editor_console);
	add_console(bars_editor_console);
	add_console(curve_console);
	add_console(sample_ref_console);
	add_console(capture_console);
	add_console(signal_chain_console);

	event("close", [this]{ on_close(); });

	expand("revealer", false);
	visible = false;
	active_console = -1;
}

SideBar::~SideBar() {
	if (visible and (active_console >= 0))
		consoles[active_console]->on_leave();
}

void SideBar::add_console(SideBarConsole *c) {
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	c->hide();
}

void SideBar::on_close() {
	test_allow_close().then([this] (bool allow) {
		if (allow)
			session->set_mode(EditMode::Default);
			//_hide();
	});
}

void SideBar::_show() {
	if (!visible and (active_console >= 0))
		consoles[active_console]->on_enter();

	expand("revealer", true);
	visible = true;
	out_changed.notify();
}

// FIXME: this is the official closing function...
void SideBar::_hide() {
	if (visible and (active_console >= 0))
		consoles[active_console]->on_leave();

	expand("revealer", false);
	visible = false;
	out_changed.notify();
}

void SideBar::choose(int console) {
	if (active_console >= 0) {
		if (visible)
			consoles[active_console]->on_leave();
		consoles[active_console]->hide();
	}

	active_console = console;

	consoles[active_console]->show();
	if (visible)
		consoles[active_console]->on_enter();
	set_string("title", consoles[active_console]->title);

	out_changed.notify();
}

void SideBar::open(int console) {
	choose(console);

	if (!visible)
		_show();
	out_changed.notify();
}

bool SideBar::is_active(int console) {
	return (active_console == console) and visible;
}

base::future<bool> SideBar::test_allow_close() {
	if (!visible or active_console < 0)
		return base::success<bool>(true);
	return consoles[active_console]->test_allow_close();
}


SideBarConsole::SideBarConsole(const string &_title, const string &_id, Session *_session, SideBar *bar) : obs::Node<hui::Panel>(_id, bar) {
	set_id(_id);
	title = _title;
	session = _session;
	song = session->song.get();
	view = session->view;

	BreadCrumps::add(this, session);
}

SideBarConsole::~SideBarConsole() {
	on_leave();
}

base::future<bool> SideBarConsole::test_allow_close() {
	return base::success<bool>(true);
}

}

