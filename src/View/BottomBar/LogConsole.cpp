/*
 * LogConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "BottomBar.h"
#include "../../Session.h"
#include "../../Stuff/Log.h"
#include "LogConsole.h"

LogConsole::LogConsole(Session *session) :
	BottomBar::Console(_("Messages"), session)
{
	log = session->log;

	fromResource("log_console");

	event("clear", std::bind(&LogConsole::onClear, this));

	//hui::RunLater(0.5f, std::bind(&LogConsole::reload, this));
	reload();

	log->subscribe3(this, std::bind(&LogConsole::onLogAdd, this), Log::MESSAGE_ADD);
}

LogConsole::~LogConsole()
{
	log->unsubscribe(this);
}

void LogConsole::onClear()
{
	reset("log_list");
}

void console_add_message(LogConsole *lc, Log::Message &m)
{
	string text = m.text;
	if (m.session == Session::GLOBAL)
		text = "[global] " + m.text;
	if (m.type == Log::TYPE_ERROR){
		lc->addString("log_list", "hui:error\\" + text);
		lc->blink();
	}else if (m.type == Log::TYPE_WARNING){
		lc->addString("log_list", "hui:warning\\" + text);
		lc->blink();
	}else{
		lc->addString("log_list", "hui:info\\" + text);
	}
}

void LogConsole::reload()
{
	reset("log_list");
	auto messages = log->all(session);
	for (auto &m: messages)
		console_add_message(this, m);
}

void LogConsole::onLogAdd()
{
	auto m = log->last();
	if ((m.session == session) or (m.session == Session::GLOBAL))
		console_add_message(this, m);
}
