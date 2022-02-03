/*
 * LogConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "BottomBar.h"
#include "../../Session.h"
#include "../../Stuff/Log.h"
#include "../../Data/Song.h"
#include "LogConsole.h"

LogConsole::LogConsole(Session *session, BottomBar *bar) :
	BottomBar::Console(_("Messages"), "log-console", session, bar)
{
	log = session->log;
	messages_loaded = 0;

	from_resource("log_console");

	// only start after this->win is set
	hui::run_later(0.01f, [=]{ reload(); });
}

LogConsole::~LogConsole() {
	log->unsubscribe(this);
}

void console_add_message(LogConsole *lc, Log::Message &m) {
	hui::ComboBoxSeparator = "§§";
	string text = m.text;
	if (m.session == Session::GLOBAL)
		text = "[global] " + text;
	else
		text = "[" + m.session->song->filename.basename() + "]: " + text;
	if (m.type == Log::Type::DEBUG)
		text = "[debug] " + text;

	if (m.type == Log::Type::ERROR) {
		lc->add_string("log_list", "hui:error§§" + text);
		//lc->blink();
		lc->win->set_info_text(m.text, {"error", "allow-close", "id=error"});
	} else if (m.type == Log::Type::WARNING) {
		lc->add_string("log_list", "hui:warning§§" + text);
		lc->win->set_info_text(m.text, {"warning", "allow-close", "id=warning"});
	} else if (m.type == Log::Type::QUESTION) {
		lc->add_string("log_list", "hui:question§§" + text);
		Array<string> options = {"warning", "allow-close", "id=question-" + i2s(rand())};
		for (auto &o: m.responses)
			options.add("button:" + o);
		lc->win->set_info_text(m.text, options);
	} else {
		lc->add_string("log_list", "§§" + text);
	}
	hui::ComboBoxSeparator = "\\";
}

void LogConsole::reload() {
	log->unsubscribe(this);

	reset("log_list");
	auto messages = log->all(session);
	for (auto &m: messages)
		console_add_message(this, m);
	messages_loaded = messages.num;

	log->subscribe(this, [=]{ on_log_add(); }, Log::MESSAGE_ADD);
}

void LogConsole::on_log_add() {
	auto messages = log->all(session);
	for (auto &m: messages.sub_ref(messages_loaded))
		console_add_message(this, m);
	messages_loaded = messages.num;
}
