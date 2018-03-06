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
	BottomBar::Console(_("Messages"))
{
	log = _log;

	fromResource("log_console");

	event("clear", std::bind(&LogConsole::onClear, this));

	//hui::RunLater(0.5f, std::bind(&LogConsole::reload, this));
	reload();

	log->subscribe3(this, std::bind(&LogConsole::onLogAdd, this), Log::MESSAGE_ADD);
	log->subscribe3(this, std::bind(&LogConsole::onLogClear, this), Log::MESSAGE_CLEAR);
}

LogConsole::~LogConsole()
{
	log->unsubscribe(this);
}

void LogConsole::onClear()
{
	log->clear();
}

void console_add_message(LogConsole *lc, Log::Message &m)
{
	if (m.type == Log::TYPE_ERROR){
		lc->addString("log_list", "hui:error\\" + m.text);
		lc->blink();
	}else if (m.type == Log::TYPE_WARNING){
		lc->addString("log_list", "hui:warning\\" + m.text);
		lc->blink();
	}else{
		lc->addString("log_list", "hui:info\\" + m.text);
	}
}

void LogConsole::reload()
{
	reset("log_list");
	auto messages = log->all();
	for (auto &m: messages)
		console_add_message(this, m);
}

void LogConsole::onLogAdd()
{
	auto m = log->last();
	console_add_message(this, m);
}

void LogConsole::onLogClear()
{
	reset("log_list");
}
