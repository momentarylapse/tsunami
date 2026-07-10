/*
 * LogConsole.h
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_LOGCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_LOGCONSOLE_H_

#include "BottomBar.h"

namespace tsunami {

class LogHub;
class LogSource;

class LogConsole: public BottomBar::Console {
public:
	LogConsole(Session* session, BottomBar* bar);
	~LogConsole() override;

	void reload();

	void on_log_add();

	LogSource* source;
	int messages_loaded;
};

}

#endif /* SRC_VIEW_BOTTOMBAR_LOGCONSOLE_H_ */
