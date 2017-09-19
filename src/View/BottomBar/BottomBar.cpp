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
#include "../AudioView.h"
#include "MiniBar.h"

BottomBar::BottomBar(AudioView *view, Song *song, DeviceManager *device_manager, Log *log)
{
	addRevealer("!slide-up", 0, 0, 0, 0, "revealer");
	setTarget("revealer", 0);
	addGrid("!noexpandy,height=300,expandx", 0, 0, 1, 2, "root_grid0");
	setTarget("root_grid0", 0);
	addSeparator("!horizontal,expandx", 0, 0, 0, 0, "");
	addGrid("!expandx", 0, 1, 3, 1, "root_grid");
	setTarget("root_grid", 0);
	addGrid("!noexpandx,width=130", 0, 0, 1, 2, "button_grid");
	addSeparator("!vertical", 1, 0, 0, 0, "");
	addGrid("", 2, 0, 1, 20, "console_grid");
	setTarget("button_grid", 0);
	addButton("!noexpandy,flat", 0, 0, 0, 0, "close");
	setImage("close", "hui:close");
	addListView("!nobar\\name", 0, 1, 0, 0, "choose");

	log_console = new LogConsole(log);
	mixing_console = new MixingConsole(song, device_manager, view);
	device_console = new DeviceConsole(device_manager);
	addConsole(log_console, "");
	addConsole(mixing_console, "");
	addConsole(device_console, "");

	eventX("choose", "hui:select", std::bind(&BottomBar::onChoose, this));
	event("close", std::bind(&BottomBar::onClose, this));

	reveal("revealer", false);
	visible = false;
	active_console = NULL;

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

void BottomBar::Console::blink()
{
	if (bar()){
		bar()->choose(this);
	}else{
		notify = true;
	}
}

