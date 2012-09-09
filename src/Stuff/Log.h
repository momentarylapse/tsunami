/*
 * Log.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef LOG_H_
#define LOG_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"

class Log : public HuiEventHandler
{
public:
	Log(CHuiWindow *parent);
	virtual ~Log();

	void Error(const string &message);
	void Warning(const string &message);
	void Info(const string &message);

	void Show();
	void Clear();
	void Close();

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
	CHuiWindow *dlg;
};

#endif /* LOG_H_ */
