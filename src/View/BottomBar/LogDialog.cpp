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

	AddControlTable("", 0, 0, 2, 1, "grid");
	SetTarget("grid", 0);
	AddListView("!nobar,format=it\\type\\msg", 0, 0, 0, 0, "log_list");
	AddButton("", 1, 0, 0, 0, "clear");
	SetImage("clear", "hui:clear");
	SetTooltip("clear", _("alle Nachrichten l&oschen"));

	EventM("clear", this, &LogDialog::onClear);

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
	Reset("log_list");
	foreach(Log::Message &m, log->messages){
		if (m.type == Log::TYPE_ERROR){
			AddString("log_list", "hui:error\\" + m.text);
			((BottomBar*)parent)->choose(BottomBar::LOG_CONSOLE);
		}else if (m.type == Log::TYPE_WARNING){
			AddString("log_list", "hui:warning\\" + m.text);
			((BottomBar*)parent)->choose(BottomBar::LOG_CONSOLE);
		}else{
			AddString("log_list", "hui:info\\" + m.text);
		}
	}
}

void LogDialog::onUpdate(Observable *o, const string &message)
{
	reload();
}
