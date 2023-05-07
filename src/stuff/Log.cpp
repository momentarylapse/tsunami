/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#include "../lib/hui/hui.h"
#include "../Session.h"

const string Log::MESSAGE_ADD = "add-message";

Log::Log() {
	allow_debug = hui::config.get_bool("Log.Debug", false);
	allow_console_output = true;
}


void Log::error(Session *session, const string &message) {
	add_message(session, Type::ERROR, message, {});
}


void Log::warn(Session *session, const string &message) {
	add_message(session, Type::WARNING, message, {});
}


void Log::info(Session *session, const string &message) {
	add_message(session, Type::INFO, message, {});
}


void Log::debug(Session *session, const string &message) {
	if (allow_debug)
		add_message(session, Type::DEBUG, message, {});
}


void Log::question(Session *session, const string &message, const Array<string> &responses) {
	add_message(session, Type::QUESTION, message, responses);
}


void Log::status(Session *session, const string &message) {
	add_message(session, Type::STATUS, message, {});
}


Array<Log::Message> Log::all(Session *session) {
	Array<Log::Message> r;
	for (auto &m: messages)
		if ((m.session == session) or (m.session == Session::GLOBAL))
			r.add(m);
	return r;
}


Log::Message Log::latest(Session *session) {
	for (int i=messages.num-1; i>=0; i--)
		if ((messages[i].session == session) or (messages[i].session == Session::GLOBAL))
			return messages[i];
	return Message();
}

bool Log::Message::operator==(const Log::Message &o) const {
	return (session == o.session) and (type == o.type) and (text == o.text);
}


void Log::add_message(Session *session, Type type, const string &message, const Array<string> &responses) {
	Message m = {session, type, message, responses};
	for (auto &b: blocked)
		if (m == b)
			return;

	int count = 0;
	for (auto &mm: messages.sub_ref(max(messages.num - 40, 0)))
		if (m == mm and m.type != Type::STATUS) {
			count ++;
			if (count > 8) {
				blocked.add(m);
				hui::run_later(0.1f, [this, session, message] {
					warn(session, format("message blocked: '%s'", message));
				});
				return;
			}
		}

	messages.add(m);

	if (allow_console_output) {
		if (type == Type::ERROR) {
			msg_error(message);
		} else if (type == Type::WARNING) {
			msg_write(message);
		} else if (type == Type::QUESTION) {
		} else if (type == Type::DEBUG) {
			msg_write(message);
		} else if (type == Type::STATUS) {
		} else {
			msg_write(message);
		}
	}

	// make sure messages are handled in the gui thread...
	hui::run_later(0.01f, [this] {
		out_add_message.notify();
	});
}
