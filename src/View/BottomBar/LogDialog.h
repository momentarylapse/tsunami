/*
 * LogDialog.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef LOGDIALOG_H_
#define LOGDIALOG_H_

#include "BottomBar.h"

class Log;

class LogDialog: public BottomBarConsole, public Observer
{
public:
	LogDialog(Log *log);
	virtual ~LogDialog();

	void OnClear();
	void Reload();
	virtual void OnUpdate(Observable *o, const string &message);

	Log *log;
};

#endif /* LOGDIALOG_H_ */
