/*
 * LogConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_LOGCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_LOGCONSOLE_H_

#include "BottomBar.h"

class Log;

class LogConsole: public BottomBar::Console {
public:
	LogConsole(Session *session, BottomBar *bar);
	virtual ~LogConsole();

	void reload();

	void on_log_add();

	Log *log;
	int messages_loaded;
};

#endif /* SRC_VIEW_BOTTOMBAR_LOGCONSOLE_H_ */
