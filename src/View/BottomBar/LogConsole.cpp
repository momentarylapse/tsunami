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

	from_resource("log_console");

	// only start after this->win is set
	hui::RunLater(0.01f, [=]{ reload(); });
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
	if (m.type == Log::Type::DEBUG)
		text = "[debug] " + text;
	if (m.type == Log::Type::ERROR){
		lc->add_string("log_list", "hui:error§§" + text);
		//lc->blink();
		lc->win->set_info_text(m.text, {"error", "allow-close"});
	}else if (m.type == Log::Type::WARNING){
		lc->add_string("log_list", "hui:warning§§" + text);
		lc->win->set_info_text(m.text, {"warning", "allow-close"});
	}else if (m.type == Log::Type::QUESTION){
		lc->add_string("log_list", "hui:question§§" + text);
		Array<string> options = {"warning", "allow-close"};
		for (auto &o: m.responses)
			options.add("button:" + o);
		lc->win->set_info_text(m.text, options);
	}else{
		lc->add_string("log_list", "§§" + text);
	}
	hui::ComboBoxSeparator = "\\";
}

void LogConsole::reload()
{
	log->unsubscribe(this);

	reset("log_list");
	auto messages = log->all(session);
	for (auto &m: messages)
		console_add_message(this, m);
	messages_loaded = messages.num;

	log->subscribe3(this, std::bind(&LogConsole::on_log_add, this), Log::MESSAGE_ADD);
}

void LogConsole::on_log_add()
{
	auto messages = log->all(session);
	for (auto &m: messages.sub(messages_loaded, -1))
		console_add_message(this, m);
	messages_loaded = messages.num;
}
