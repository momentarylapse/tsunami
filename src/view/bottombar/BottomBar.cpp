/*
 * BottomBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "MixingConsole.h"
#include "LogConsole.h"
#include "SignalEditor.h"
#include "PluginConsole.h"
#include "SessionConsole.h"
#include "../audioview/AudioView.h"
#include "MiniBar.h"
#include "../../Session.h"

BottomBar::BottomBar(Session *session, hui::Panel *parent) {
	set_parent(parent);
	set_id("bottom-bar");

	add_expander("!slide=up", 0, 0, "revealer");
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

	mixing_console = new MixingConsole(session, this);
	signal_editor = new SignalEditor(session, this);
	plugin_console = new PluginConsole(session, this);
	session_console = new SessionConsole(session, this);
	log_console = new LogConsole(session, this);
	add_console(mixing_console, "");
	add_console(signal_editor, "");
	add_console(plugin_console, "");
	add_console(session_console, "");
	add_console(log_console, "");

	event("choose", [this]{ on_choose(); });
	event("close", [this]{ on_close(); });

	expand("revealer", false);
	visible = false;
	active_console = nullptr;

	choose(mixing_console);


	for (auto c: weak(consoles))
		if (c->notify)
			open(c);
}

void BottomBar::on_close() {
	_hide();
}

void BottomBar::_show() {
	expand("revealer", true);
	visible = true;
	if (active_console)
		active_console->show();
	notify();
}

void BottomBar::_hide() {
	expand("revealer", false);
	visible = false;
	if (active_console)
		active_console->hide();
	notify();
}

void BottomBar::add_console(BottomBar::Console *c, const string &list_name) {
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	add_string("choose", list_name + c->title);
	c->hide();
}

int BottomBar::index(BottomBar::Console *console) {
	foreachi (auto c, weak(consoles), i)
		if (console == c)
			return i;
	return -1;
}

void BottomBar::on_choose() {
	int n = get_int("");
	if (n >= 0)
		open(weak(consoles)[n]);
}

void BottomBar::choose(BottomBar::Console *console) {
	if (active_console)
		active_console->hide();

	active_console = console;

	if (visible)
		active_console->show();
	set_int("choose", index(active_console));

	notify();
}

void BottomBar::open(BottomBar::Console *console) {
	choose(console);

	if (!visible)
		_show();
	notify();
}

void BottomBar::open(int console_index) {
	open(weak(consoles)[console_index]);
}

void BottomBar::toggle(int console_index) {
	if (is_active(console_index)) {
		_hide();
	} else {
		open(console_index);
	}
}

bool BottomBar::is_active(int console_index) {
	return (active_console == consoles[console_index]) and visible;
}


BottomBar::Console::Console(const string &_title, const string &id, Session *_session, BottomBar *bar) : hui::Panel(id, bar) {
	title = _title;
	notify = false;
	session = _session;
	song = session->song.get();
	view = session->view;
}

void BottomBar::Console::blink() {
	if (bar()) {
		bar()->open(this);
	} else {
		notify = true;
	}
}

