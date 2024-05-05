/*
 * SessionConsole.h
 *
 *  Created on: 6 May 2024
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SIGNALCHAINCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_SIGNALCHAINCONSOLE_H_

#include "BottomBar.h"

struct SessionLabel;

class SignalChainConsole: public BottomBar::Console {
public:
	SignalChainConsole(Session *session, BottomBar *bar);
	~SignalChainConsole() override;

	string id_list;

	void on_new();
	void on_load();
	void on_save();
	void on_delete();
	void on_list_double_click();
	void on_right_click();

	void load_data();
};

#endif /* SRC_VIEW_BOTTOMBAR_SIGNALCHAINCONSOLE_H_ */
