/*
 * ModuleSelectorDialog.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_MODULESELECTORDIALOG_H_
#define SRC_VIEW_DIALOG_MODULESELECTORDIALOG_H_

#include "../../lib/hui/hui.h"

class Session;
enum class ModuleCategory;

class ModuleSelectorDialog: public hui::Dialog {
public:
	ModuleSelectorDialog(hui::Window *_parent, ModuleCategory type, Session *session, const string &old_name = "");

	void on_list_select();
	void on_select();

	ModuleCategory type;
	Session *session;
	struct Label {
		string full;
		string name, group;
	};
	Array<Label> labels;
	Set<string> ugroups;

	static Label split_label(const string &s);

	string _return;
};

#endif /* SRC_VIEW_DIALOG_MODULESELECTORDIALOG_H_ */
