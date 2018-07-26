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
#include "../AudioView.h"
#include "MiniBar.h"
#include "../../Session.h"

BottomBar::BottomBar(Session *session)
{
	addRevealer("!slide-up", 0, 0, "revealer");
	setTarget("revealer");
	addGrid("!noexpandy,height=300,expandx", 0, 0, "root_grid0");
	setTarget("root_grid0");
	addSeparator("!horizontal,expandx", 0, 0, "");
	addGrid("!expandx", 0, 1, "root_grid");
	setTarget("root_grid");
	addGrid("!noexpandx", 0, 0, "button_grid");
	addSeparator("!vertical", 1, 0, "");
	addGrid("", 2, 0, "console_grid");
	setTarget("button_grid");
	addButton("!noexpandy,flat", 0, 0, "close");
	setImage("close", "hui:close");
	addTabControl("!left,expandx,expandy", 0, 1, "choose");

	mixing_console = new MixingConsole(session);
	signal_editor = new SignalEditor(session);
	device_console = new DeviceConsole(session);
	log_console = new LogConsole(session);
	addConsole(mixing_console, "");
	addConsole(signal_editor, "");
	addConsole(device_console, "");
	addConsole(log_console, "");

	event("choose", std::bind(&BottomBar::onChoose, this));
	event("close", std::bind(&BottomBar::onClose, this));

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

void BottomBar::onClose()
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

void BottomBar::addConsole(BottomBar::Console *c, const string &list_name)
{
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	addString("choose", list_name + c->title);
	c->hide();
}

int BottomBar::index(BottomBar::Console *console)
{
	foreachi (auto c, consoles, i)
		if (console == c)
			return i;
	return -1;
}

void BottomBar::onChoose()
{
	int n = getInt("");
	if (n >= 0)
		open(consoles[n]);
}

void BottomBar::choose(BottomBar::Console *console)
{
	if (active_console)
		active_console->hide();

	active_console = console;

	active_console->show();
	setInt("choose", index(active_console));

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
	open(consoles[console_index]);
}

bool BottomBar::isActive(int console_index)
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

