/*
 * Log.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include "Observable.h"

namespace obs {

class LogHub;

class LogSource {
public:
	explicit LogSource(LogHub *hub);
	~LogSource();
	void error(const string &message);
	void warn(const string &message);
	void info(const string &message);
	void debug(const string &message);
	void question(const string &message, const Array<string> &responses);
	void status(const string &message);
	LogHub* hub;
	bool broadcasting;
};

enum class MessageType {
	Error,
	Warning,
	Question,
	Info,
	Debug,
	Status
};

class LogHub : public Node<VirtualBase> {
	friend class LogSource;
public:
	LogHub();
	~LogHub() override;

	source out_add_message{this, "add-message"};

	struct Message {
		LogSource* source;
		MessageType type;
		string text;
		Array<string> responses;
		bool operator==(const Message &o) const;
	};

	Array<Message> all(LogSource* source);
	Message latest(LogSource* source);

	bool allow_debug;
	bool allow_console_output;

	LogSource* create_broadcaster();
	LogSource* create_source();

private:
	void add_message(LogSource* source, MessageType type, const string &message, const Array<string> &responses);
	Array<Message> messages;
	Array<Message> blocked;
};

}
