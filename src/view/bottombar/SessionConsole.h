/*
 * SessionConsole.h
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_


#include "BottomBar.h"

class Path;

class SessionConsole: public BottomBar::Console {
public:
	SessionConsole(Session *session, BottomBar *bar);
	~SessionConsole() override;

	Path directory() const;
	string id_list;

	void on_save();
	void on_list_double_click();

	void load_data();

	void save_session(const Path &filename);
	void load_session(const Path &filename);
};

#endif /* SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_ */
