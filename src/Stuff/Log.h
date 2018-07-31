/*
 * Log.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef LOG_H_
#define LOG_H_

#include "../lib/base/base.h"
#include "Observable.h"

class Session;

class Log : public Observable<VirtualBase>
{
public:
	Log(){}
	virtual ~Log(){}

	static const string MESSAGE_ADD;

	void error(Session *session, const string &message);
	void warn(Session *session, const string &message);
	void info(Session *session, const string &message);

	enum class Type{
		ERROR,
		WARNING,
		INFO
	};

	struct Message
	{
		Session *session;
		Type type;
		string text;
	};

	Array<Message> all(Session *session);

private:
	void addMessage(Session *session, Type type, const string &message);
	Array<Message> messages;
};

#endif /* LOG_H_ */
