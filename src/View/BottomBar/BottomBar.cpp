/*
 * BottomBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "MixingConsole.h"
#include "LogConsole.h"
#include "DeviceConsole.h"
#include "SignalEditor.h"
#include "PluginConsole.h"
#include "../AudioView.h"
#include "MiniBar.h"
#include "../../Session.h"

BottomBar::BottomBar(Session *session)
{
	add_revealer("!slide=up", 0, 0, "revealer");
	set_target("revealer");
	add_grid("!noexpandy,height=330,expandx", 0, 0, "root_grid0");
	set_target("root_grid0");
	add_separator("!horizontal,expandx", 0, 0, "");
	add_grid("!expandx", 0, 1, "root_grid");
	set_target("root_grid");
	add_grid("!noexpandx", 0, 0, "button_grid");
	add_separator("!vertical", 1, 0, "");
	add_grid("", 2, 0, "console_grid");
	set_target("button_grid");
	add_button("!noexpandy,flat", 0, 0, "close");
	set_image("close", "hui:close");
	add_tab_control("!left,expandx,expandy", 0, 1, "choose");

	mixing_console = new MixingConsole(session);
	signal_editor = new SignalEditor(session);
	device_console = new DeviceConsole(session);
	plugin_console = new PluginConsole(session);
	log_console = new LogConsole(session);
	add_console(mixing_console, "");
	add_console(signal_editor, "");
	add_console(device_console, "");
	add_console(plugin_console, "");
	add_console(log_console, "");

	event("choose", [=]{ on_choose(); });
	event("close", [=]{ on_close(); });

	reveal("revealer", false);
	visible = false;
	active_console = nullptr;

	choose(mixing_console);


	for (auto c: consoles)
		if (c->notify)
			open(c);
}

BottomBar::~BottomBar()
{
}

void BottomBar::on_close()
{
	_hide();
}

void BottomBar::_show()
{
	reveal("revealer", true);
	visible = true;
	notify();
}

void BottomBar::_hide()
{
	reveal("revealer", false);
	visible = false;
	notify();
}

void BottomBar::add_console(BottomBar::Console *c, const string &list_name)
{
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	add_string("choose", list_name + c->title);
	c->hide();
}

int BottomBar::index(BottomBar::Console *console)
{
	foreachi (auto c, consoles, i)
		if (console == c)
			return i;
	return -1;
}

void BottomBar::on_choose()
{
	int n = get_int("");
	if (n >= 0)
		open(consoles[n]);
}

void BottomBar::choose(BottomBar::Console *console)
{
	if (active_console)
		active_console->hide();

	active_console = console;

	active_console->show();
	set_int("choose", index(active_console));

	notify();
}

void BottomBar::open(BottomBar::Console *console)
{
	choose(console);

	if (!visible)
		_show();
	notify();
}

void BottomBar::open(int console_index)
{
	if (console_index == FAKE_FX_CONSOLE){
		open(mixing_console);
		mixing_console->set_mode(MixerMode::EFFECTS);
		return;
	}
	if (console_index == FAKE_MIDI_FX_CONSOLE){
		open(mixing_console);
		mixing_console->set_mode(MixerMode::MIDI_EFFECTS);
		return;
	}
	open(consoles[console_index]);
}

bool BottomBar::is_active(int console_index)
{
	return (active_console == consoles[console_index]) and visible;
}


BottomBar::Console::Console(const string &_title, Session *_session)
{
	title = _title;
	notify = false;
	session = _session;
	song = session->song;
	view = session->view;
}

void BottomBar::Console::blink()
{
	if (bar()){
		bar()->open(this);
	}else{
		notify = true;
	}
}

