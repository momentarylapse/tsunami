/*
 * SideBar.cpp
 *
 *  Created on: 25.03.2014
 *      Author: michi
 */

#include "SideBar.h"
#include "AudioFileConsole.h"
#include "TrackConsole.h"
#include "MidiEditor.h"
#include "SubDialog.h"

SideBar::SideBar(AudioView *view, AudioFile *audio) :
	Observable("SideBar")
{
	addGrid("!noexpandx,width=270,expandy", 0, 0, 2, 1, "root_grid0");
	setTarget("root_grid0", 0);
	addSeparator("!vertical,expandy", 0, 0, 0, 0, "");
	addGrid("!noexpandx,width=270,expandy", 1, 0, 1, 3, "root_grid");
	setTarget("root_grid", 0);
	addGrid("", 0, 0, 2, 1, "button_grid");
	addSeparator("", 0, 1, 0, 0, "");
	addGrid("", 0, 2, 1, 20, "console_grid");
	setTarget("button_grid", 0);
	addButton("!noexpandx,flat", 0, 0, 0, 0, "close");
	setImage("close", "hui:close");
	addLabel("!big,expandx,center\\...", 1, 0, 0, 0, "title");

	audio_file_console = new AudioFileConsole(audio);
	track_console = new TrackConsole(view);
	track_midi_editor = new MidiEditor(view, audio);
	sub_dialog = new SubDialog(view, audio);

	//addConsole(audio_file_console, "");
	embed(audio_file_console, "console_grid", 0, 0);
	embed(track_console, "console_grid", 0, 1);
	embed(track_midi_editor, "console_grid", 0, 2);
	embed(sub_dialog, "console_grid", 0, 3);

	event("close", (HuiPanel*)this, (void(HuiPanel::*)())&SideBar::onClose);

	choose(AUDIOFILE_CONSOLE);
	visible = true;
}

SideBar::~SideBar()
{
}

void SideBar::onClose()
{
	hide();
}

void SideBar::onShow()
{
	visible = true;
	notify();
}

void SideBar::onHide()
{
	visible = false;
	notify();
}

void SideBar::choose(int console)
{
	foreachi(HuiPanel *p, children, i){
		if (i == console){
			setString("title", "!big\\" + ((SideBarConsole*)p)->title);
			p->show();
		}else
			p->hide();
	}
	active_console = console;
	notify();
}

void SideBar::open(int console)
{
	notifyBegin();
	choose(console);
	active_console = console;
	if (!visible)
		show();
	notifyEnd();
}

bool SideBar::isActive(int console)
{
	return (active_console == console) && visible;
}

