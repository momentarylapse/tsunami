/*
 * LogConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef LOGCONSOLE_H_
#define LOGCONSOLE_H_

#include "BottomBar.h"

class Log;

class LogConsole: public BottomBar::Console
{
public:
	LogConsole(Session *session);
	virtual ~LogConsole();

	void reload();

	void on_log_add();

	Log *log;
	int messages_loaded;
};

#endif /* LOGCONSOLE_H_ */
