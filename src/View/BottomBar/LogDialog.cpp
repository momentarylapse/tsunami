/*
 * LogDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "LogDialog.h"
#include "BottomBar.h"
#include "../../Stuff/Log.h"

LogDialog::LogDialog(Log *_log) :
	BottomBarConsole(_("Nachrichten")),
	Observer("LogDialog")
{
	log = _log;

	addGrid("", 0, 0, 2, 1, "grid");
	setTarget("grid", 0);
	addListView("!nobar,format=it\\type\\msg", 0, 0, 0, 0, "log_list");
	addButton("", 1, 0, 0, 0, "clear");
	setImage("clear", "hui:clear");
	setTooltip("clear", _("alle Nachrichten l&oschen"));

	event("clear", this, &LogDialog::onClear);

	HuiRunLaterM(0.5f, this, &LogDialog::reload);

	subscribe(log);
}

LogDialog::~LogDialog()
{
	unsubscribe(log);
}

void LogDialog::onClear()
{
	log->clear();
}

void LogDialog::reload()
{
	reset("log_list");
	foreach(Log::Message &m, log->messages){
		if (m.type == Log::TYPE_ERROR){
			addString("log_list", "hui:error\\" + m.text);
			((BottomBar*)parent)->choose(BottomBar::LOG_CONSOLE);
		}else if (m.type == Log::TYPE_WARNING){
			addString("log_list", "hui:warning\\" + m.text);
			((BottomBar*)parent)->choose(BottomBar::LOG_CONSOLE);
		}else{
			addString("log_list", "hui:info\\" + m.text);
		}
	}
}

void LogDialog::onUpdate(Observable *o, const string &message)
{
	reload();
}
