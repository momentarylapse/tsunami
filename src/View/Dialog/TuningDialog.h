/*
 * TuningDialog.h
 *
 *  Created on: 11.02.2016
 *      Author: michi
 */

#ifndef SRC_VIEW_DIALOG_TUNINGDIALOG_H_
#define SRC_VIEW_DIALOG_TUNINGDIALOG_H_

#include "../../lib/hui/hui.h"

class Track;

class TuningDialog : public hui::Dialog {
public:
	TuningDialog(hui::Window *parent, Track *t);

	void update();

	void on_ok();

	void on_add_first();
	void on_add();
	void on_delete();
	void on_edit();

	Track *track;
	Array<int> tuning;

	int gui_num_strings;
};

#endif /* SRC_VIEW_DIALOG_TUNINGDIALOG_H_ */
