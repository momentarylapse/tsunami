/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#if __has_include(<lib/hui/hui.h>)
#define HAS_HUI 1
#include <lib/hui/Callback.h>
#include <lib/hui/config.h>
#endif
#include <lib/os/msg.h>

namespace os {
	extern bool is_main_thread();
}

namespace tsunami {
LogSource::LogSource(LogHub* _hub) {
	hub = _hub;
	broadcasting = false;
}

// TODO remove messages...?
LogSource::~LogSource() = default;

void LogSource::error(const string &message) {
	hub->add_message(this, LogHub::Type::Error, message, {});
}

void LogSource::warn(const string &message) {
	hub->add_message(this, LogHub::Type::Warning, message, {});
}

void LogSource::info(const string &message) {
	hub->add_message(this, LogHub::Type::Info, message, {});
}

void LogSource::debug(const string &message) {
	if (hub->allow_debug)
		hub->add_message(this, LogHub::Type::Debug, message, {});
}

void LogSource::question(const string &message, const Array<string> &responses) {
	hub->add_message(this, LogHub::Type::Question, message, responses);
}

void LogSource::status(const string &message) {
	hub->add_message(this, LogHub::Type::Status, message, {});
}

LogHub::LogHub() {
	allow_debug = false;
#ifdef HAS_HUI
	allow_debug = hui::config.get_bool("Log.Debug", false);
#endif
	allow_console_output = true;
}

LogHub::~LogHub() = default;

LogSource* LogHub::create_source() {
	return new LogSource(this);
}

LogSource* LogHub::create_broadcaster() {
	auto s = new LogSource(this);
	s->broadcasting = true;
	return s;
}


Array<LogHub::Message> LogHub::all(LogSource* source) {
	Array<Message> r;
	for (auto &m: messages)
		if ((m.source == source) or m.source->broadcasting)
			r.add(m);
	return r;
}


LogHub::Message LogHub::latest(LogSource* source) {
	for (int i=messages.num-1; i>=0; i--)
		if ((messages[i].source == source) or messages[i].source->broadcasting)
			return messages[i];
	return {};
}

bool LogHub::Message::operator==(const LogHub::Message &o) const {
	return (source == o.source) and (type == o.type) and (text == o.text);
}


void LogHub::add_message(LogSource* source, Type type, const string &message, const Array<string> &responses) {

	// make sure messages are handled in the gui thread...
	if (!os::is_main_thread()) {
#ifdef HAS_HUI
		hui::run_in_gui_thread([this, source, type, _message = message, _responses = responses] {
			add_message(source, type, _message, _responses);
		});
#endif
		return;
	}



	Message m = {source, type, message, responses};
	for (auto &b: blocked)
		if (m == b)
			return;

	int count = 0;
	for (auto &mm: messages.sub_ref(max(messages.num - 40, 0)))
		if (m == mm and m.type != Type::Status) {
			count ++;
			if (count > 8) {
				blocked.add(m);
#ifdef HAS_HUI
				hui::run_later(0.1f, [source, message] {
					source->warn(format("message blocked: '%s'", message));
				});
#endif
				return;
			}
		}

	messages.add(m);

	if (allow_console_output) {
		if (type == Type::Error) {
			msg_error(message);
		} else if (type == Type::Warning) {
			msg_write(message);
		} else if (type == Type::Question) {
		} else if (type == Type::Debug) {
			msg_write(message);
		} else if (type == Type::Status) {
		} else {
			msg_write(message);
		}
	}

	out_add_message.notify();
}

}
