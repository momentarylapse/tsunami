/*
 * LogConsole.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "BottomBar.h"
#include "../../Session.h"
#include "../../data/Song.h"
#include "LogConsole.h"
#include <lib/pattern/Log.h>
#include <lib/hui/hui.h>

namespace tsunami {

string title_filename(const Path &filename);


LogConsole::LogConsole(Session *session, BottomBar *bar) :
	BottomBar::Console(_("Messages"), "log-console", session, bar)
{
	source = session->log_source.get();
	messages_loaded = 0;

	from_resource("log_console");

	// only start after this->win is set
	hui::run_later(0.01f, [this]{ reload(); });
}

LogConsole::~LogConsole() {
	source->hub->unsubscribe(this);
}

void console_add_message(LogConsole *lc, obs::LogHub::Message &m) {
	hui::separator = "§§";
	string text = m.text;
	string source;
	if (m.source->broadcasting)
		source = "[global]";

	auto wrap_source = [] (const string &s) {
		return "<span alpha=\"50%%\">" + s + "</span>";
	};

	if (m.type == obs::MessageType::Error) {
		lc->add_string("log_list", format("%s  <span foreground=\"red\">َ<b>Error: %s</b></span>", wrap_source(source), text));
		//lc->blink();
	} else if (m.type == obs::MessageType::Warning) {
		lc->add_string("log_list", format("%s  <span foreground=\"orange\">َ<b>Warning:</b> %s</span>", wrap_source(source), text));
	} else if (m.type == obs::MessageType::Question) {
		lc->add_string("log_list", format("%s  <b>Question:</b> %s", wrap_source(source), text));
	} else if (m.type == obs::MessageType::Debug) {
		lc->add_string("log_list", format("%s  <span alpha=\"50%%\">َDebug: %s</span>", wrap_source(source), text));
	} else if (m.type == obs::MessageType::Status) {
	} else {
		lc->add_string("log_list", format("%s  %s", wrap_source(source), text));
	}
	hui::separator = "\\";
}

void LogConsole::reload() {
	source->hub->unsubscribe(this);

	reset("log_list");
	auto messages = source->hub->all(source);
	for (auto &m: messages)
		console_add_message(this, m);
	messages_loaded = messages.num;

	source->hub->out_add_message >> create_sink([this]{ on_log_add(); });
}

void LogConsole::on_log_add() {
	auto messages = source->hub->all(source);
	for (auto &m: messages.sub_ref(messages_loaded))
		console_add_message(this, m);
	messages_loaded = messages.num;
}

}
