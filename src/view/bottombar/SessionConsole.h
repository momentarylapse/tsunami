/*
 * SessionConsole.h
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_


#include "BottomBar.h"

struct SessionLabel;

class SessionConsole: public BottomBar::Console {
public:
	SessionConsole(Session *session, BottomBar *bar);
	~SessionConsole() override;

	string id_list;

	void on_load();
	void on_save();
	void on_delete();
	void on_list_double_click();
	void on_right_click();

	void load_data();

	Array<SessionLabel> session_labels;

	owned<hui::Menu> popup_menu;
};

#endif /* SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_ */
