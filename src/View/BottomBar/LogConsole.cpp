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
	messages_loaded = 0;

	fromResource("log_console");

	reload();

	log->subscribe3(this, std::bind(&LogConsole::on_log_add, this), Log::MESSAGE_ADD);
}

LogConsole::~LogConsole()
{
	log->unsubscribe(this);
}

void console_add_message(LogConsole *lc, Log::Message &m)
{
	hui::ComboBoxSeparator = "§§";
	string text = m.text;
	if (m.session == Session::GLOBAL)
		text = "[global] " + text;
	if (m.type == Log::Type::ERROR){
		lc->addString("log_list", "hui:error§§" + text);
		lc->blink();
	}else if (m.type == Log::Type::WARNING){
		lc->addString("log_list", "hui:warning§§" + text);
		lc->blink();
	}else{
		lc->addString("log_list", "§§" + text);
	}
	hui::ComboBoxSeparator = "\\";
}

void LogConsole::reload()
{
	reset("log_list");
	auto messages = log->all(session);
	for (auto &m: messages)
		console_add_message(this, m);
	messages_loaded = messages.num;
}

void LogConsole::on_log_add()
{
	auto messages = log->all(session);
	for (auto &m: messages.sub(messages_loaded, -1))
		console_add_message(this, m);
	messages_loaded = messages.num;
}
