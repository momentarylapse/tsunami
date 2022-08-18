/*
 * SessionConsole.h
 *
 *  Created on: 1 May 2022
 *      Author: michi
 */

#ifndef SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_
#define SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_


#include "BottomBar.h"

class SessionConsole: public BottomBar::Console {
public:
	SessionConsole(Session *session, BottomBar *bar);
	~SessionConsole() override;

	string id_list;

	enum class Type {
		ACTIVE,
		SAVED,
		BACKUP
	};

	struct SessionLabel {
		Type type;
		string name;
		Session *session;
	};
	Array<SessionLabel> session_labels;
	void find_sessions();

	void on_save();
	void on_list_double_click();

	void load_data();
};

#endif /* SRC_VIEW_BOTTOMBAR_SESSIONCONSOLE_H_ */
