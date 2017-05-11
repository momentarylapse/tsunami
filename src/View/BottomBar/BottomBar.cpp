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
#include "../../lib/hui/Controls/HuiControl.h"
#include "../AudioView.h"
#include "MiniBar.h"

BottomBar::BottomBar(AudioView *view, Song *song, DeviceManager *device_manager, Log *log) :
	Observable("BottomBar")
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
	mixing_console = new MixingConsole(song, device_manager, view->stream, view);
	device_console = new DeviceConsole(device_manager);
	addConsole(log_console, "");
	addConsole(mixing_console, "");
	addConsole(device_console, "");

	eventX("choose", "hui:select", std::bind(&BottomBar::onChoose, this));
	event("close", std::bind(&BottomBar::onClose, this));

	reveal("revealer", false);
	visible = false;
	active_console = -1;

	choose(MIXING_CONSOLE);
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

void BottomBar::addConsole(BottomBarConsole *c, const string &list_name)
{
	embed(c, "console_grid", 0, consoles.num);
	consoles.add(c);
	addString("choose", list_name + c->title);
	c->hide();
}

void BottomBar::onChoose()
{
	int n = getInt("");
	if (n >= 0)
		open(n);
}

void BottomBar::choose(int console)
{
	if (active_console >= 0)
		consoles[active_console]->hide();

	active_console = console;

	consoles[active_console]->show();
	setInt("choose", console);

	notify();
}

void BottomBar::open(int console)
{
	choose(console);

	if (!visible)
		_show();
	notify();
}

bool BottomBar::isActive(int console)
{
	return (active_console == console) and visible;
}

