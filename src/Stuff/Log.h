/*
 * Log.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef LOG_H_
#define LOG_H_

#include "../lib/file/file.h"

class Log
{
public:
	Log();
	virtual ~Log();

	void Error(const string &message);
	void Warning(const string &message);
	void Info(const string &message);

	void Show();
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
	Array<Message> message;
};

#endif /* LOG_H_ */
