/*
 * ModuleSelectorDialog.h
 *
 *  Created on: 16.08.2013
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_MODULESELECTORDIALOG_H_
#define SRC_VIEW_DIALOG_MODULESELECTORDIALOG_H_

#include "../../lib/hui/hui.h"
#include "../../lib/base/optional.h"

class Session;
enum class ModuleCategory;

class ModuleSelectorDialog: public hui::Dialog {
public:
	ModuleSelectorDialog(hui::Window *_parent, ModuleCategory type, Session *session, const base::optional<string> &old_name = base::None);

	void on_list_select();
	void on_toggle_favorite();
	void on_select();

	ModuleCategory type;
	Session *session;
	struct Label {
		string full;
		string name, group;
	};
	Array<Label> labels;
	base::set<string> ugroups;

	static Label split_label(const string &s);

	base::optional<string> _return;
	hui::promise<string> _promise;


	static hui::future<string> choose(hui::Panel *parent, Session *session, ModuleCategory type, const base::optional<string> &old_name = base::None);
};

#endif /* SRC_VIEW_DIALOG_MODULESELECTORDIALOG_H_ */
