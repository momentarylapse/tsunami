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

class LogConsole;

class Log : public Observable
{
	friend class LogConsole;
public:
	Log();
	virtual ~Log();

	static const string MESSAGE_ADD;
	static const string MESSAGE_CLEAR;

	void error(const string &message);
	void warn(const string &message);
	void info(const string &message);

	void clear();

	enum{
		TYPE_ERROR,
		TYPE_WARNING,
		TYPE_INFO
	};

	struct Message
	{
		int type;
		string text;
	};
private:
	void addMessage(int type, const string &message);
	Array<Message> messages;
};

#endif /* LOG_H_ */
