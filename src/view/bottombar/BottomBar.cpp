/*
 * BottomBar.cpp
 *
 *  Created on: 23.03.2014
 *      Author: michi
 */

#include "BottomBar.h"
#include "MixingConsole.h"
#include "LogConsole.h"
#include "SignalChainConsole.h"
#include "DeviceConsole.h"
#include "PluginConsole.h"
#include "SessionConsole.h"
#include "../audioview/AudioView.h"
#include "../../Session.h"

namespace tsunami {

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
	if (hui::Application::adwaita_started)
		add_list_view("!width=130,noexpandx,nobar,class=navigation-sidebar,noframe\\", 0, 0, "choose");
	else
		add_tab_control("!left,noframe,noexpandx,expandy", 1, 0, "choose");
	//add_separator("!vertical", 1, 0, "");
	add_grid("", 2, 0, "console_grid");
	set_target("button_grid");

	mixing_console = new MixingConsole(session, this);
	signal_chain_console = new SignalChainConsole(session, this);
	plugin_console = new PluginConsole(session, this);
	device_console = new DeviceConsole(session, this);
	session_console = new SessionConsole(session, this);
	log_console = new LogConsole(session, this);
	add_console(mixing_console, "");
	add_console(signal_chain_console, "");
	add_console(plugin_console, "");
	add_console(device_console, "");
	add_console(session_console, "");
	add_console(log_console, "");

	if (hui::Application::adwaita_started)
		event_x("choose", "hui:select", [this] { on_choose(); });
	else
		event("choose", [this] { on_choose(); });
	event("close", [this] { on_close(); });

	expand("revealer", false);
	visible = false;
	active_console = nullptr;

	choose(mixing_console);


	for (auto c: weak(consoles))
		if (c->request_notify)
			open(c);
}

void BottomBar::on_close() {
	_hide();
}

void BottomBar::_show() {
	if (!visible and active_console)
		active_console->on_enter();
	expand("revealer", true);
	visible = true;
	if (active_console)
		active_console->show();
	out_changed.notify();
}

void BottomBar::_hide() {
	expand("revealer", false);
	visible = false;
	if (active_console) {
		active_console->hide();
		active_console->on_leave();
	}
	out_changed.notify();
}

void BottomBar::add_console(BottomBar::Console *c, const string &list_name) {
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	add_string("choose", list_name + c->title);
	c->hide();
}

BottomBar::Index BottomBar::index(BottomBar::Console *console) {
	foreachi (auto c, weak(consoles), i)
		if (console == c)
			return (Index)i;
	return (Index)-1;
}

void BottomBar::on_choose() {
	int n = get_int("");
	if (n >= 0)
		open(weak(consoles)[n]);
}

void BottomBar::choose(BottomBar::Console *console) {
	if (active_console) {
		if (visible)
			active_console->on_leave();
		active_console->hide();
	}

	active_console = console;

	if (visible) {
		active_console->show();
		active_console->on_enter();
	}
	set_int("choose", (int)index(active_console));

	out_changed.notify();
}

void BottomBar::open(BottomBar::Console *console) {
	choose(console);

	if (!visible)
		_show();
	out_changed.notify();
}

void BottomBar::open(Index console_index) {
	open(weak(consoles)[(int)console_index]);
}

void BottomBar::toggle(Index console_index) {
	if (is_active(console_index)) {
		_hide();
	} else {
		open(console_index);
	}
}

bool BottomBar::is_active(Index console_index) const {
	return (active_console == consoles[(int)console_index]) and visible;
}


BottomBar::Console::Console(const string &_title, const string &id, Session *_session, BottomBar *bar) :
		obs::Node<hui::Panel>(id, bar) {
	title = _title;
	request_notify = false;
	session = _session;
	song = session->song.get();
	view = session->view;
}

void BottomBar::Console::blink() {
	if (bar()) {
		bar()->open(this);
	} else {
		request_notify = true;
	}
}

}

