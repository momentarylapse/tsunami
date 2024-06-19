/*
 * EditStringsDialog.h
 *
 *  Created on: 11.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_EDITSTRINGSDIALOG_H_
#define SRC_VIEW_DIALOG_EDITSTRINGSDIALOG_H_

#include "../../lib/hui/hui.h"

namespace tsunami {

class Track;

class EditStringsDialog : public hui::Dialog {
public:
	EditStringsDialog(hui::Window *parent, const Array<int> &strings);

	void update();

	void on_ok();

	void on_add_first();
	void on_add();
	void on_delete();
	void on_edit();

	Array<int> strings;
	bool ok = false;

	int gui_num_strings;
};

}

#endif /* SRC_VIEW_DIALOG_EDITSTRINGSDIALOG_H_ */
