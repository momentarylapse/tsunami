/*
 * LogConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "BottomBar.h"
#include "../../Stuff/Log.h"
#include "LogConsole.h"

LogConsole::LogConsole(Log *_log) :
	BottomBarConsole(_("Nachrichten")),
	Observer("LogConsole")
{
	log = _log;

	addGrid("", 0, 0, 2, 1, "grid");
	setTarget("grid", 0);
	addListView("!nobar,format=it\\type\\msg", 0, 0, 0, 0, "log_list");
	addButton("", 1, 0, 0, 0, "clear");
	setImage("clear", "hui:clear");
	setTooltip("clear", _("alle Nachrichten l&oschen"));

	event("clear", this, &LogConsole::onClear);

	HuiRunLaterM(0.5f, this, &LogConsole::reload);

	subscribe(log);
}

LogConsole::~LogConsole()
{
	unsubscribe(log);
}

void LogConsole::onClear()
{
	log->clear();
}

void LogConsole::reload()
{
	reset("log_list");
	foreach(Log::Message &m, log->messages){
		if (m.type == Log::TYPE_ERROR){
			addString("log_list", "hui:error\\" + m.text);
			((BottomBar*)parent)->open(BottomBar::LOG_CONSOLE);
		}else if (m.type == Log::TYPE_WARNING){
			addString("log_list", "hui:warning\\" + m.text);
			((BottomBar*)parent)->open(BottomBar::LOG_CONSOLE);
		}else{
			addString("log_list", "hui:info\\" + m.text);
		}
	}
}

void LogConsole::onUpdate(Observable *o, const string &message)
{
	reload();
}
