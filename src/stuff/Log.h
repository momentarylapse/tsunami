/*
 * Log.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_STUFF_LOG_H_
#define SRC_STUFF_LOG_H_

#include "../lib/base/base.h"
#include "../lib/pattern/Observable.h"

namespace tsunami {

class Session;

class Log : public obs::Node<VirtualBase> {
public:
	Log();

	obs::source out_add_message{this, "add-message"};

	void error(Session *session, const string &message);
	void warn(Session *session, const string &message);
	void info(Session *session, const string &message);
	void debug(Session *session, const string &message);
	void question(Session *session, const string &message, const Array<string> &responses);
	void status(Session *session, const string &message);

	enum class Type {
		Error,
		Warning,
		Question,
		Info,
		Debug,
		Status
	};

	struct Message {
		Session *session;
		Type type;
		string text;
		Array<string> responses;
		bool operator==(const Message &o) const;
	};

	Array<Message> all(Session *session);
	Message latest(Session *session);

	bool allow_debug;
	bool allow_console_output;

private:
	void add_message(Session *session, Type type, const string &message, const Array<string> &responses);
	Array<Message> messages;
	Array<Message> blocked;
};

}

#endif /* SRC_STUFF_LOG_H_ */
