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
// TODO also xhui
#if __has_include(<lib/os/msg.h>)
#define HAS_OS 1
#include <lib/os/msg.h>

namespace os {
	extern bool is_main_thread();
}
#endif

namespace obs {
LogSource::LogSource(LogHub* _hub) {
	hub = _hub;
	broadcasting = false;
}

// TODO remove messages...?
LogSource::~LogSource() = default;

void LogSource::error(const string &message) {
	hub->add_message(this, MessageType::Error, message, {});
}

void LogSource::warn(const string &message) {
	hub->add_message(this, MessageType::Warning, message, {});
}

void LogSource::info(const string &message) {
	hub->add_message(this, MessageType::Info, message, {});
}

void LogSource::debug(const string &message) {
	if (hub->allow_debug)
		hub->add_message(this, MessageType::Debug, message, {});
}

void LogSource::question(const string &message, const Array<string> &responses) {
	hub->add_message(this, MessageType::Question, message, responses);
}

void LogSource::status(const string &message) {
	hub->add_message(this, MessageType::Status, message, {});
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


void LogHub::add_message(LogSource* source, MessageType type, const string &message, const Array<string> &responses) {

	// make sure messages are handled in the gui thread...
#ifdef HAS_HUI
	if (!os::is_main_thread()) {
		hui::run_in_gui_thread([this, source, type, _message = message, _responses = responses] {
			add_message(source, type, _message, _responses);
		});
		return;
	}
#endif



	Message m = {source, type, message, responses};
	for (auto &b: blocked)
		if (m == b)
			return;

	int count = 0;
	for (auto &mm: messages.sub_ref(max(messages.num - 40, 0)))
		if (m == mm and m.type != MessageType::Status) {
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

#ifdef HAS_OS
	if (allow_console_output) {
		if (type == MessageType::Error) {
			msg_error(message);
		} else if (type == MessageType::Warning) {
			msg_write(message);
		} else if (type == MessageType::Question) {
		} else if (type == MessageType::Debug) {
			msg_write(message);
		} else if (type == MessageType::Status) {
		} else {
			msg_write(message);
		}
	}
#endif

	out_add_message.notify();
}

}
