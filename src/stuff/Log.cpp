/*
 * Log.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "Log.h"
#include "../lib/hui/Callback.h"
#include "../lib/hui/config.h"
#include "../lib/os/msg.h"
#include "../Session.h"

namespace os {
	extern bool is_main_thread();
}

namespace tsunami {

Log::Log() {
	allow_debug = hui::config.get_bool("Log.Debug", false);
	allow_console_output = true;
}


void Log::error(Session *session, const string &message) {
	add_message(session, Type::Error, message, {});
}


void Log::warn(Session *session, const string &message) {
	add_message(session, Type::Warning, message, {});
}


void Log::info(Session *session, const string &message) {
	add_message(session, Type::Info, message, {});
}


void Log::debug(Session *session, const string &message) {
	if (allow_debug)
		add_message(session, Type::Debug, message, {});
}


void Log::question(Session *session, const string &message, const Array<string> &responses) {
	add_message(session, Type::Question, message, responses);
}


void Log::status(Session *session, const string &message) {
	add_message(session, Type::Status, message, {});
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

	// make sure messages are handled in the gui thread...
	if (!os::is_main_thread()) {
		hui::run_in_gui_thread([this, session, type, _message = message, _responses = responses] {
			add_message(session, type, _message, _responses);
		});
		return;
	}



	Message m = {session, type, message, responses};
	for (auto &b: blocked)
		if (m == b)
			return;

	int count = 0;
	for (auto &mm: messages.sub_ref(max(messages.num - 40, 0)))
		if (m == mm and m.type != Type::Status) {
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
