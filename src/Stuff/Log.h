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

class LogDialog;

class Log : public Observable
{
	friend class LogDialog;
public:
	Log();
	virtual ~Log();

	static const string MESSAGE_ADD;
	static const string MESSAGE_CLEAR;

	void Error(const string &message);
	void Warning(const string &message);
	void Info(const string &message);

	void Clear();

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
	void AddMessage(int type, const string &message);
	Array<Message> messages;
};

#endif /* LOG_H_ */
