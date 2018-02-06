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

class TuningDialog : public hui::ResourceWindow
{
public:
	TuningDialog(hui::Window *parent, Track *t);

	void update();

	void onOk();

	void onAddFirst();
	void onAdd();
	void onDelete();
	void onEdit();

	Track *track;
	Array<int> tuning;

	int gui_num_strings;
};

#endif /* SRC_VIEW_DIALOG_TUNINGDIALOG_H_ */
