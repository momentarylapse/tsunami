/*
 * SideBar.cpp
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#include "SideBar.h"
#include "LevelConsole.h"
#include "TrackConsole.h"
#include "MidiEditor.h"
#include "SynthConsole.h"
#include "SampleRefDialog.h"
#include "SampleManager.h"
#include "../AudioView.h"
#include "SongConsole.h"

SideBar::SideBar(AudioView *view, Song *song) :
	Observable("SideBar")
{
	addRevealer("!slide-left", 0, 0, 0, 0, "revealer");
	setTarget("revealer", 0);
	addGrid("!noexpandx,width=380,expandy", 0, 0, 2, 1, "root_grid0");
	setTarget("root_grid0", 0);
	addSeparator("!vertical,expandy", 0, 0, 0, 0, "");
	addGrid("!expandx,expandy", 1, 0, 1, 3, "root_grid");
	setTarget("root_grid", 0);
	addGrid("", 0, 0, 2, 1, "button_grid");
	addSeparator("", 0, 1, 0, 0, "");
	addGrid("", 0, 2, 1, 20, "console_grid");
	setTarget("button_grid", 0);
	addButton("!noexpandx,flat", 0, 0, 0, 0, "close");
	setImage("close", "hui:close");
	addLabel("!big,expandx,center\\...", 1, 0, 0, 0, "title");

	song_console = new SongConsole(song);
	level_console = new LevelConsole(song, view);
	sample_manager = new SampleManager(song);
	track_console = new TrackConsole(view);
	midi_editor = new MidiEditor(view, song);
	synth_console = new SynthConsole(view);
	sample_ref_dialog = new SampleRefDialog(view, song);

	addConsole(song_console);
	addConsole(level_console);
	addConsole(sample_manager);
	addConsole(track_console);
	addConsole(midi_editor);
	addConsole(synth_console);
	addConsole(sample_ref_dialog);

	event("close", (HuiPanel*)this, (void(HuiPanel::*)())&SideBar::onClose);

	reveal("revealer", false);
	visible = false;
	active_console = -1;

	view->subscribe(this);
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
	reveal("revealer", true);
	visible = true;
	notify();
}

void SideBar::_hide()
{
	reveal("revealer", false);
	visible = false;
	notify();
}

void SideBar::choose(int console)
{
	if (active_console >= 0)
		consoles[active_console]->hide();

	active_console = console;

	consoles[active_console]->show();
	setString("title", "!big\\" + consoles[active_console]->title);

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

