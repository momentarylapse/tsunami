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

class LogConsole: public BottomBarConsole, public Observer
{
public:
	LogConsole(Log *log);
	virtual ~LogConsole();

	void onClear();
	void reload();
	virtual void onUpdate(Observable *o, const string &message);

	Log *log;
};

#endif /* LOGCONSOLE_H_ */
