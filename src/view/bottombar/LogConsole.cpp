/*
 * LogConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "BottomBar.h"
#include "../../Session.h"
#include "../../stuff/Log.h"
#include "../../data/Song.h"
#include "LogConsole.h"

namespace tsunami {

string title_filename(const Path &filename);


LogConsole::LogConsole(Session *session, BottomBar *bar) :
	BottomBar::Console(_("Messages"), "log-console", session, bar)
{
	log = session->log;
	messages_loaded = 0;

	from_resource("log_console");

	// only start after this->win is set
	hui::run_later(0.01f, [this]{ reload(); });
}

LogConsole::~LogConsole() {
	log->unsubscribe(this);
}

void console_add_message(LogConsole *lc, Log::Message &m) {
	hui::separator = "§§";
	string text = m.text;
	string source;
	if (m.session == Session::GLOBAL)
		source = "global";
	else
		source = title_filename(m.session->song->filename);

	auto wrap_source = [] (const string &s) {
		return "<span alpha=\"50%%\">[" + s + "]</span>";
	};

	if (m.type == Log::Type::Error) {
		lc->add_string("log_list", format("%s  <span foreground=\"red\">َ<b>Error: %s</b></span>", wrap_source(source), text));
		//lc->blink();
	} else if (m.type == Log::Type::Warning) {
		lc->add_string("log_list", format("%s  <span foreground=\"orange\">َ<b>Warning:</b> %s</span>", wrap_source(source), text));
	} else if (m.type == Log::Type::Question) {
		lc->add_string("log_list", format("%s  <b>Question:</b> %s", wrap_source(source), text));
	} else if (m.type == Log::Type::Debug) {
		lc->add_string("log_list", format("%s  <span alpha=\"50%%\">َDebug: %s</span>", wrap_source(source), text));
	} else if (m.type == Log::Type::Status) {
	} else {
		lc->add_string("log_list", format("%s  %s", wrap_source(source), text));
	}
	hui::separator = "\\";
}

void LogConsole::reload() {
	log->unsubscribe(this);

	reset("log_list");
	auto messages = log->all(session);
	for (auto &m: messages)
		console_add_message(this, m);
	messages_loaded = messages.num;

	log->out_add_message >> create_sink([this]{ on_log_add(); });
}

void LogConsole::on_log_add() {
	auto messages = log->all(session);
	for (auto &m: messages.sub_ref(messages_loaded))
		console_add_message(this, m);
	messages_loaded = messages.num;
}

}
