//
// Created by michi on 21.05.23.
//

#ifndef TSUNAMI_PRESETSELECTIONDIALOG_H
#define TSUNAMI_PRESETSELECTIONDIALOG_H

#include "../../lib/hui/Window.h"
#include "../../lib/base/future.h"

namespace tsunami {

class PresetSelectionDialog : public hui::Dialog {
public:
	PresetSelectionDialog(hui::Window *parent, const Array<string> &_names, bool _save);

	void on_list();
	void on_list_select();
	void on_name();
	void on_ok();

	bool save;
	Array<string> names;
	string selection;

	static base::future<string> ask(hui::Window* parent, const Array<string> &names, bool save);
};

}

#endif //TSUNAMI_PRESETSELECTIONDIALOG_H
